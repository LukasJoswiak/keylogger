all:
	clang++ -Wall -std=c++11 src/main.cpp src/keylogger.cpp src/recorder.cpp -o keylogger -framework ApplicationServices
