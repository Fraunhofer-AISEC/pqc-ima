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

import subprocess
from pathlib import Path
from sh import mount, umount

def get_partition_offset(device, partition_number):
    try:
        # Run the parted command
        result = subprocess.run(
            ['parted', device, 'unit', 'B', 'print'],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Split the output into lines
        lines = result.stdout.splitlines()
        
        # Find the line corresponding to the specified partition number
        for line in lines:
            if line.startswith(f' {partition_number} '):
                # Split the line and return the offset (second column)
                return line.split()[1].rstrip('B')
                
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
    
    return None


def get_block_device(mount_point):
    try:
        # Run the findmnt command
        result = subprocess.run(
            ['findmnt', '-n', '-o', 'SOURCE', mount_point],
            capture_output=True,
            text=True,
            check=True
        )
        
        return result.stdout.strip()  # Return the block device without trailing whitespace
    
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        return None


def mount_image(c):
    mountpoint = c.joinpath(Path('mnt'))
    image = c.joinpath(Path('kiwi-pqc-ima.x86_64-12.1.raw'))
    offset = get_partition_offset(str(image), 5)
    mountpoint.mkdir(exist_ok=True)
    mount("-o", f"loop,offset={offset}", image, mountpoint)
    dev = get_block_device(mountpoint).rstrip('[/@]')
    return mountpoint, dev
