CXXFLAGS=-std=c++11 -pthread -Wall -Werror -g

all: gtest_kvdb

kvdb.o: kvdb.cc kvdb.h
test_kvdb.o: test_kvdb.cc kvdb.h
gtest_kvdb.o: gtest_kvdb.cc kvdb.h

gtest_kvdb: gtest_kvdb.o kvdb.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} -lgtest -lgtest_main -o $@ $^

clean:
	${RM} *.o gtest_kvdb

test: gtest_kvdb
	for i in `seq 1 200`; do echo -e "\nRun $$i...\n"; ./gtest_kvdb || exit 1; done
