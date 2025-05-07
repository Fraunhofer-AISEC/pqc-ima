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

export OMP_NUM_THREADS=8

do_evmctl () {
	key=$2

	if [[ $key =~ "hsslms" ]]
	then
		echo "$key"
		EVMCTL_USE_BOTAN=1 evmctl ima_sign -r --key $(realpath appliance/pki/$2/$2.key) --keyid A519789D $1 > /dev/null 2>&1
		ret=$?
	else
		evmctl ima_sign -r --key $(realpath appliance/pki/$2/$2.key) $1 > /dev/null 2>&1
		ret=$?
	fi

	if [[ "$ret" != "0" ]]
	then
		return 1
	fi
}

do_ima_sign() {
	pfx=$1
	KIWI_IMA_SIG_ALGO=$2

	do_evmctl $pfx $KIWI_IMA_SIG_ALGO
	if [[ "$ret" != "0" ]]
	then
		echo "$KIWI_IMA_SIG_ALGO failed during IMA signing."
		return 1
	fi
}

arm_ima () {
	mv $1/etc/ima/ima-policy.off $1/etc/ima/ima-policy
}

set_ima_cert () {
	mkdir -p $1/etc/keys
	cp appliance/pki/$2/${2}_crt.der $1/etc/keys/x509_ima.der
}

mount_part() {
	index=$1
	subdir=$2
	set_uuid=$3
	mount_opts="$4"

	mkdir -p $DSTDIR/$subdir
	offset=$(parted $DSTDIR/$IMGFILE unit B print | awk "/^ $index/ {print \$2}" | tr 'B' ' ')
	loopdev=$(losetup --offset $offset --find --show $DSTDIR/$IMGFILE)
	if [[ "$set_uuid" = true ]]
	then
		[[ "$fs" = "ext4" ]] && tune2fs -O ea_inode $loopdev
		[[ "$fs" = "btrfs" ]] && btrfstune -f -u $loopdev
		[[ "$fs" = "xfs" ]] && xfs_admin -U generate $loopdev
	fi
	mount $loopdev $mount_opts $DSTDIR/$subdir
	echo $loopdev
}

STARTDIR=$(pwd)
OUTDIR=$1
fs=$2
cipher=$3
app=$4

SRCDIR=$OUTDIR/${app}/${fs}_${app}/
DSTDIR=$OUTDIR/${app}/${fs}_${cipher}_${app}

mkdir -p $DSTDIR
cp -r --reflink=auto $SRCDIR/$IMGFILE $DSTDIR/$IMGFILE

rm -f $DSTDIR/failed

mount_opts=
if [[ "$fs" = "f2fs" ]]
then
	mount_opts="-o inline_xattr,inline_xattr_size=900"
fi

mount_part 5 mnt true "$mount_opts"
loopdev_mnt=$loopdev

arm_ima $DSTDIR/mnt
set_ima_cert $DSTDIR/mnt $cipher
do_ima_sign $DSTDIR/mnt $cipher
if [[ "$?" != "0" ]]
then
	touch $DSTDIR/failed
	umount $DSTDIR/mnt
	losetup -d $loopdev
	exit 0 #do not failed the entire build process just log
fi

mount_part 2 mnt_efi false ""
cd $DSTDIR/mnt
initrd=$(ls ../mnt_efi/os/initrd* | head -1)
sd_boot_entry=$(ls ../mnt_efi/loader/entries/debian* | head -1)
echo "$initrd"
find etc/keys | cpio -H newc -o | gzip >> $initrd
[[ "$fs" = "bcachefs" ]] && sed -i '/^options/s/$/ rd.modules_load=bcachefs/' $sd_boot_entry
cd $STARTDIR
sync $DSTDIR/mnt_efi
umount $DSTDIR/mnt_efi
losetup -d $loopdev

umount $DSTDIR/mnt
losetup -d $loopdev_mnt

pigz --force --rsyncable --keep $DSTDIR/$IMGFILE
