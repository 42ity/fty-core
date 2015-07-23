#/!bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: sets up the sandbox and runs the tests of REST API for
# the $BIOS project.

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to run the requests"

[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"
[ -z "$SUT_HOST" ] && SUT_HOST="127.0.0.1"
[ -z "$SUT_WEB_PORT" ] && SUT_WEB_PORT="8000"

[ -z "$WEBLIB_FUNC" ] && WEBLIB_FUNC="api_auth_get_content"

# Set up weblib test engine preference defaults for automated CI tests
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="fatal"
[ -z "$WEBLIB_QUICKFAIL" ] && \
    WEBLIB_QUICKFAIL=no
[ -z "$WEBLIB_CURLFAIL" ] && \
    WEBLIB_CURLFAIL=no
[ -z "$SKIP_NONSH_TESTS" ] && \
    SKIP_NONSH_TESTS=yes
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_QUICKFAIL WEBLIB_CURLFAIL SKIP_NONSH_TESTS

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

usage(){
    echo "Usage: $(basename $0) [options...] RELATIVE_URL [MethodArgs...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
    echo "  -m|--method which routine to use from weblib.sh (Default: '$WEBLIB_FUNC')"
    echo "NOTE: RELATIVE_URL is under the BASE_URL (host:port/api/v1)"
}

RELATIVE_URL=""
while [ $# -gt 0 ] ; do
    case "$1" in
        --user|-u)
            BIOS_USER="$2"
            shift
            ;;
        --passwd|-p)
            BIOS_PASSWD="$2"
            shift
            ;;
        -m|--method)
            WEBLIB_FUNC="$2"
            shift
            ;;
        --help|-h)
            usage
            exit 1
            ;;
        /*) # Assume that an URL follows
            RELATIVE_URL="$1"
            shift # if anything remains, pass it to CURL - e.g. POST data or headers
            break
            ;;
        *)  die "Unrecognized params follow: $*" \
                "Note that the RELATIVE_URL must start with a slash"
            ;;
    esac
    shift
done

# Included after CLI processing because sets autovars like BASE_URL
. "$SCRIPTDIR/weblib.sh" || CODE=$? die "Can not include web script library"

test_web_port() {
    # NOTE: Localhost only :)
    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}

wait_for_web() {
    for a in $(seq 60) ; do
        if ( test_web_port ) ; then
            return 0
        fi
        sleep 5
    done
    logmsg_error "Port ${SUT_WEB_PORT} still not in LISTEN state" >&2
    return 1
}

# it is up to the caller to prepare environment - tntnet, saslauthd, malamute etc.
  if ! pidof saslauthd > /dev/null; then
    logmsg_error "saslauthd is not running, you may need to start it first!"
  fi

  if ! pidof malamute > /dev/null; then
    logmsg_error "malamute is not running (locally), you may need to start it first!"
  fi

  if ! pidof mysqld > /dev/null ; then
    logmsg_error "mysqld is not running (locally), you may need to start it first!"
  fi

  logmsg_info "Waiting for web-server to begin responding..."
  wait_for_web && \
    logmsg_info "Web-server is responsive!" || \
    die "Web-server is NOT responsive!" >&2

# Validate the fundamental BIOS webserver capabilities
  logmsg_info "Testing webserver ability to serve the REST API"
  curlfail_push_expect_404
  if [ -n "`api_get "" 2>&1 | grep '< HTTP/.* 500'`" ] >/dev/null 2>&1 ; then
    logmsg_error "api_get() returned an error:"
    api_get "" >&2
    CODE=4 die "Webserver code is deeply broken, please fix it first!"
  fi

  if [ -z "`api_get "" 2>&1 | grep '< HTTP/.* 404 Not Found'`" ] >/dev/null 2>&1 ; then
    # We do expect an HTTP-404 on the API base URL
    logmsg_error "api_get() returned an error:"
    api_get "" >&2
    CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
  fi
  curlfail_pop

  curlfail_push_expect_noerrors
  if [ -z "`api_get '/oauth2/token' 2>&1 | grep '< HTTP/.* 200 OK'`" ] >/dev/null 2>&1 ; then
    # We expect that the login service responds
    logmsg_error "api_get() returned an error:"
    api_get "/oauth2/token" >&2
    CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
  fi
  curlfail_pop
  logmsg_info "Webserver seems basically able to serve the REST API"

  logmsg_info "Requesting: '$BASE_URL$RELATIVE_URL' with $WEBLIB_FUNC $*"
  eval "$WEBLIB_FUNC" "$RELATIVE_URL" "$@"
