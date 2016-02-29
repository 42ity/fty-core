/*
 *
 * Copyright (C) 2016 Eaton
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
 * \file dbstore_bench.cc
 * \author Gerald Guillaume <GeraldGuillaume@Eaton.com>
 * \brief do intensive and endurance insertion job 
 */
#include "dbpath.h"
#include "log.h"
#include "persistencelogic.h"
#include <getopt.h>
#include <bios_proto.h>
#include <ctime>



using namespace std;


long get_clock_ms(){
    struct timeval time;
    gettimeofday(&time, NULL); // Get Time
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

char clock_fmt[26];
char *get_clock_fmt(){
    time_t timer;
    
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(clock_fmt, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    return clock_fmt;
}

/* Insert one measurement on a random device, a random topic and a random value
 */
void insert_new_measurement( int device_id, int topic_id, persist::TopicCache &cache){
    char topic_name[32];
    char device_name[32];
    char value[16];
    sprintf(topic_name,"bench.topic%d",topic_id);
    sprintf(device_name,"bench.asset%d",device_id);
    sprintf(value,"%d",rand() % 999999 );
    zmsg_t *msg = bios_proto_encode_metric (NULL, topic_name, device_name, value, "%", time(NULL));
    persist::process_measurement(&msg, cache);
}
/*
 * do the bench insertion
 * \param delay       - pause between each insertion (in ms), 0 means no delay 
 * \param num_device  - number of simulated device  
 * \param topic_per_device  - number of topic per device simulated  
 * \param periodic_display  - Each periodic_display seconds, output time; row; average
 * \param total_duration    - bench duration in minute, -1 means infinite loop
 */ 
void bench(int delay=100, int num_device=100, int topic_per_device=100, int periodic_display=10, int total_duration=-1){
    log_info("delay=%dms periodic=%ds minute=%dm element=%d topic=%d", 
            delay,periodic_display,total_duration,num_device,topic_per_device);
    persist::TopicCache topic_cache{(size_t)(num_device*topic_per_device)};
    int stat_total_row=0; 
    int stat_periodic_row=0; 
    
    zsys_catch_interrupts ();
    
    long begin_overall_ms = get_clock_ms();
    long begin_periodic_ms = get_clock_ms();

    log_info("time;rows; mean over last %ds (row/s)",periodic_display);
    while(!zsys_interrupted) {
        insert_new_measurement(rand() % num_device, rand() % topic_per_device, topic_cache);
        //count stat
        stat_total_row++; 
        stat_periodic_row++;
        
        //time to display stat ?
        long now_ms = get_clock_ms();
        long elapsed_periodic_ms = (now_ms - begin_periodic_ms);
        //every period seconds display current total row count and the trend over the last periodic_display second
        if(elapsed_periodic_ms > periodic_display * 1000 ){
            log_info("%s;%d;%.2lf",get_clock_fmt(),stat_periodic_row,stat_periodic_row/(elapsed_periodic_ms/1000.0));
            stat_periodic_row=0;
            begin_periodic_ms = now_ms;
        }
        if (total_duration>0 && (now_ms - begin_overall_ms)/1000.0/60.0>total_duration)goto exit;
        
        //sleep before loop
        if(delay>0)usleep(delay*1000);
    }
    
exit:
    long elapsed_overall_ms = (get_clock_ms() - begin_overall_ms);
    
    log_info("%d rows inserted in  %.2lf seconds, overall avg=%.2lf row/s",stat_total_row,elapsed_overall_ms/1000.0,stat_total_row/(elapsed_overall_ms/1000.0));
    
    
    
}

void usage ()
{
    puts ("dbstore_bench [options] \n"
          "  -u|--url              mysql:db=box_utf8;user=bios;password=test (or set DB_PASSWD and DB_USER env variable)\n"
          "  -d|--delay            pause between each insertion (in ms), 0 means no delay [100]\n"
          "  -p|--periodic         output time; row; average each periodic_display seconds [10]\n"
          "  -m|--minute           bench duration in minute, -1 means infinite loop [-1]\n"
          "  -e|--element          number of simulated elements [100]\n"
          "  -t|--topic            number of simulated topic per element [100]\n"
          "  -h|--help             print this information");
}

/*
 * 
 */
int main(int argc, char** argv) {
    // set default
    int help = 0;
    int delay=100; //ms
    int periodic=10; //s
    int minute=-1; //min
    int element=100;
    int topic=100;
    
     // get options
    int c;
    while(true) {
        static struct option long_options[] =
        {
            {"help",       no_argument,       &help,    1},
            {"url",        required_argument, 0,'u'},
            {"delay",      required_argument, 0,'d'},
            {"periodic",   required_argument, 0,'p'},
            {"minute",     required_argument, 0,'m'},
            {"element",    required_argument, 0,'e'},
            {"topic",      required_argument, 0,'t'}, 
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long (argc, argv, "h:u:d:p:m:e:t:", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
        case 'u':
            url = optarg;
            break;
        case 'd':
            delay = atoi(optarg);
            break;
        case 'p':
            periodic = atoi(optarg);
            break;
        case 'm':
            minute = atoi(optarg);
            break;
        case 'e':
            element = atoi(optarg);
            break;
        case 't':
            topic = atoi(optarg);
            break;
        case 0:
            // just now walking trough some long opt
            break;
        case 'h':
        default:
            help = 1;
            break;
        }
    }
    if (help) { usage(); exit(1); }
    
    log_open();
    log_debug("## bench started ##");

    bench(delay,element, topic,  periodic, minute);
    return 0;
}

