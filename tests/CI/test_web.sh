#!/bin/sh

PASS=0
TOTAL=0

BIOS_USER="bios"
BIOS_PASSWD="nosoup4u"
if [ "x$1" = "x-u" ]; then
    BIOS_USER="$2"
    shift 2
fi
if [ "x$1" = "x-p" ]; then
    BIOS_PASSWD="$2"
    shift 2
fi

PATH="$PATH:/sbin:/usr/sbin"

BASE_URL="http://127.0.0.1:8000/api/v1"

_TOKEN_=""

print_result() {
    _ret=0
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
        _ret=1
        FAILED="$FAILED $NAME"
    fi
    TOTAL="`expr $TOTAL + 1`"
    echo
    return $_ret
}

test_it() {
    if [ "$1" ]; then
        NAME="$1"
    fi
    [ "$NAME" ] || NAME="`basename "$0" .sh`"
    echo "Running test $NAME:"
}

api_get() {
    curl -v --progress-bar "$BASE_URL$1" 2>&1
}

api_get_json() {
    curl -v --progress-bar "$BASE_URL$1" 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_post() {
    curl -v -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

_api_get_token() {
    if [ -z "$_TOKEN_" ]; then
    _TOKEN_="`api_get "/oauth2/token?username=$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" | \
            sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
    fi
    echo "$_TOKEN_"
}

api_auth_post() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_delete() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -X "DELETE" --progress-bar "$BASE_URL$1" 2>&1
}

# fixture ini
if ! pidof saslauthd > /dev/null; then
    echo "saslauthd does not run, please start it first!"
    exit 1
fi

# check the user morbo in system
# I expects SASL uses Linux PAM, therefor getent will tell us it all
if ! getent passwd "$BIOS_USER" > /dev/null; then
    echo "User $BIOS_USER is not known to system administrative database"
    echo "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody --no-create-home --no-user-group $BIOS_USER"
    echo "and don't forget the password '$BIOS_PASSWD'"
    exit 2
fi

if ! testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios > /dev/null; then
    echo "SASL autentification for user '$BIOS_USER' have failed. Check the existence of /etc/sasl2/bios.conf and /etc/pam.d/bios"
    exit 3
fi

if [ -z "`api_get "" | grep "< HTTP/.* 404 Not Found"`" ]; then
    echo "Webserver is not running, please start it first!"
    exit 4
fi

cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4
cd web/commands
[ "$1" ] || set *
while [ "$1" ]; do
    for NAME in *$1*; do
    . ./"$NAME" 5> "$LOG_DIR/$NAME".log
    if [ -r "../results/$NAME".res ]; then
        diff -Naru "../results/$NAME".res "$LOG_DIR/$NAME".log
        print_result $?
    fi
    done
    shift
done

echo "Testing completed, $PASS/$TOTAL tests passed"
[ -z "$FAILED" ] && exit 0

echo "Following tests failed:"
for i in $FAILED; do
    echo " * $i"
done
exit 1
