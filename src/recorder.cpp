#include "recorder.hpp"

#include <Carbon/Carbon.h>

#include <cassert>
#include <chrono>
#include <iostream>

Recorder::Recorder(const std::string& out_path) : ostrm_(out_path, std::ios::binary | std::ios::app) {}

void Recorder::Handle(CGEventType type, CGEventRef event) {
  CGEventTimestamp timestamp = CGEventGetTimestamp(event);
  int64_t raw_keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
  // Keycodes should be one byte (based on virtual keycodes defined in Event.h
  // system header).
  assert(raw_keycode >= 0 && raw_keycode < 256);
  uint8_t keycode = static_cast<uint8_t>(raw_keycode);

  std::cout << timestamp << " " << (int) keycode << std::endl;
  ostrm_.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
  ostrm_.write(reinterpret_cast<const char*>(&keycode), sizeof(keycode));
}
