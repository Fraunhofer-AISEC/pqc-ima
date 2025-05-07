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
import pandas as pd
import subprocess
import re
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter,MultipleLocator

def parse_xfs_io_fsmap(file_path):
    try:
        # Execute the command with the provided file path
        result = subprocess.run(['xfs_io', '-c', 'fsmap -v', file_path], capture_output=True, text=True, check=True)
        output = result.stdout
        
        # Parse the output into a list of lines
        lines = output.strip().split('\n')
        
        # Prepare a list to hold parsed data
        data = []
        
        # Regular expression to match the output format
        #pattern = re.compile(r'^(\d+): 7:0 [\s+]:           static fs metadata                    \d  (\s+)               (\d+)$')
        patterns = [
            re.compile(r'^.*\d+: \d:\d \[\d+\.\.\d+\]: +(\d+) +\d+\.\.\d+ +(\d+) +\(\d+\.\.\d+\) +(\d+) (\d+)$'), # 4 groups
            re.compile(r'^.*\d+: \d:\d \[\d+\.\.\d+\]: +(\d+) +\d+\.\.\d+ +(\d+) +\(\d+\.\.\d+\) +(\d+)$'), # 3 groups
            re.compile(r'^.*\d+: \d:\d \[.*\]: +(.*) (\d+) +\(\d+\.\.\d+\) +(\d+)$'), # 3 groups
        ]

        # Process each line
        for line in lines[1:]:  # Skip the header line
            for i,p in enumerate(patterns):
                match = p.match(line)
                if match:
                    mgrp = match.groups()
                    if len(mgrp) == 3:
                        mgrp += (0, ) # set default flags = 0
                    data.append(mgrp)
                    break

        # Create a DataFrame
        df = pd.DataFrame(data, columns=['Owner', 'AG', 'Blocks', 'Flags'])
        df['Owner'] = df['Owner'].str.strip()
        df['AG'] = df['AG'].astype(int)
        df['Blocks'] = df['Blocks'].astype(int)
        df['Flags'] = df['Flags'].astype(int)
        return df

    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        return None


def load_image_xfs(c, case, fs):
    mountpoint, dev = mount_image(c)

    usage = disk_usage(mountpoint)
    data = parse_xfs_io_fsmap(mountpoint)
    umount(mountpoint)

    freespace_strings = ['free space']
    metadata_strings = ['static fs metadata', 'per-AG metadata', 'inode btree', 'refcount btree', 'inodes', 'journalling log', 'extent map']

    freespace = data[data['Owner'].isin(freespace_strings)]
    metadata = data[data['Owner'].str.contains('|'.join(metadata_strings)) | (data['Flags'] == 1000000)]
    used = data[pd.to_numeric(data['Owner'], errors='coerce').notnull() & (data['Flags'] != 1000000)]
    
    freespace_blocks = int(freespace['Blocks'].sum()/8)*4096
    metadata_blocks = int(metadata['Blocks'].sum()/8)*4096
    used_blocks = int(used['Blocks'].sum()/8)*4096
    
    df = pd.DataFrame([(case, fs, used_blocks, metadata_blocks, freespace_blocks)], columns=['case', 'fs', 'used', 'meta', 'free']).set_index('case')
    return (c.name, (usage.used, usage.free), df)
