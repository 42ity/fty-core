# Set up history syslogging for BASH
# Partially inspired by
#   http://www.pointsoftware.ch/howto-bash-audit-command-logger/
#   http://askubuntu.com/questions/93566/how-to-log-all-bash-commands-by-all-users-on-a-server
if [ -n "${BASH-}" ]; then
    # Only log different history entries (e.g. pressing "enter" also triggers this activity)
    # We use 'echo _LAST_LOGGED' without quotes to quickly chomp surrounding whitespaces
    _LAST_LOGGED=""
    for _LOGGER in /usr/bin/logger /bin/logger ; do
        [ -x "${_LOGGER}" ] && \
        export PROMPT_COMMAND='RETRN_VAL=$?; [ "$(fc -ln -0)" = "$(fc -ln -1)" -o "$(fc -ln -0)" = "${_LAST_LOGGED}" ] || { _LAST_LOGGED="$(fc -ln -0)"; '"${_LOGGER}"' -p local6.debug -t "bash[$$]" "$(whoami)($USER ${UID-}/${EUID-}:${GID-}):" "`echo $_LAST_LOGGED`" "[$RETRN_VAL]"; }' \
        && break
    done
    unset _LOGGER
fi

