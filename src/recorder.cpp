#include "recorder.hpp"

#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

#include <cassert>
#include <chrono>

Recorder::Recorder(const std::string& out_path) : ostrm_(out_path, std::ios::binary | std::ios::app) {
  // If there is an existing data file, make sure its version matches the
  // current write version. If the versions don't match, stop execution to
  // avoid corrupting the file by writing data in an incompatible format.
  if (ostrm_.tellp() > 0) {
    std::ifstream istrm = std::ifstream(out_path, std::ios::binary | std::ios::in);
    auto version = decltype(kWriteVersion){0};
    istrm.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != kWriteVersion) {
      throw std::runtime_error("output file has existing data encoded at a different version");
    }
  } else {
    // Write the version, in little-endian, to the new file.
    ostrm_.write(reinterpret_cast<const char*>(&kWriteVersion), sizeof(kWriteVersion));
  }
}

void Recorder::Handle(CGEventType type, CGEventRef event) {
  CGEventTimestamp timestamp = CGEventGetTimestamp(event);

  // The keycode returned is a virtual keycode. This code corresponds to which
  // key was pressed on the keyboard, not which character that key produces.
  // For example, on US keyboards, the key labeled 'B' will produce a virtual
  // keycode of 11 (kVK_ANSI_B) no matter if the input source is set to Qwerty,
  // Dvorak, etc...
  int64_t value = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
  assert(value >= 0 && value < 65536);
  // Apple's documentation for UCKeyTranslate (used below) states "For ADB
  // keyboards, virtual key codes are in the range from 0 to 127". However, the
  // function accepts a 16-bit unsigned integer. I haven't been able to verify
  // a virtual keycode can be larger than 8-bits, but I can imagine a keyboard
  // with more than 256 keys that would require a larger storage type here.
  // Making this 16-bit to be safe.
  uint16_t virtual_keycode = static_cast<uint16_t>(value);

  // Translate the virtual keycode plus any dead keys and modifiers into the
  // Unicode character the user meant to type. This facilitates recording the
  // count of Unicode characters the user is typing, but removes the ability to
  // record the count of physical keyboard keys pressed.
  // TODO: Create an option to allow the user to select whether they want to
  // record physical keyboard keys pressed or characters produced. For example,
  // with the current recording methodology, pressing the 'B' key on QWERTY,
  // switching to a Dvorak input source, and pressing the 'N' key on your
  // keyboard will result in two 'B' characters being recorded, even though two
  // different physical keys were pressed.
  uint16_t unicode_keycode = KeycodeToUnicode(virtual_keycode, event);

  ostrm_.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
  ostrm_.write(reinterpret_cast<const char*>(&unicode_keycode), sizeof(unicode_keycode));
}

uint16_t Recorder::KeycodeToUnicode(uint16_t virtual_keycode, CGEventRef event) {
  // First, get the current input source. Each input source has a set of
  // mappings of virtual keycodes to Unicode symbol(s). An input source has
  // multiple mappings to allow different output based on which modifier key is
  // currently held down. These mappings are contained in a format called
  // 'uchr'. A UCKeyboardLayout pointer refers to the header of the 'uchr'
  // resource and contains information on how to read the mappings contained
  // within.
  TISInputSourceRef input_source = TISCopyCurrentKeyboardLayoutInputSource();
  CFDataRef input_source_property = static_cast<CFDataRef>(TISGetInputSourceProperty(input_source, kTISPropertyUnicodeKeyLayoutData));
  const UCKeyboardLayout* keyboard_layout = reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr((CFDataRef) input_source_property));

  int64_t keyboard_type = CGEventGetIntegerValueField(event, kCGKeyboardEventKeyboardType);

  // Record any modifier keys that are being held down. Modifier keys may
  // change the character produced when pressing a key.
  uint32_t modifiers = 0;
  CGEventFlags flags = CGEventGetFlags(event);
  if (flags & kCGEventFlagMaskAlphaShift) modifiers |= alphaLock;  // caps lock
  if (flags & kCGEventFlagMaskShift) modifiers |= shiftKey;
  if (flags & kCGEventFlagMaskControl) modifiers |= controlKey;
  if (flags & kCGEventFlagMaskAlternate) modifiers |= optionKey;
  if (flags & kCGEventFlagMaskCommand) modifiers |= cmdKey;

  // A dead key is a modifier key used to change the output symbol of the
  // subsequently pressed key. It differs from regular modifiers in that it
  // doesn't have to be held down while the next key is pressed.
  uint32_t dead_keys = 0;

  UniCharCount max_string_length = 1;
  UniCharCount actual_string_length;
  // UniChar is typedef'd as an unsigned short. This allows a single UniChar
  // value to represent 2^16 - 1 = 65535 Unicode characters. It appears to me
  // that almost all common Unicode characters that appear on a keyboard have
  // encodings in the range [0, 65535]. In fact, the macOS "Unicode Hex Input"
  // input source doesn't even support entering Unicode symbols longer than
  // four hex bytes (65535). For this reason, as well as to keep the character
  // encoding compact, I've decided to limit the Unicode characters recognized
  // here to those in the range [0, 2^16 - 1].
  UniChar unicode_string[max_string_length];

  // Finally, convert the virtual keycode, modifier key state, and dead-key
  // state into a Unicode string using the 'uchr' mapping for the current input
  // source.
  OSStatus result = UCKeyTranslate(
      keyboard_layout,
      virtual_keycode,
      kUCKeyActionDown,
      (modifiers >> 8) & 0xff,
      keyboard_type,
      kUCKeyTranslateNoDeadKeysMask,
      &dead_keys,
      max_string_length,
      &actual_string_length,
      unicode_string);

  // TODO: Handle `result` non-zero return codes.

  CFRelease(input_source);

  return unicode_string[0];
}
