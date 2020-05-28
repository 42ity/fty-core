#!/bin/bash
#
# Copyright (C) 2014 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   restapi-request.sh
#  \brief  sets up the sandbox and runs the tests of REST API for the 42ity project
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

# Defaults; note that SUT_WEB_PORT is guessed below based on ultimate SUT_HOST
[ -z "$SUT_HOST" ] && SUT_HOST="127.0.0.1"
[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""

# REST API returns JSON except one or two special cases
[ -z "$WEBLIB_FUNC" ] && WEBLIB_FUNC="api_auth_get_json"

# Set up weblib test engine preference defaults for automated CI tests
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="fatal"
[ -z "$WEBLIB_CURLFAIL" ] && \
    WEBLIB_CURLFAIL=no
[ -z "$SKIP_NONSH_TESTS" ] && \
    SKIP_NONSH_TESTS=yes
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_CURLFAIL SKIP_NONSH_TESTS

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

usage(){
    echo "Usage: $(basename $0) [options...] RELATIVE_URL [MethodArgs...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
    echo "  --host NAME       REST API service host name [$SUT_HOST]"
    echo "  --port-web PORT   REST API service HTTP port (default: it depends)"
    echo "  --use-https|--use-http  Specify if REST API service requires HTTP(S)"
    echo "  -q|--quick  skip sanity checks that the server serves 42ity REST API"
    echo "  -m|--method which routine to use from weblib.sh (Default: '$WEBLIB_FUNC')"
    echo "  -ap|--api-prefix URL    Specify the REST API prefix (default '$SUT_API_PREFIX')"
    echo "NOTE: RELATIVE_URL is under the BASE_URL (host:port/apiPrefix)"
}

SUT_is_localhost() {
    [ "$SUT_HOST" = "127.0.0.1" -o "$SUT_HOST" = "localhost" ]
}

SHOW_HELP=no
RELATIVE_URL=""
# SKIP_SANITY=(yes|no|onlyerrors)
[ x"${SKIP_SANITY-}" = x- ] && SKIP_SANITY=""
[ -z "$SKIP_SANITY" ] && SKIP_SANITY=no
while [ $# -gt 0 ] ; do
    case "$1" in
        --port-web|--sut-port-web|-wp|--port)
            SUT_WEB_PORT="$2"
            shift
            ;;
        --host|--machine|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift
            ;;
        --api-prefix|-ap)
            SUT_API_PREFIX="$2"
            shift
            ;;
        --use-https|--sut-web-https)    SUT_WEB_SCHEMA="https"; export SUT_WEB_SCHEMA;;
        --use-http|--sut-web-http)      SUT_WEB_SCHEMA="http"; export SUT_WEB_SCHEMA;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift
            ;;
        -p|--passwd|--bios-passwd|--password|--bios-password)
            BIOS_PASSWD="$2"
            shift
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift
            ;;
        -m|--method)
            WEBLIB_FUNC="$2"
            shift
            ;;
        -q|-f|--force|--quick) SKIP_SANITY=yes ;;
        --help|-h)
            SHOW_HELP=yes
            ;;
        /*) # Assume that an URL follows
            RELATIVE_URL="$1"
            shift # if anything remains, pass it to CURL - e.g. POST data or headers
            break
            ;;
        *)  echo "Unrecognized params follow: $*" \
                "Note that the RELATIVE_URL must start with a slash" >&2
            exit 1
            ;;
    esac
    shift
done

test_web_port() {
    if ! SUT_is_localhost ; then
        logmsg_warn "test_web_port() skipped for non-localhost ($SUT_HOST:$SUT_WEB_PORT)"
        return 0
    fi

    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}

if [ -z "$SUT_WEB_PORT" ]; then
    if SUT_is_localhost ; then
        for SUT_WEB_PORT in 8000 80 443 ; do
            # Note: scriptlib not included yet, so no logmsg_info
            test_web_port && echo "Using port $SUT_WEB_PORT by default (it is listening on localhost)" && break
        done
    else
        SUT_WEB_PORT="443"
    fi
fi

if [ -z "$SUT_WEB_SCHEMA" ]; then
    case "$SUT_WEB_PORT" in
        *443*) SUT_WEB_SCHEMA="https" ;;
        *80*)  SUT_WEB_SCHEMA="http" ;;
        *)
            if SUT_is_localhost ; then
                SUT_WEB_SCHEMA="http"
            else
                SUT_WEB_SCHEMA="https"
            fi ;;
    esac
fi

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh >&2 || \
    { [ "$SHOW_HELP" = yes ] && usage ; \
      echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }

if [ "$SHOW_HELP" = yes ]; then
    usage
    exit 1
fi

NEED_BUILDSUBDIR=no determineDirs_default >&2 || true
NEED_TESTLIB=no

# No "cd" is required for this script to perform
[ -n "$CHECKOUTDIR" ] && { [ -d "$CHECKOUTDIR" ] || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"; }
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to run the requests" >&2

# Included after CLI processing because sets autovars like BASE_URL
. "$SCRIPTDIR/weblib.sh" >&2 || CODE=$? die "Can not include web script library"

logmsg_info "Using BASE_URL='$BASE_URL' to run the requests" >&2

if [ -z "$RELATIVE_URL" ]; then
    if [ "$WEBLIB_FUNC" = "accept_license" ]; then
        RELATIVE_URL=/
    else
        die "No RELATIVE_URL was provided"
    fi
fi

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
  if [ "$SKIP_SANITY" = yes ]; then
    logmsg_info "Skipping sanity checks due to SKIP_SANITY=$SKIP_SANITY" >&2
  else
    if ! pidof saslauthd > /dev/null; then
      SUT_is_localhost && \
      logmsg_warn "saslauthd is not running (locally), you may need to start it first!"
    fi

    if ! pidof malamute > /dev/null; then
      SUT_is_localhost && \
      logmsg_warn "malamute is not running (locally), you may need to start it first!"
    fi

    if ! pidof mysqld > /dev/null ; then
      SUT_is_localhost && \
      logmsg_warn "mysqld is not running (locally), you may need to start it first!"
    fi

    logmsg_info "Waiting for web-server to begin responding..." >&2
    if wait_for_web ; then
      SUT_is_localhost && \
      logmsg_info "Web-server is responsive!" >&2
    else
      die "Web-server at $BASE_URL is NOT responsive!" >&2
    fi

    # Validate the fundamental 42ity webserver capabilities
    logmsg_info "Testing webserver ability to serve the REST API" >&2
    curlfail_push_expect_404
    if [ -n "`api_get "" 2>&1 | grep 'HTTP/.* 500'`" ] >/dev/null 2>&1 ; then
        logmsg_error "api_get() returned an Internal Server Error:"
        api_get "" >&2
        CODE=4 die "Webserver code is deeply broken (maybe missing libraries), please fix it first at $BASE_URL!"
    fi

    if [ -z "`api_get "" 2>&1 | grep 'HTTP/.* 404 Not Found'`" ] >/dev/null 2>&1 ; then
        # We do expect an HTTP-404 on the API base URL
        logmsg_error "api_get() returned an error:"
        api_get "" >&2
        CODE=4 die "Webserver at $BASE_URL is not running or serving the REST API, please start it first!"
    fi
    curlfail_pop

    if [ "$SKIP_SANITY" != onlyerrors ]; then
        curlfail_push_expect_noerrors
        if [ -z "`api_auth_get '/admin/ifaces' 2>&1 | grep 'HTTP/.* 200 OK'`" ] >/dev/null 2>&1 ; then
            # We expect that the login service responds
            logmsg_error "api_auth_get() returned an error:"
            api_auth_get "/admin/ifaces" >&2
            CODE=4 die "Webserver at $BASE_URL is not running or serving the REST API (maybe login data is bad), please start it first!"
        fi
        curlfail_pop
    fi

    logmsg_info "Webserver seems basically able to serve the REST API" >&2
  fi

  logmsg_info "Requesting: '$BASE_URL$RELATIVE_URL' with '$WEBLIB_FUNC' $*" >&2
  "$WEBLIB_FUNC" "$RELATIVE_URL" "$@"
