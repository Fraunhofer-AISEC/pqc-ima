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


def parse_bcachefs_usage(mount_point):
    command = f"bcachefs fs usage {mount_point}"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    
    if result.returncode != 0:
        raise Exception("Command failed: " + result.stderr)

    output = result.stdout.splitlines()
    usage_data = {}
    
    # Parse the general filesystem information
    usage_data['filesystem'] = output[0].split(": ")[1]
    usage_data['size'] = int(output[1].split(": ")[1].strip())
    usage_data['used'] = int(output[2].split(": ")[1].strip())
    usage_data['online_reserved'] = int(output[3].split(": ")[1].strip())
    
    # Parse Btree usage using regular expressions
    usage_data['btree_usage'] = {}
    btree_usage_section = False
    
    for line in output:
        if "Btree usage:" in line:
            btree_usage_section = True
            continue
        if btree_usage_section:
            if line.strip() == "":
                break  # Exit on empty line
            match = re.match(r'(\w+):\s+(\d+)', line)
            if match:
                key = match.group(1).strip()
                value = int(match.group(2).strip())
                usage_data['btree_usage'][key] = value

    # Parse device information
    usage_data['device'] = {}
    device_section = False
    
    for line in output:
        if "(no label)" in line:
            device_section = True
            continue
        if device_section:
            if line.strip() == "":
                break  # Exit on empty line
            match = re.match(r'\s*(\w+):\s+(\d+)\s+(\d+)(?:\s+(\d+))?', line)
            if match:
                key = match.group(1).strip()
                data_value = int(match.group(2).strip())
                buckets_value = int(match.group(3).strip())
                fragmented_value = int(match.group(4).strip()) if match.group(4) else None

                usage_data['device'][key] = {
                    'data': data_value,
                    'buckets': buckets_value,
                    'fragmented': fragmented_value
                }
    
    return usage_data

def load_image_bcachefs(c, case, fs):
    mountpoint, dev = mount_image(c)

    usage = disk_usage(mountpoint)
    data = parse_bcachefs_usage(mountpoint)
    umount(mountpoint)

    used_strings = ['user']
    metadata_strings = ['btree', 'cached', 'journal', 'need_discard', 'need_gc_gens', 'parity', 'sb', 'stripe', 'unstriped']
    free_strings = ['free']

    used = sum((data['device'][s]['data'] for s in used_strings))
    used_fragmented = sum((data['device'][s]['fragmented'] for s in used_strings if data['device'][s]['fragmented']))
    metadata = sum((data['device'][s]['data'] for s in metadata_strings))
    metadata_fragmented = sum((data['device'][s]['fragmented'] for s in metadata_strings if data['device'][s]['fragmented']))
    free = sum((data['device'][s]['data'] for s in free_strings))

    df = pd.DataFrame([(case, fs, used,used_fragmented,metadata,metadata_fragmented,free)],
        columns=['case', 'fs', 'used', 'used_fragmented', 'meta', 'meta_fragmented', 'free']).set_index('case')
    return (c.name, usage, df)


def plot_bcachefs(df, xstep=1):
    color_palette = ['#1F82C0', '#79B4D9', '#009374', '#66BFAC', '#DDDFE0']
    fig,ax = plt.subplots(1, 1, sharey=True, figsize=(25, 7))

    df.plot.barh(stacked=True, ax=ax, legend=False, color=color_palette)

    def to_gib(x, pos):
        return f'{x / (2**30):.1f} GiB'

    # Set the formatter for the y-axis
    formatter = FuncFormatter(to_gib)
    ax.xaxis.set_major_formatter(formatter)
    ax.xaxis.set_major_locator(MultipleLocator(xstep*2**30))

    return ax
