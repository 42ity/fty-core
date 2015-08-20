#!/bin/sh
#!/bin/sh

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


#!
# \file dshell.sh
# \author Jim Klimov
# \author Karol Hrdina
# \author Michal Vyskocil
# \brief Not yet documented file

#!/bin/sh

# dshell binary wrapper 

# TODO
# - usage

PATH="${BUILDSUBDIR}/:`dirname $0`/:`dirname $0`/..:${CHECKOUTDIR}/tools:$CHECKOUTDIR/Installation/usr/bin:/usr/bin:$PATH"
export PATH
dsh="`which dshell`"
if [ $? != 0 ]; then
        echo "FATAL: dshell binary not found in PATH='$PATH'" >&2
        exit 1
fi
echo "INFO: Using dshell binary '$dsh'" >&2

exec ${dsh} ipc://@/malamute 1000 mshell "$1" "$2"
