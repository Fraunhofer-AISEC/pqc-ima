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

import ext4

def parse_ext4(dev, case):
    file = open(dev, "rb")
    volume = ext4.Volume(file, offset=0)
    inode_count = volume.superblock.s_inodes_count

    eblocks_data = 0
    eblocks_meta = 0
    for i in range(1, inode_count+1):
        try:
            inode = volume.inodes[i]
            for ex in inode.extents:
                # META: EAs + DIRS + everything in special inodes (e.g. journal)
                if (inode.i_flags & ext4.enum.EA_INODE) or i < 11 or (inode.i_mode & ext4.enum.IFDIR):
                    eblocks_meta += ex.blocks.ee_len
                else: #DATA
                    eblocks_data += ex.blocks.ee_len
        except:
            continue
    
    total_blocks = volume.superblock.s_blocks_count_lo
    
    free_blocks = volume.superblock.s_free_blocks_count
    data_blocks = eblocks_data
    metadata_blocks = total_blocks - data_blocks - free_blocks
    file.close()

    tmp = pd.DataFrame([(case, free_blocks*volume.block_size, metadata_blocks*volume.block_size, data_blocks*volume.block_size)],
                       columns=['case', 'free', 'meta', 'used']).set_index('case')
    
    return tmp


def load_image_ext4(c, case, fs):
    mountpoint, dev = mount_image(c)

    usage = disk_usage(mountpoint)

    df = parse_ext4(dev, case)
    
    umount(mountpoint)
    return (case, (usage.used, usage.free), df)


def plot_ext4(ext4_apache, show_table=False):
    selected_columns = ext4_apache[['Inode count', 'Free inodes', 'Block count', 'Free blocks', 'Block size']].copy()
    selected_columns['Used blocks'] = ext4_apache['Block count'] - ext4_apache['Free blocks']
    selected_columns['Used inodes'] = ext4_apache['Inode count'] - ext4_apache['Free inodes']
    selected_columns = selected_columns # * 4096
    if show_table:
        display(selected_columns)

    color_palette = ['#1F82C0', '#DDDFE0']
    fig,ax = plt.subplots(1, 1, sharey=True, figsize=(25, 7))
    selected_columns[['Used blocks', 'Free blocks']].plot.barh(stacked=True, ax=ax, color=color_palette)
    
    def to_gib(x, pos):
            return f'{x / (2**30):.1f} GiB'
    
    # Set the formatter for the y-axis
    formatter = FuncFormatter(to_gib)
    ax.xaxis.set_major_formatter(formatter)
    ax.xaxis.set_major_locator(MultipleLocator(2**30))

    return ax


def plot_ext4_inodes(ext4_apache):
    color_palette = ['#1F82C0', '#DDDFE0']
    fig,ax = plt.subplots(1, 1, sharey=True, figsize=(25, 8))
    selected_columns[['Used inodes', 'Free inodes']].plot.barh(stacked=True, ax=ax, color=color_palette)

    return ax
