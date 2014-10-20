all: bios.so fake_message

CXXFLAGS+=-Iinclude -I../simple -I../utils/  -I../../tools `for i in cxxtools libsodium libsasl2; do pkg-config $$i --cflags; done` --std=c++11
CFLAGS+=-Iinclude -fPIC -I../simple -g
LDFLAGS+=-lm `for i in cxxtools libsodium libsasl2; do pkg-config $$i --libs; done` -lczmq -lzmq -ltntdb

test: all
	${TNTNET} tntnet.xml

clean:
	${RM} bios.so fake_message fake_message.o src/*.o

bios.so: src/auth.o src/auth-verify.o src/sasl.o src/tokens.o src/time.o src/item.o src/json.o src/list.o src/mock.o ../simple/asset_msg.o src/data.cpp ../utils/assetmsgpersistence.o ../utils/log.o
	${CXX} -o $@ $^ ${LDFLAGS} ${CXXFLAGS}

fake_message: fake_message.o ../simple/asset_msg.o src/data.o
	${CXX} -o $@ -I../simple $^ -lczmq

.SUFFIXES: .ecpp .gif .jpg .css .js .cpp
ECPPC=/usr/bin/ecppc
ECPPFLAGS+=-Isrc -Iinclude
TNTNET=/usr/bin/tntnet
CXXFLAGS+=-I/usr/include -fPIC -O0 -g
LDFLAGS+=-shared -L/usr/lib64 -L/usr/lib -ltntnet -lcxxtools

.ecpp.cpp:
	${ECPPC} ${ECPPFLAGS} ${ECPPFLAGS_CPP} -o $@ $<
.gif.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/gif ${ECPPFLAGS_GIF} -b -o $@ $<
.jpg.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/jpg ${ECPPFLAGS_JPG} -b -o $@ $<
.png.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/png ${ECPPFLAGS_PNG} -b -o $@ $<
.ico.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/x-icon ${ECPPFLAGS_ICO} -b -o $@ $<
.css.cpp:
	${ECPPC} ${ECPPFLAGS} -m text/css ${ECPPFLAGS_CSS} -b -o $@ $<
.js.cpp:
	${ECPPC} ${ECPPFLAGS} -m application/javascript ${ECPPFLAGS_JS} -b -o $@ $<
