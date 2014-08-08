#ifndef MAIN_H_
#define MAIN_H_

#define MSG_INF_DEFAULT_SLEEP_NETMON \
  "Using default (sleep_min, sleep_max) values for netmon module. Please see \
help: -h OR --help"

#define MOD_NETMON_DEFAULT_MIN  1
#define MOD_NETMON_DEFAULT_MAX  3

#define PRINTF_STDERR(FORMAT,...) \
  fprintf(stderr, (FORMAT), __VA_ARGS__);


#endif // MAIN_H_
