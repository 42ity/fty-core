#!/bin/sh

#
# Copyright (C) 2015 - 2020 Eaton
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

#! \file license_add.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \brief Not yet documented file

BASEDIR="$1"
[ -n "$BASEDIR" ] || BASEDIR="`pwd`"
FILE="`mktemp -t bios_license.XXXXXX`"

add_license() {
    SH=""
    SD=""
    ED=""
    MD=""
    SDD=""
    EDD=""
    MDD=""
    case "$1" in
        *.sh|*.sh.in)
            MD="#"
            SH="true"
            SDD="#!"
            MDD="#"
            ;;
        *.cc|*.h|*.c|*.cc.in|*.h.in|*.c.in)
            SD="/*"
            MD=" *"
            ED=" */"
            SDD="/*!"
            MDD=" *"
            EDD=" */"
            ;;
        *.ecpp|*.ecpp.in)
            SD="<#"
            MD=" #"
            ED=" #>"
            SDD="<#\n/*!"
            MDD=" *"
            EDD=" */\n#>"
            ;;
        *)
            echo "Skipping invalid file $1"
            continue;
            ;;
    esac
    if [ -z "`grep "Copyright " "$1"`" ]; then
        echo -n '' > "$FILE"
        [ -z "$SH" ] || sed '1 p' "$fl" | grep '^#!' > "$FILE"
        cat - "$fl" >> "$FILE" << EOF
$SD
$MD
$MD Copyright (C) `date +%Y` Eaton
$MD
$MD This program is free software; you can redistribute it and/or modify
$MD it under the terms of the GNU General Public License as published by
$MD the Free Software Foundation; either version 2 of the License, or
$MD (at your option) any later version.
$MD
$MD This program is distributed in the hope that it will be useful,
$MD but WITHOUT ANY WARRANTY; without even the implied warranty of
$MD MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$MD GNU General Public License for more details.
$MD
$MD You should have received a copy of the GNU General Public License along
$MD with this program; if not, write to the Free Software Foundation, Inc.,
$MD 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
$MD
$ED

$SDD \\file   `basename $1`
`git blame -c "$1" | sed 's|.*(\([^\t]*\)\t.*|\1|' | sort | uniq -c | sort -nr | sed "s|^[[:blank:]]*[0-9]\\+[[:blank:]]\\(.*\)\+[[:blank:]]\\(.*\\)|$MDD\\  \\\\\\\\author\\ \\1 \\2 <\\1\\2@Eaton.com>|" | sed "s,JimKlimov@Eaton.com,EvgenyKlimov@Eaton.com,"`
$MDD \\brief  TODO: Not yet documented file
$EDD
EOF
        cat "$FILE" > "$fl"
    fi
}

find "$BASEDIR" -name '*.sh' -o -name '*.sh.in' -o \
    -name '*.cc' -o -name '*.h' -o -name '*.c' -o -name '*.ecpp' \
    -name '*.cc.in' -o -name '*.h.in' -o -name '*.c.in' -o -name '*.ecpp.in' \
| while read fl; do
    add_license "$fl"
done
rm "$FILE"
