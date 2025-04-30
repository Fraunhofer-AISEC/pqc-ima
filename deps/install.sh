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

set -e

build_botan () {
	echo "Build botan..."
	git clone https://github.com/randombit/botan.git
	cd botan
	./configure.py --prefix=/usr
	make -j $(($(nproc)-1))
	make install
	cd ..
}

build_evmctl () {
	echo "Build evmctl..."
	pwd
	cd ima-evm-utils
	set +e
	autoreconf
	autoreconf -i
	autoreconf
	autoreconf -i
	set -e
	./configure --prefix=/usr
	make
	make install
	cd ..
}

build_oqs () {
	echo "Build oqs..."
	cd liboqs
	mkdir build
	cd build
	cmake ..
	make -j $(($(nproc)-1))
	make install
	cd ../..
}

build_oqsprovider () {
	##git clone https://github.com/open-quantum-safe/oqs-provider.git
	export LIBOQS_SRC_DIR="$(realpath liboqs)"
	cd oqs-provider
	mkdir build
	cd build
	cmake ..
	make -j $(($(nproc)-1))
	make install
	cd ../..
}

enable_oqsprovider () {
	cp openssl.cnf /etc/ssl/openssl.cnf
}

install_kiwi () {
	echo "Install kiwi from source"
	pip install --break-system-packages jing
	pip install --break-system-packages -e ./kiwi
}

install_rust () {
	echo "Install rust"
	curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --no-modify-path
}

install_bcachefs () {
	echo "Install bcachefs"
	source /root/.cargo/env
	git clone --depth 1 --branch v1.13.0 https://github.com/koverstreet/bcachefs-tools.git
	cd bcachefs-tools
	make -j $(($(nproc)-1))
	make install
}

config_git () {
	git config --global user.email "pqc@ima"
	git config --global user.name "pqc-ima"
}

install_jupyter () {
	echo "Install jupyter"
	pip install --break-system-packages jupyterlab pandas matplotlib seaborn sh ext4
}

if [ "$#" -ne "1" ]; then
	echo "Usage $0 SETUP_FUNCTION"
	exit 1
fi

$1
