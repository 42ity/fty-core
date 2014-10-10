all: bios.so

CXXFLAGS+=-Iinclude `for i in cxxtools libsodium libsasl2; do pkg-config $$i --cflags; done` --std=c++11
CFLAGS+=-Iinclude -fPIC
LDFLAGS+=-lm `for i in cxxtools libsodium libsasl2; do pkg-config $$i --libs; done`

test: all
	${TNTNET} tntnet.xml

clean:
	${RM} bios.so src/*.o

bios.so: src/auth.o src/auth-verify.o src/sasl.o src/tokens.o src/time.o
	${CXX} -o $@ $^ ${LDFLAGS}

.SUFFIXES: .ecpp .gif .jpg .css .js .cpp
ECPPC=/usr/bin/ecppc
ECPPFLAGS+=-Isrc -Iinclude
TNTNET=/usr/bin/tntnet
CXXFLAGS+=-I/usr/include -fPIC -O2
LDFLAGS+=-shared -L/usr/lib64 -ltntnet -lcxxtools

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
