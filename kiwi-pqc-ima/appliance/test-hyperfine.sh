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

sudo bash /usr/local/bin/measure-boottime.sh
sync

TESTCASE="chromium"
LOGFILE_DIR="/var/log/hyperfine"
LOGFILE_BASEPATH="/var/log/hyperfine/test_$TESTCASE"

TESTRUN_NR=0
LOGFILE_PATH="${LOGFILE_BASEPATH}${TESTRUN_NR}"

mkdir -p "${LOGFILE_DIR}"

if [[ -f "${LOGFILE_PATH}" ]]
then
	TESTRUN_NR=$(ls ${LOGFILE_BASEPATH}* | wc -l)
fi

LOGFILE_PATH="${LOGFILE_BASEPATH}${TESTRUN_NR}"

timeout 120 hyperfine --runs 10 --export-json ${LOGFILE_PATH} "chromium --password-store=basic /opt/testpage.html"

sleep 1
sync

if [[ $TESTRUN_NR -lt $BOOT_TEST_COUNT ]]
then
	sudo reboot
else
	sudo poweroff
fi
