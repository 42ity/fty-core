/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file libth.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Not yet documented file
 */
#include <sys/types.h>

#ifndef SRC_AGENTS_TH_LIBTH_H
#define SRC_AGENTS_TH_LIBTH_H
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

#endif // SRC_AGENTS_TH_LIBTH_H
