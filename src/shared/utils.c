#include "utils.h"
#include "str_defs.h"

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

bool is_average_step_supported (const char *step) {
    if (!step) {
        return false;
    }
    for (int i = 0; i < AVG_STEPS_SIZE; ++i) {
        if (strcmp (step, AVG_STEPS[i]) == 0) {
            return true;
        }
    }
    return false;
}
