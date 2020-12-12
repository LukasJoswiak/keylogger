CXX = clang++
CXXFLAGS = -Wall -g -std=c++17

SRCDIR = src
BUILDDIR = build
BINDIR = bin

all: $(BINDIR)/keylogger $(BINDIR)/reporter

$(BINDIR)/keylogger: $(BUILDDIR)/keylogger/keylogger.o $(BUILDDIR)/keylogger/recorder.o $(BUILDDIR)/keylogger/main.o | $(BINDIR)
	$(CXX) $^ -o $@ -framework ApplicationServices

$(BUILDDIR)/keylogger/%.o: $(SRCDIR)/keylogger/%.cpp | $(BUILDDIR)/keylogger
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BINDIR)/reporter: $(BUILDDIR)/reporter/reporter.o $(BUILDDIR)/reporter/main.o | $(BINDIR)
	$(CXX) $^ -o $@

$(BUILDDIR)/reporter/%.o: $(SRCDIR)/reporter/%.cpp | $(BUILDDIR)/reporter
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR)/keylogger $(BUILDDIR)/reporter $(BINDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(BINDIR)
