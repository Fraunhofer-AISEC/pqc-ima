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

IMGFILE="kiwi-pqc-ima.x86_64-12.1.raw"

mount_part() {
	index=$1
	subdir=$2
	set_uuid=$3

	mkdir -p $DSTDIR/$subdir
	offset=$(parted $DSTDIR/$IMGFILE unit B print | awk "/^ $index/ {print \$2}" | tr 'B' ' ')
	loopdev=$(losetup --offset $offset --find --show $DSTDIR/$IMGFILE)
	mount $loopdev $DSTDIR/$subdir
	echo $loopdev
}

STARTDIR=$(pwd)
OUTDIR=$1
fs=$2
app=$3

SRCDIR=$OUTDIR/${app}/${fs}_${app}/
DSTDIR=$OUTDIR/${app}/${fs}_${app}/

mount_part 2 mnt_efi false
cd $SRCDIR
sd_boot_entry=$(ls mnt_efi/loader/entries/debian* | head -1)
[[ "$fs" = "ext4" ]] && sed -i '/^options/s/$/ rd.modules_load=ext4/' $sd_boot_entry
[[ "$fs" = "xfs" ]] && sed -i '/^options/s/$/ rd.modules_load=ext4,xfs/' $sd_boot_entry
[[ "$fs" = "btrfs" ]] && sed -i '/^options/s/$/ rd.modules_load=ext4,btrfs/' $sd_boot_entry
[[ "$fs" = "f2fs" ]] && sed -i '/^options/s/$/ rd.modules_load=ext4,f2fs/' $sd_boot_entry
[[ "$fs" = "bcachefs" ]] && sed -i '/^options/s/$/ rd.modules_load=ext4,bcachefs/' $sd_boot_entry
cd $STARTDIR
sync $SRCDIR/mnt_efi
umount $SRCDIR/mnt_efi
losetup -d $loopdev
