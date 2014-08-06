#include <assert.h>
#include <stdbool.h>

#include "utils.h"

int test_streq() {
/* test streq */
    assert(streq(NULL, NULL));
    assert(!streq("a", NULL));
    assert(!streq(NULL, "b"));
    assert(!streq("a", "b"));
    assert(streq("a", "a"));
    return 0;
}

int test_strbool() {
    assert(streq(str_bool(true), "true"));
    assert(streq(str_bool(false), "false"));
    return 0;
}

int test_safe_str() {
    assert(streq(safe_str(NULL), "(null)"));
    assert(streq(safe_str("a"), "a"));
    return 0;
}

int main() {
    test_streq();
    test_strbool();
    test_safe_str();
    return 0;
}

