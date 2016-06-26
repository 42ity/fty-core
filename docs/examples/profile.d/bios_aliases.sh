# A few helper aliases
if [ -n "${BASH-}" ]; then
    alias dbb='mysql -u root box_utf8 -t'
    alias la='ls -la'
    alias renet='sudo systemctl restart bios-networking; sleep 5; ip a; ip r'
fi
