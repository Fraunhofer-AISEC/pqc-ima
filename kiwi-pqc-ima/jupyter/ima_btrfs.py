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
import json


def btrfs_show(device):
    try:
        # Run the btrfs command
        result = subprocess.run(
            ['btrfs', 'filesystem', 'show', '--raw', device],
            capture_output=True,
            text=True,
            check=True
        )

        output = result.stdout
        for line in output.splitlines():
            if "devid" in line:
                total_size = line.split()[3]  # Adjust index based on output format

        # Extract the free space information
        return int(total_size)
        
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
    
    return None

def btrfs_df(device):
    try:
        # Run the btrfs command
        result = subprocess.run(
            ['btrfs', '--format=json', 'filesystem', 'df', device],
            capture_output=True,
            text=True,
            check=True
        )

        data = json.loads(result.stdout)
        df = pd.json_normalize(data["filesystem-df"])
        df = df.set_index('bg-type')
        df = df.drop(['bg-profile'], axis=1)
        df = df.apply(pd.to_numeric)

        combinations = pd.MultiIndex.from_product([df.index, df.columns], names=['Row', 'Column'])
        result = pd.DataFrame(index=[0], columns=combinations)
        for row in df.index:
            for col in df.columns:
                result[(row, col)] = df.at[row, col]
        
        return result
        
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        
    
    return None

def load_image_btrfs(c, case, fs):
    mountpoint, dev = mount_image(c)

    usage = disk_usage(mountpoint)
    
    total_size = btrfs_show(mountpoint)
    df = btrfs_df(mountpoint)
    
    umount(mountpoint)
    return (case, (usage.used, usage.free), total_size, df)
