#!/bin/sh

PASS=0
TOTAL=0

USER="morbo"
PASSWD="iwilldestroyyou"

PATH="$PATH:/sbin:/usr/sbin"

BASE_URL="http://127.0.0.1:8000/api/v1"

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

api_auth_post() {
    TOKEN="`api_get "/oauth2/token?username=$USER&password=$PASSWD&grant_type=password" | \
            sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

# fixture ini
if ! pidof saslauthd > /dev/null; then
    echo "saslauthd does not run, please start it first!"
    exit 1
fi

# check the user morbo in system
# I expects SASL uses Linux PAM, therefor getent will tell us it all
if ! getent passwd "${USER}" > /dev/null; then
    echo "User ${USER} is not known to system administrative database"
    echo "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment "BIOS REST API testing user" --groups nobody --no-create-home --no-user-group ${USER}"
    echo "and don't forget the password '${PASSWD}'"
    exit 2
fi

if ! testsaslauthd -u "${USER}" -p "${PASSWD}" -s bios > /dev/null; then
    echo "SASL autentification for user '${USER}' have failed. Check the existence of /etc/sasl2/bios.conf and /etc/pam.d/bios"
    exit 3
fi

cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4
cd web/commands
[ "$1" ] || set *
while [ "$1" ]; do
    NAME="$1"
    . ./"$1" 5> "$LOG_DIR/$1".log
    if [ -r "../results/$1".res ]; then
        diff -Naru "../results/$1".res "$LOG_DIR/$1".log | sed 's|^|\ \ \ |'
        print_result $?
    fi
    shift
done

echo "Testing completed, $PASS/$TOTAL tests passed"
echo "Following tests failed:"
for i in $FAILED; do
    echo " * $i"
done
