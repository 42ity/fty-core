#!/bin/bash

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

#! \file verify-fs.sh
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \brief Verify the filesystem and report suspicious things
#

OVERLAY=/mnt/nand/overlay
DEFAULT=/etc/default/verify-fs

die () {
    echo "${@}" >&2
    exit 1
}

is_tree_empty () {
    local dir excludes
    dir=${1}

    # build chain of -not -path foo -or -not -path bar from args
    shift 1
    [[ -n ${1} ]] && excludes="-not -path '${1}'"
    shift 1
    while [[ -n ${1} ]]; do
        excludes="${excludes} -or -not -path '${1}'"
        shift 1
    done

    [[ $(find "${dir}" -xdev -type f ${excludes} | wc -l) == 0 ]]

}

log () {
    /usr/bin/logger -i -p security.err -t "verify-fs" "${@}"
}

cat_default () {
    if [[ ! -f "${DEFAULT}" ]]; then
        cat <<EOF
bin:
sbin:
lib:
lib64:
usr:local
EOF
        return
    fi
    /bin/grep -v '^#' "${DEFAULT}"
}

##### MAIN #####

[[ $(< /proc/mounts) =~ upperdir=${OVERLAY} ]] || \
    die "Can't find overlay ${OVERLAY}"

cat_default | while read LINE; do
    TREE=${LINE%%:*}
    EXCLUDES=${LINE#*:}

    [[ -d "${OVERLAY}/${TREE}" ]] || continue

    is_tree_empty "${OVERLAY}/${TREE}" ${EXCLUDES//:/ } || \
        log "${OVERLAY}/${TREE} contains suspicious files."
done
