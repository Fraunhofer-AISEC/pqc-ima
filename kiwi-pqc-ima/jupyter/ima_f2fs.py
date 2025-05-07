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

from part_util import mount_image
from sh import umount
from shutil import disk_usage
import subprocess
import re
import pandas as pd

def parse_f2fs_sit(file_path):
    try:
        # Execute the command with the provided file path
        result = subprocess.run(['dump.f2fs', '-s', '0~-1', file_path], capture_output=True, text=True, check=True)
        
        # Regular expression to match the output format
        # segno:    2537  vblocks:  0     seg_type:0      sit_pack:1
        #pattern = re.compile(r'^segno: *(\d+)  vblocks: *(\d+) *seg_type:(\d) *sit_pack:\d+$')
        pattern = re.compile(r'segno: *\d+\s+vblocks:\s*(\d+)\s*seg_type:(\d)\s*sit_pack:\d+')
        
        with open('dump_sit', 'r') as f:
            sitin = f.read()

        return pd.DataFrame.from_records(pattern.findall(sitin), columns=['vblocks', 'seg_type']).astype(int)

    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        return None

def get_f2fs_main_blkaddr(file_path):
    try:
        # Execute the command with the provided file path
        result = subprocess.run(['dump.f2fs', '-d1', file_path], capture_output=True, text=True, check=True)
        output = result.stdout

        pattern = re.compile(r'main_blkaddr\s+\[0x\s+\S+ : (\d+)\]')

        main_blkaddr_match = pattern.findall(output)
        if len(main_blkaddr_match) > 0:
            return int(main_blkaddr_match[0])
    
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        return None
        
def load_image_f2fs(c, case, fs):
    mountpoint, dev = mount_image(c)

    usage = disk_usage(mountpoint)
    data = parse_f2fs_sit(dev)
    main_blkaddr = get_f2fs_main_blkaddr(dev)
    umount(mountpoint)

    section_count = len(data)
    total_blocks = main_blkaddr + 512 * section_count
    
    data = data.groupby('seg_type').sum().reset_index()
    
    used_bytes = data[data['seg_type'].isin([0,1,2])]['vblocks'].sum()*4096
    metadata_bytes = data[data['seg_type'].isin([3,4,5])]['vblocks'].sum()*4096 + main_blkaddr*4096
    freespace_bytes = total_blocks * 4096 - used_bytes - metadata_bytes

    df = pd.DataFrame([(case, fs, used_bytes, metadata_bytes, freespace_bytes)], columns=['case', 'fs', 'used', 'meta', 'free']).set_index('case')
    return (c.name, (usage.used, usage.free), df)
