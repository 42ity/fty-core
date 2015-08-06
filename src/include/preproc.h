#ifndef SRC_INCLUDE_PREPROC_H__
#define SRC_INCLUDE_PREPROC_H__

// marker to tell humans and GCC that the unused parameter is there for some
// reason (i.e. API compatibility) and compiler should not warn if not used
#ifndef UNUSED_PARAM
# ifdef __GNUC__
#  define UNUSED_PARAM __attribute__ ((__unused__))
# else
#  define UNUSED_PARAM
# endif
#endif

#endif // SRC_INCLUDE_PREPROC_H__

