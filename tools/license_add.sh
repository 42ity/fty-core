#!/bin/sh

BASEDIR="$1"
[ -n "$BASEDIR" ] || BASEDIR="`pwd`"
FILE="`mktemp -t bios_license.XXXXXX`"

find "$BASEDIR" -name '*.sh' -o -name '*.sh.in' | while read fl; do
    if [ -z "`grep 'Copyright ' "$fl"`" ]; then
        sed '1 p' "$fl" | grep '^#!' > "$FILE"
        cat - "$fl" >> "$FILE" << EOF

#
# Copyright (C) 2015 Eaton
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

EOF
        cat "$FILE" > "$fl"
    fi
done
find "$BASEDIR" -name '*.c' -o -name '*.cc' -o -name '*.h' | while read fl; do
    if [ -z "`grep 'Copyright ' "$fl"`" ]; then
        cat - "$fl" > "$FILE" << EOF
/*

Copyright (C) 2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
EOF
        cat "$FILE" > "$fl"
    fi
done
find "$BASEDIR" -name '*.ecpp' | while read fl; do
    if [ -z "`grep 'Copyright ' "$fl"`" ]; then
        cat - "$fl" > "$FILE" << EOF
<#

Copyright (C) 2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#>
EOF
        cat "$FILE" > "$fl"
    fi
done
rm "$FILE"
