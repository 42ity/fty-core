test: logg cidr pers ../../../tests/utils/test-persistence.cc 
	g++ -I../../../tests/include -I.. -I. ../../../tests/utils/test-persistence.cc ../../../tests/testing.o log.o pers.o cidr.o -std=c++11 -lcidr -ltntdb -lcxxtools -o gotest

testing:
	g++ -I./../../include ./../../test-run.cc -c -o testing.o

go: common.o testing.o monitor.o totime.o databaseobject.o
	g++ -I../../include -I../../../src/utils -I../../../tools -I../../../src/simple test-monitor.cc testing.o common.o monitor.o totime.o databaseobject.o -lczmq -ltntdb  -lcxxtools  -o gotest

common: 
	g++ ../../../src/simple/common_msg.c -c -o common.o

monitor:
	g++ -I../../../src/simple -I../../../src/utils/persistence/ ../../../src/utils/monitorpersistence.cc -std=c++11 -c -o monitor.o

totime:
	g++ -I../../../src/utils/ ../../../src/utils/persistence/databasetimeobject.cc  -std=c++11  -c -o totime.o

databaseobject:
	g++  -I../../../src/utils/ ../../../src/utils/persistence/databaseobject.cc  -std=c++11  -c -o databaseobject.o

