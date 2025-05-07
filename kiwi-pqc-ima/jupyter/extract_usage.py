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

from sh import mount, umount
from pathlib import Path
import subprocess
import re
import pandas as pd
from IPython.display import display
from multiprocessing import Pool
import os.path
import numpy as np
import sys
import pickle

from part_util import *
from ima_ext4 import *
from ima_btrfs import *
from ima_xfs import *
from ima_f2fs import *
from ima_bcachefs import *

def check_success(c):
    return not os.path.isfile(c / "failed")

def load_image(c):
    if str(c.name).count("_") == 2:
        fs, cipher, app = tuple(str(c.name).split('_'))
        case = f"{cipher}_{app}"
    elif str(c.name).count("_") == 1:
        fs, app = tuple(str(c.name).split('_'))
        case = f"none_{app}"
    if fs == "btrfs":
        return load_image_btrfs(c, case, fs) + (check_success(c),)
    elif fs == "ext4":
        return load_image_ext4(c, case, fs) + (check_success(c),)
    elif fs == "xfs":
        return load_image_xfs(c, case, fs) + (check_success(c),)
    elif fs == "bcachefs":
        return load_image_bcachefs(c, case, fs) + (check_success(c),)
    elif fs == "f2fs":
        return load_image_f2fs(c, case, fs) + (check_success(c),)


def load_scenario(base_dir, fs, use_case):
    p = Path(f"{base_dir}/{use_case}")
    base_case = list(p.glob(f"{fs}_{use_case}"))[0]
    all_cases = sorted(list(p.glob(f"{fs}_*_*")))

    if fs == "btrfs":
        base_name, base_used, base_total, base_stats, base_success = load_image(base_case)

        with Pool(20) as p:
            all_data = p.map(load_image, [base_case] + all_cases)
        all_names, all_usage, all_totals, all_stats, all_success = tuple(map(list, zip(*all_data)))

        df_total = pd.DataFrame(columns=['total'], index=all_names)
        df_stats = pd.DataFrame(columns=base_stats.columns, index=all_names)
    
        df_total.loc[base_name] = pd.Series({'total': base_total})
        df_stats.loc[base_name] = base_stats.loc[0]

        for c in all_cases:
            name, usage, total, stats, success = load_image(c)
            df_total.loc[name] = pd.Series({'total': total})
            df_stats.loc[name] = stats.loc[0]
        
        tmp = pd.DataFrame()
        tmp['used'] = df_stats[('Data', 'used')]
        tmp['used_fragmented'] = df_stats[('Data', 'total')] - df_stats[('Data', 'used')]
        tmp['meta'] = df_stats[('Metadata', 'used')] + df_stats[('System', 'used')] + df_stats[('GlobalReserve', 'used')]
        tmp['meta_fragmented'] = df_stats[('Metadata', 'total')] + df_stats[('System', 'total')] + df_stats[('GlobalReserve', 'total')] - tmp['meta']
        tmp['free'] = df_total['total'] - tmp['used'] - tmp['used_fragmented'] - tmp['meta'] - tmp['meta_fragmented']
        tmp = tmp.astype(int)
        tmp['fs'] = fs
    
        tmp = tmp.reset_index().rename(columns={'index': 'case'})
        tmp['success'] = all_success
        return tmp
    elif fs == "ext4":
        with Pool(20) as p:
            all_data = p.map(load_image, [base_case] + all_cases)
        all_names, all_usage, all_stats, all_success = tuple(map(list, zip(*all_data)))
        data = pd.concat(all_stats).astype(int)

        tmp = pd.DataFrame()
        tmp['used'] = data['used']
        tmp['used_fragmented'] = 0
        tmp['meta'] = data['meta']
        tmp['meta_fragmented'] = 0
        tmp['free'] = data['free']
        tmp['fs'] = fs

        tmp = tmp.reset_index().rename(columns={'index': 'case'})
        tmp['success'] = all_success
        return tmp
    elif fs == "xfs":
        all_data = [load_image(c) for c in [base_case] + all_cases]
        all_names, all_usage, all_stats, all_success = tuple(map(list, zip(*all_data)))
        tmp = pd.concat(all_stats)
        tmp['success'] = all_success
        return tmp
    elif fs == "bcachefs":
        all_data = [load_image(c) for c in [base_case] + all_cases]
        all_names, all_usage, all_stats, all_success = tuple(map(list, zip(*all_data)))
        tmp = pd.concat(all_stats)
        tmp['success'] = all_success
        return tmp
    elif fs == "f2fs":
        all_data = [load_image(c) for c in [base_case] + all_cases]
        all_names, all_usage, all_stats, all_success = tuple(map(list, zip(*all_data)))
        tmp = pd.concat(all_stats)
        tmp['success'] = all_success
        return tmp

def load_all(base_dir, case):
    data = {}
    data["btrfs"] = load_scenario(base_dir, "btrfs", case)
    data["ext4"] = load_scenario(base_dir, "ext4", case)
    data["xfs"] = load_scenario(base_dir, "xfs", case)
    data["bcachefs"] = load_scenario(base_dir, "bcachefs", case)
    data["f2fs"] = load_scenario(base_dir, "f2fs", case)
    return data

testcase = sys.argv[1]
outfile = f"{sys.argv[2]}/ima_usage"

data_apache = load_all(f"out-kiwi/2024_12_30", testcase)
print(data_apache)

with open(outfile, 'wb') as f:
    pickle.dump(data_apache, f)
