#!/bin/sh

#TEST for /admin/network POST/DELETE
#TODO: can't run twice, needs to extract ids from POSTed entries

# common testing function
# Usage:
# do_test $test_name $api_call $url $regexp
# do_test $test_name $api_call $url $url_args $regexp
do_test() {
    local test_name api_call url api_args regexp out err

    case $# in
        0|1|2|3)
            echo "ERROR: do_test insuficient number of arguments" >&2
            return 1
            ;;
        4)
            test_name=${1}
            api_call=${2}
            url=${3}
            api_args=""
            regexp=${4}
            ;;
        5)
            test_name=${1}
            api_call=${2}
            url=${3}
            api_args=${4}
            regexp=${5}
            ;;
        *)
            echo "ERROR: do_test too many arguments: $#" >&2
            return 1
            ;;
    esac

    out="../log/${test_name}.stdout.log"
    err="../log/${test_name}.stderr.log"

    test_it "${test_name}"
    ${api_call} ${url} "${api_args}" > "${out}"  2> "${err}"
    if ! grep -q -E "${regexp}" "${out}"; then
        echo "    >>>>> DEBUG: ${out} <<<<"
        cat "${out}"
        echo "    >>>>> DEBUG: ${err} <<<<"
        cat "${err}"
        echo "    >>>>> \\DEBUG: ${test_name} <<<<"
        return 1
    fi
    rm -f "${err}" "${out}"
    return 0
}

do_test "admin_network_bad_method" api_get /admin/network 'HTTP/1.1 404 Not Found'
print_result $?

do_test "admin_network_bad_request" api_auth_post /admin/network '{"net" : "123"}' 'HTTP/1.1 400 Bad Request'
print_result $?

do_test "admin_network_post_excl1" api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "E"}' '{ "id" : "5" }'
print_result $?

# check the double POST just returns the same ID
do_test "admin_network_post_excl2" api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "E"}' '{ "id" : "5" }'
print_result $?

# ... but manual status is different
do_test "admin_network_post_man1" api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "M"}' '{ "id" : "6" }'
print_result $?

do_test "admin_network_post_man2" api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "M"}' '{ "id" : "6" }'
print_result $?

# and let's del 'em all
do_test "admin_network_delete5" api_auth_delete /admin/network/5 'HTTP/1.1 200 OK'
print_result $?
do_test "admin_network_delete6" api_auth_delete /admin/network/6 'HTTP/1.1 200 OK'
print_result $?

# delete non-existent network
do_test "admin_network_delete12" api_auth_delete /admin/network/12 'HTTP/1.1 400 Bad Request'
print_result $?

# non numeric id in url - catched by tntnet.xml mapping
do_test "admin_network_deleteabcd" api_auth_delete /admin/network/abcd 'HTTP/1.1 404 Not Found'
print_result $?

# delete automatic network
test_it "admin_network_delete7" api_auth_delete /admin/network/7 'HTTP/1.1 400 Bad Request'
print_result $?
