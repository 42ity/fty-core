#include <sys/types.h>

                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0

#ifdef __cplusplus
extern "C" {
#endif

//! Opens serial port
int open_device(const char* dev);
//! Check whether sensor is connected
bool device_connected(int fd);
//! Read measurement from sensor
int get_th_data(int fd, unsigned char what);
//! Fix temperature readings from the sensor
int compensate_temp(int in, int32_t *out);
//! Fix humidity readings from the sensor
int compensate_humidity(int H, int T, int32_t* out);
//! Reset device
void reset_device(int fd);

#ifdef __cplusplus
}
#endif
