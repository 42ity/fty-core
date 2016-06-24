# Set up history tracking for BASH
# Partially inspired by
#   http://www.pointsoftware.ch/howto-bash-audit-command-logger/
if [ -n "${BASH-}" ]; then
    # 'history' options
    declare -rx HISTFILE="$HOME/.bash_history"
    declare -rx HISTSIZE=500000       #nbr of cmds in memory
    declare -rx HISTFILESIZE=500000   #nbr of cmds on file
    declare -rx HISTCONTROL=""        #does not ignore spaces or duplicates
    declare -rx HISTIGNORE=""         #does not ignore patterns
    declare -rx HISTCMD               #history line number
    #history -r                       #to reload history from file if a prior HISTSIZE has truncated it

    # Ensure the file exists (even if empty), as we use it with "fc" below
    # Secure the file to minimize leak of sensitive data (mis-pasted passwords etc.)
    [ -f "$HISTFILE" ] || { touch "$HISTFILE" ; chmod 600 "$HISTFILE"; }

    shopt -s histappend
    shopt -s cmdhist
    set -o history

    #history substitution ask for a confirmation
    shopt -s histverify
fi

