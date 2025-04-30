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

FROM debian:bookworm

RUN apt-get update -y && apt-get install -y build-essential libssl-dev libkeyutils-dev libtasn1-dev libgmp-dev libnspr4-dev libnss3-dev pkg-config git python3 python3-pip cmake automake libtool bison flex
RUN apt-get -y install bc cpio kmod libelf-dev rsync
RUN apt-get -y install debhelper pahole
RUN apt-get -y install qemu-utils gdisk systemd kpartx
RUN apt-get -y install dosfstools parted pigz
RUN apt-get -y install btrfs-progs xfsprogs f2fs-tools gawk
RUN apt-get -y install attr

RUN apt install -y pkg-config libaio-dev libblkid-dev libkeyutils-dev \
    liblz4-dev libsodium-dev liburcu-dev libzstd-dev \
    uuid-dev zlib1g-dev valgrind libudev-dev udev git build-essential \
    python3 python3-docutils libclang-dev debhelper dh-python

ADD deps /root/deps
WORKDIR /root/deps

RUN ./install.sh build_botan
RUN ./install.sh build_evmctl
RUN ./install.sh build_oqs
RUN ./install.sh build_oqsprovider
RUN ./install.sh enable_oqsprovider
RUN ./install.sh install_kiwi
RUN ./install.sh install_rust
RUN ./install.sh install_bcachefs
RUN ./install.sh config_git
RUN ./install.sh install_jupyter

WORKDIR /mnt
