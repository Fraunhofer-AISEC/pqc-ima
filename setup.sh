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

build_docker () {
	docker build -t pqc-ima .
}

build_linux_internal () {
	cd kernel_build
	echo "Prepare kernel config..."
	if [[ ! -f "linux/.config" ]]; then
		export ROOT_CERT_PATH="$(realpath ../kiwi-pqc-ima/appliance/pki/rootca/rootca_crt.pem)"
		cd linux
		cp debian_config_pqc .config
		sed -i "s|#IMA_ROOT_CERT_PATH#|"${ROOT_CERT_PATH}"|g" .config
		cd ..
	else
		echo "Already done."
	fi

	echo "Compile linux kernel..."
	if ls linux-image-6.11.7-pqc_*deb 1> /dev/null 2>&1; then
		echo "Already done."
	else
		cd linux
		make oldconfig
		nice -n19 make -j $(($(nproc)-1)) bindeb-pkg
		cd ..
	fi

	echo "Deploy kernel package..."
	mkdir -p ../kiwi-pqc-ima/appliance/root/tmp/packages
	cp linux-image-6.11.7-pqc_*deb ../kiwi-pqc-ima/appliance/root/tmp/packages
}

build_linux () {
	docker run --privileged -it --rm -v.:/mnt pqc-ima:latest ./setup.sh build_linux_internal
}

run_jupyter () {
	docker run -it --rm \
		-v $PWD/kiwi-pqc-ima:/mnt \
		-p 127.0.0.1:8888:8888 \
		pqc-ima:latest \
		jupyter lab --allow-root --ip 0.0.0.0
}

die () {
        echo "Usage: $0 [--build-docker] [--build-linux] [--run-jupyter]"
        exit 1
}

FUNC=
while [[ "$#" -gt 0 ]]; do
        case $1 in
                --build-docker)
                        FUNC=build_docker
                        ;;
                --build-linux)
                        FUNC=build_linux
                        ;;
                --run-jupyter)
                        FUNC=run_jupyter
                        ;;
                *)
                        echo "Unknown parameter passed: $1"; exit 1 ;;
        esac
        shift
done

[[ -z "$FUNC" ]] && die

echo $FUNC

$FUNC
