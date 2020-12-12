all: keylogger reporter

keylogger:
	mkdir -p bin
	clang++ -Wall -std=c++17 -g src/keylogger/keylogger.cpp src/keylogger/recorder.cpp src/keylogger/main.cpp -o bin/keylogger -framework ApplicationServices

reporter:
	mkdir -p bin
	clang++ -Wall -std=c++17 -g src/reporter/reporter.cpp src/reporter/main.cpp -o bin/reporter
