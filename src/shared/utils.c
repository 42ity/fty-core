#include "utils.h"

const char *str_bool(const bool b) {
    return b ? "true" : "false";
}

const char *safe_str(const char *s) {
    return s == NULL ? "(null)" : s;
}

bool streq(const char *a, const char *b) {
	if (!a || !b) {
		return (!a && !b);
	}
	return !strcmp(a, b);
}
