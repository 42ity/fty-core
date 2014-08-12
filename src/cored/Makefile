# TODO KHR 1.8 Migrate flgas to Makefile.am
CFLAGS=-Wall -Wextra -Wunused-variable -Wshadow -Wpedantic -Wcast-qual -std=c++11
LDFLAGS=

TARGETS=main

.PHONY: all clean runtests

all: $(TARGETS)

$(TARGETS): main.o mock_netmon.o utilities.o utilities_zeromq.o mock_db.o
	$(CC) $(CFLAGS) -lstdc++ -lzmq -pthread -ljsoncpp -o $@ $^

main.o: main.cc
	$(CC) $(CFLAGS) -c -o $@ $<

mock_netmon.o: mock_netmon.cc mock_netmon.h
	$(CC) $(CFLAGS) -c -o $@ $<

utilities.o: utilities.cc utilities.h
	$(CC) $(CFLAGS) -c -o $@ $<

utilities_zeromq.o: utilities_zeromq.cc utilities_zeromq.h
	$(CC) $(CFLAGS) -c -o $@ $<

mock_db.o: mock_db.cc mock_db.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) *.o

# template  file.o: dependecies .cc .h ...
#  -c -o $@ $<
