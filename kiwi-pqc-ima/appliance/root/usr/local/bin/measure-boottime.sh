#!/bin/bash
#
# Copyright(c) 2025 Fraunhofer AISEC
# Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2 (GPL 2), as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GPL 2 license for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>
#

source /var/log/boottimeenv

systemd-analyze | grep -oP '\d+\.\d+s|\d+ms' | tr '\n' ',' | sed 's/,$/\n/g' >> /var/log/boottime

if [[ -f "/usr/local/bin/test-hyperfine.sh" ]]
then
	exit 0
fi

count=$(cat /var/log/boottime | wc -l)
if [[ $count -lt $BOOT_TEST_COUNT ]]
then
	reboot
else
	poweroff
fi
