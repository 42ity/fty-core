#include "sample_agent.h"
#include "libth.h"
#include "defs.h"
#include "log.h"

#include <vector>
#include <map>
#include <string>

ymsg_t* get_measurement(char* what);

char *vars[] = {
    "temperature.TH1",
    "humidity.TH1",
    "temperature.TH2",
    "humidity.TH2",
    "temperature.TH3",
    "humidity.TH3",
    "temperature.TH4",
    "humidity.TH4",
    NULL
};

sample_agent agent = {
    "agent-th",
    NULL,
    NULL,
    vars,
    "%s",
    "%s",
    50,
    get_measurement
};

struct c_item {
    time_t time;
    bool broken;
    int32_t T;
    int32_t H;
};

ymsg_t* get_measurement(char* what) {
    static std::map<std::string, c_item> cache;
    ymsg_t* ret = NULL;
    char *th = strdup(what + (strlen(what) - 3));
    c_item data, *data_p = NULL;
    log_debug("Measuring %s", what);

    // Get data from cache and maybe update the cache
    auto it = cache.find(th);
    if((it == cache.end()) || (time(NULL) - it->second.time) > (NUT_POLLING_INTERVAL/1000)) {
        log_debug("No usable value in cache");
        std::string path = "/dev/ttyS1";
        th[2] += 2;
        path  += th + 2;
        th[2] -= 2;
        log_debug("Reading from %s", path.c_str());
        data.time = time(NULL);
        int fd = open_device(path.c_str());
        if(!device_connected(fd)) {
            if(fd > 0)
                close(fd);
            log_debug("No sensor attached to %s", path.c_str());
            data.broken = true;
        } else {
            reset_device(fd);
            data.T = get_th_data(fd, MEASURE_TEMP);
            data.H = get_th_data(fd, MEASURE_HUMI);
            compensate_temp(data.T, &data.T);
            compensate_humidity(data.H, data.T, &data.H);
            close(fd);
            log_debug("Got data from sensor %s - T = %" PRId32 ".%02" PRId32 " C,"
                                                "H = %" PRId32 ".%02" PRId32 " %%",
                      th, data.T/100, data.T%100, data.H/100, data.H%100);
            data_p = &data;
            data.broken = false;
        }
        if(it != cache.end()) {
            log_debug("Updating data in cache");
            it->second = data;
        } else {
            log_debug("Putting data into cache");
            cache.insert(std::make_pair(th, data));
        }
    } else {
        log_debug("Got usable data from cache");
        data_p = &(it->second);
    }

    free(th);
    if((data_p == NULL) || data_p->broken)
        return NULL;

    // Formulate a response
    if(what[0] == 't') {
        ret = bios_measurement_encode("", "", "C", data_p->T, 2, data_p->time);
        log_debug("Returning T = %" PRId32 ".%02" PRId32 " C",
                  data_p->T/100, data_p->T%100);
    } else {
        ret = bios_measurement_encode("", "", "%", data_p->H, 2, data_p->time);
        log_debug("Returning H = %" PRId32 ".%02" PRId32 " %%",
                  data_p->H/100, data_p->H%100);
    }
    return ret;
}
