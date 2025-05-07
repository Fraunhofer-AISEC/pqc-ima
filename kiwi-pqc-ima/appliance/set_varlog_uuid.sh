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

root_device=$2
loop_name=$(basename $root_device | cut -f 1-2 -d'p')
disk_device=/dev/${loop_name}
root_dir=$(findmnt -nr -o target -S $root_device | head -n1)

# determine if $root_dir contains a btrfs volume
root_type=$(blkid $root_device -o value -s TYPE)
if [ "$root_type" = "btrfs" ]; then
	shim=@/
else
	shim=
fi

varlog_partuuid=$(awk 'match($0, /PARTUUID=(.*) \/var\/log/, a) {print a[1]}' $root_dir/${shim}etc/fstab)
varlog_uuid=$(blkid --match-token PARTUUID=$varlog_partuuid -o value -s UUID)
sed -i "s/##FSUUID##/$varlog_uuid/g" $root_dir/${shim}etc/ima/ima-policy.off
