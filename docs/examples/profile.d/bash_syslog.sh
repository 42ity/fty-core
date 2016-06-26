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
        declare -rx PROMPT_COMMAND='RETRN_VAL=$?; [ "$(fc -ln -0 2>/dev/null)" = "$(fc -ln -1 2>/dev/null)" -o "$(fc -ln -0 2>/dev/null)" = "${_LAST_LOGGED}" ] || { _LAST_LOGGED="$(fc -ln -0 2>/dev/null)"; '"${_LOGGER}"' -p local6.debug -t "bash[$$]" "$(whoami)($USER ${UID-}/${EUID-}:${GID-}):" "`echo $_LAST_LOGGED`" "[$RETRN_VAL]"; }' \
        && export PROMPT_COMMAND && break
    done
    unset _LOGGER
fi

