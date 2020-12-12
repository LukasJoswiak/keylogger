all:
	clang++ -Wall -std=c++17 -g src/main.cpp src/keylogger.cpp src/recorder.cpp src/reporter.cpp -o keylogger -framework ApplicationServices
