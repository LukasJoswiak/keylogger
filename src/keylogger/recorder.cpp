#include "recorder.hpp"

#include <Carbon/Carbon.h>

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
  // CGEventGetTimestamp(event) can be used to get the timestamp of the event
  // in nanoseconds since startup, but it's easier to just use the system clock
  // to get a normalized timestamp (microseconds since epoch) at the point in
  // time when the event is handled. Exact nanosecond precision isn't really
  // all that important here, as long as the time is relatively accurate. On my
  // computer, std::chrono::system_clock provides microsecond precision, and
  // std::chrono::high_resolution_clock provides nanosecond precision.
  int64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

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
  // Computers print characters on the screen by mapping physical key presses
  // to Unicode output symbols. Different keyboard layouts (input sources)
  // allow the same physical key on a keyboard to produce different symbols on
  // the screen. Physical key to Unicode output symbol mappings are stored in
  // the a format called 'uchr'. This function takes a keyboard event, which
  // contains things like modifier key state, and uses the current input source
  // to translate the physical key press into a Unicode output character.
  CGEventKeyboardGetUnicodeString(event, max_string_length, &actual_string_length, unicode_string);
  uint16_t unicode_keycode = unicode_string[0];

  /*
   * BEGIN UNUSED SECTION
   *
   * The code in the following section fetches a virtual keycode instead of the
   * underlying Unicode character produced when pressing a key. A virtual
   * keycode is a number representing a physical key on the keyboard. For
   * example, when using the QWERTY input source, pressing the 'B' key will
   * produce a Unicode 'B' character and the virtual keycode 11 (on ANSI
   * keyboards at least, equal to the constant kVK_ANSI_B). Switching to Dvorak
   * and pressing the physical 'B' key on the keyboard will produce a Unicode
   * 'X' character, but the virtual keycode produced will still be 11.
   * Depending on the goal of the user, either behavior might be desired. For
   * now, this code will remain unused, but in the future I plan to add an
   * option to allow either methodology for recording keystrokes (TODO).
   */

  // The keycode returned is a virtual keycode. This code corresponds to which
  // key was pressed on the keyboard, not which character that key produces.
  // For example, on US keyboards, the key labeled 'B' will produce a virtual
  // keycode of 11 (kVK_ANSI_B) no matter if the input source is set to Qwerty,
  // Dvorak, etc...
  // int64_t value = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
  // assert(value >= 0 && value < 65536);
  // Apple's documentation for UCKeyTranslate states "For ADB keyboards,
  // virtual key codes are in the range from 0 to 127". However, the function
  // accepts a 16-bit unsigned integer. I haven't been able to verify a virtual
  // keycode can be larger than 8-bits, but I can imagine a keyboard with more
  // than 256 keys that would require a larger storage type here.  Making this
  // 16-bit to be safe.
  // uint16_t virtual_keycode = static_cast<uint16_t>(value);

  /*
   * END UNUSED SECTION
   */

  //
  // Serialization format (v1):
  // +--------------------+ +---------------------+ +---------+
  // |       version      | |      timestamp      | | keycode | . . .
  // +--------------------+ +---------------------+ +---------+
  //         64 bits                64 bits           16 bits
  //
  // version: 64-bit unsigned integer representing the serialization version
  // timestamp: 64-bit signed integer, microseconds since epoch
  // keycode: 16-bit unsigned integer, Unicode encoding of typed character
  //
  // Timestamp, keycode pairs repeat.
  //
  ostrm_.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
  ostrm_.write(reinterpret_cast<const char*>(&unicode_keycode), sizeof(unicode_keycode));
}
