/* 
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author: Karol Hrdina <karolhrdina@eaton.com>
 
Description: main cored
References: BIOS-248
*/
#ifndef MAIN_H_
#define MAIN_H_

#define MSG_INF_DEFAULT_SLEEP_NETMON \
  "Using default (sleep_min, sleep_max) values for netmon module. Please see \
help: -h OR --help"

#define   SRV_THREAD_POOL_MAX  5

#define PRINTF_STDERR(FORMAT,...) \
  fprintf(stderr, (FORMAT), __VA_ARGS__);


#endif // MAIN_H_

