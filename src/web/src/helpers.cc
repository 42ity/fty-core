#include <cassert>

#include "helpers.h"

#include "log.h"

bool
check_element_identifier (const char *param_name, const std::string& param_value, uint32_t& element_id, http_errors_t& errors) {
    assert (param_name);
    if (param_value.empty ()) {
        http_add_error (errors,"request-param-required", param_name);
        return false;
    }

    uint32_t eid = 0;
    try {
        eid = utils::string_to_element_id (param_value);
    }
    catch (const std::invalid_argument& e) {
        http_add_error (errors, "request-param-bad", param_name,
            std::string ("value '").append (param_value).append ("'").append (" is not an element identifier").c_str (),
            std::string ("an unsigned integer in range 1 to ").append (std::to_string (UINT_MAX)).append (".").c_str ());
        return false;
    }
    catch (const std::out_of_range& e) {
        http_add_error (errors, "request-param-bad", param_name,
            std::string ("value '").append (param_value).append ("'").append (" is out of range").c_str (),
            std::string ("value in range 1 to ").append (std::to_string (UINT_MAX)).append (".").c_str ());
        return false;
    }
    catch (const std::exception& e) {
        log_error ("std::exception caught: %s", e.what ());
        http_add_error (errors, "internal-error");
        return false;
    }
    element_id = eid;
    return true;
}
