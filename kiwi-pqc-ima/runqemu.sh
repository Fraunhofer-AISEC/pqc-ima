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

DOCKER_RUN="docker run --privileged --rm -v /dev:/dev -v $(realpath $PWD/..):/mnt -w /mnt/kiwi-pqc-ima pqc-ima"

TIMEOUT="timeout 7200"
#TIMEOUT=

#DISPLAY="-nographic"

DATE_NOW=$(date "+%Y%m%d_%H_%M")
outdir="measurements/$DATE_NOW"
mkdir -p $outdir

run_qemu() {
	local i=$1
	local DISPLAY="-display vnc=127.0.0.1:$i"
	#local DISPLAY="-nographic -serial mon:stdio"
	#set -x
	nice -n -19 su -c "$TIMEOUT taskset -c $((i*4))-$((i*4 + 3)) \
		qemu-system-x86_64 \
		-bios OVMF.fd \
		-enable-kvm \
		-cpu host \
		-smp 2 \
		-m 4G \
		-drive format=raw,file=$2 \
		-net nic -net user \
		$DISPLAY" $SUDO_USER
	#set +x
}

run_test() {
	local i=$1
	echo "$((i*4)) $((i*4 + 3))"
	#echo $2
	nice -n -10 su -c 'sleep 10' $SUDO_USER
}
export -f run_test

run_list() {
	local i=$1
	shift
	local cases=${@}
	for f in ${cases[@]}; do
		#run_test $i $f
		run_qemu $i $f
	done
}

runqemu_main () {
	[[ -z "$SUDO_USER" ]] && die

	if [[ -n "$TEST_FILES" ]]
	then
		files_full=("$TEST_FILES")
	else
		files_full=($(find $TEST_DIR -name kiwi-pqc-ima.x86_64-12.1.raw | sort))
	fi

	files=()
	for f in ${files_full[@]}; do
		dir=$(dirname $(realpath $f))
		if [ -f "$dir/failed" ];
		then
			#echo "Skipping $f"
			continue
		fi
		files+=("$f")
	done

	pids=()
	n=48
	chunk_size=$(( (${#files[@]} + n - 1) / n ))
	for ((i=0; i<n; i++)); do
		start=$((i * chunk_size))
		chunk=("${files[@]:start:chunk_size}")
		if [[ -z "${chunk[@]}" ]]
		then
			continue
		fi
		echo "Chunk $((i)): ${chunk[@]}"
		run_list $i "${chunk[@]}" &
		#run_test $i "${chunk[@]}" &
		pids[${i}]=$!
	done

	# https://stackoverflow.com/a/356154
	for pid in ${pids[*]}; do
	    wait $pid
	done
}

extract_measurements () {
	[[ -z "$SUDO_USER" ]] && die

	if [[ -n "$TEST_FILES" ]]
	then
		files_full=("$TEST_FILES")
	else
		files_full=($(find $TEST_DIR -name kiwi-pqc-ima.x86_64-12.1.raw | sort))
	fi

	for f in ${files_full[@]}; do
		dir=$(dirname $f)

		if [ -f "$dir/failed" ];
		then
			echo "Skipping $f"
			continue
		fi

		mkdir -p $dir/mnt
		offset=$(/usr/sbin/parted $f unit B print | awk "/^ 4/ {print \$2}" | tr 'B' ' ')
		size=$(/usr/sbin/parted $f unit B print | awk "/^ 4/ {print \$4}" | tr 'B' ' ')

		sudo mount -o loop,offset=$offset $f $dir/mnt
		if [[ -f $dir/mnt/boottime ]]
		then
			cp "$dir/mnt/boottime" "$outdir/$(basename $dir).log"
			sync
		fi
		if [[ -d $dir/mnt/hyperfine ]]
		then
			cp -r "$dir/mnt/hyperfine" "$outdir/$(basename $dir)"
			sync
		fi
		sudo umount $dir/mnt
	done

	chown -R $SUDO_USER:$SUDO_USER $outdir
}

disk_usage () {
	test_case=$(basename $TEST_DIR)
	if [[ "$test_case" != "apache" ]] && [[ "$test_case" != "gnome" ]]; then
		echo "Set --dir to either end in apache or gnome"
		die
	fi
	$DOCKER_RUN python3 jupyter/extract_usage.py $test_case $outdir
}

usage () {
	echo "Usage: sudo $0 [-d BASE_DIR] [-f IMAGE]"
}

die () {
	usage
	exit 1
}

TEST_DIR=""
TEST_FILES=""
EXTRACT=y
MEASURE=y
DISK_USAGE=y
while [[ "$#" -gt 0 ]]; do
	case $1 in
		--dir|-d)
			TEST_DIR="$2"
			shift ;;
		--file|-f)
			TEST_FILES="$TEST_FILES $2"
			shift ;;
		--extract-only|-e)
			EXTRACT=y
			MEASURE=n
			DISK_USAGE=n
			shift ;;
		--measure-only|-m)
			EXTRACT=n
			MEASURE=y
			DISK_USAGE=n
			shift ;;
		--disk-usage)
			EXTRACT=n
			MEASURE=n
			DISK_USAGE=y
			shift ;;
		*)
			echo "Unknown parameter passed: $1"; exit 1 ;;
	esac
	shift
done

[[ -z "$TEST_DIR$TEST_FILES" ]] && die

[[ "$DISK_USAGE" = "y" ]] && disk_usage
[[ "$MEASURE" = "y" ]] && runqemu_main
[[ "$EXTRACT" = "y" ]] && extract_measurements
