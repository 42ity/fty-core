#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <termios.h>
#include <linux/serial.h>

                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0


int compensate_humidity(int H, int T, int32_t* out) {
    double tmp = H;
    // Get linear relative humidity
    tmp = -2.0468 + 0.0367 * tmp - 1.5955E-6 * tmp * tmp;
    // Temperature compensation
    tmp = ((double)T/100 - 25.0)*(0.01 + 0.00008 * (double)H) + tmp;
    *out = tmp * 100;
    return 0;
}

int compensate_temp(int in, int32_t *out) {
    // Initial compensation for 5V taken from datasheet
    *out = in - 4010;
    return 0;
}

int usleep(unsigned int);

void msleep(unsigned int m) {
    usleep(m*1000);
}

void set_tx(int fd, int state) {
    if(fd < 0)
        return;
    int what = TIOCM_DTR;
    if(state) {
        ioctl(fd, TIOCMBIS, &what);
    } else {
        ioctl(fd, TIOCMBIC, &what);
    }
}

int get_rx(int fd) {
    int what = 0;
    if(fd < 0)
        return -1;

    ioctl(fd, TIOCMGET, &what);
    return !(what & TIOCM_CTS);
}

void tick(int fd, int state) {
    if(fd < 0)
        return;
    static int last = 1;
    if(state == -1) {
        state = (last + 1) % 2;
    }
    int what = TIOCM_RTS;
    if(state) {
        ioctl(fd, TIOCMBIS, &what);
    } else {
        ioctl(fd, TIOCMBIC, &what);
    }
    last = state;
}

void long_tick(int fd, int state) {
    tick(fd, state);
    msleep(1);
}


/*
 Start sending commands
       _____         ________
 DATA:      |_______|
           ___     ___
 SCK : ___|   |___|   |______
*/

void command_start(int fd) {
    // Init
    set_tx(fd, 1);
    tick(fd, 0);
    // Wait a little with clocks
    long_tick(fd, 0);
    long_tick(fd, 1);
    // Zero for two ticks
    set_tx(fd, 0);
    msleep(1);
    long_tick(fd, 0);
    long_tick(fd, 1);
    // Back to one
    set_tx(fd, 1);
    msleep(1);
    tick(fd, 0);
}


/*
 Reset                                                 (-- start sending --)
       _____________________________________________________         ________
 DATA:                                                      |_______|
          _    _    _    _    _    _    _    _    _        ___     ___
 SCK : __|1|__|2|__|3|__|4|__|5|__|6|__|7|__|8|__|9|______|   |___|   |______

*/

void reset_device(int fd) {
    if(fd < 0)
        return;

    // Init
    set_tx(fd, 1);
    tick(fd, 0);
    // Long 1
    for(int i = 0; i < 9; i++) {
        long_tick(fd, 1);
        long_tick(fd, 0);
    }
    command_start(fd);
}

int read_byte(int fd, unsigned char *val, int ack) {
    set_tx(fd, 1);
    *val = 0;
    for(unsigned char mask = 0x80; mask > 0; mask = mask >> 1) {
        long_tick(fd, 1);
        if(get_rx(fd))
            *val = (*val) | mask;
        long_tick(fd, 0);
    }
    if(ack)
        set_tx(fd, 0);
    else
        set_tx(fd, 0);
    long_tick(fd, 1);
    long_tick(fd, 0);
    set_tx(fd, 1);
    return 0;
}

int write_byte(int fd, unsigned char val) {
    int err = 0;
    for(unsigned char mask = 0x80; mask > 0; mask = mask >> 1) {
        set_tx(fd, val & mask);
        long_tick(fd, 1);
        long_tick(fd, 0);
    }
    set_tx(fd, 1);
    long_tick(fd, 1);
    if(get_rx(fd))
        err = 1;
    long_tick(fd, 0);
    return err;
}

int get_measurement(int fd, unsigned char what) {
    unsigned char tmp[2];
    unsigned char crc;

    if(fd < 0)
        return -1;

    command_start(fd);
    if(write_byte(fd, what))
        return -1;

    msleep(50);

    for(int i=0;i<100000 && get_rx(fd); i++);
        usleep(100);

    if(get_rx(fd))
        return -1;

    read_byte(fd, tmp,   1);
    read_byte(fd, tmp+1, 1);
    read_byte(fd, &crc,  1);
    return ((int)tmp[0])*255 + (int)tmp[1];
}

bool device_connected(int fd) {
    if(fd < 0)
        return false;
    ioctl(fd, TIOCCBRK);
    sleep(1);
    ioctl(fd, TIOCSBRK);
    sleep(1);
    int bytes = 0;
    char buf = 'x';
    ioctl(fd, TIOCINQ, &bytes);
    if(bytes > 0 && read(fd, &buf, 1) && buf == '\0')
        return true;
    return false;
}

int open_device(const char* dev) {
    int fd = open(dev, O_RDWR);
    if(fd < 0)
        return fd;

    reset_device(fd);

    // Set connection parameters
    struct termios termios_s;
    tcgetattr(fd, &termios_s);
    termios_s.c_iflag &= ~(IGNBRK | BRKINT | PARMRK);
    termios_s.c_lflag &= ~ICANON;
    tcsetattr(fd, TCSANOW, &termios_s);

    //Flush all serial port buffers
    tcflush(fd, TCIOFLUSH);

    return fd;
}

