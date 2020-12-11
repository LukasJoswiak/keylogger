all:
	clang++ -Wall -std=c++17 -g src/main.cpp src/keylogger.cpp src/recorder.cpp -o keylogger -framework ApplicationServices
