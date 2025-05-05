# PQC-IMA: Post-Quantum Cryptography for Linux File System Integrity
This project is not maintained. It has been published as part of the following ACNS 2025 conference paper:

> Johannes Wiesböck, Maximiliane Münch, Michael Weiß. 2025.
> Post-Quantum Cryptography for Linux File System Integrity. In
> 23rd International Conference, ACNS 2025, Munich, Germany, June 23-26, 2025, Proceedings, Part III. Lecture Notes in Computer Science 15827, Springer 2025, [10.1007/978-3-031-95767-3_3](https://doi.org/10.1007/978-3-031-95767-3_3)

Note that these repositories present a **prototype** implementation and are **not** to be used in production.

## Introduction
This repository contains the necessary patches to build the Linux kernel equipped with quantum-resistant cryptography.
It further contains all patches, scripts and dependencies that are required to reproduce the evaluation setup described in the paper.
Due to the large number of dependencies, we provide a docker-based workflow that automates most steps.
Please note that building **all** images from the paper evaluation requires around 4.5 TB disk space.
Building on a copy-on-write file system such as btrfs is recommended as this drastically reduces the real disk usage due to the use of reflink copies.

The following steps transparently run **privileged** docker containers.

## Create the Docker Image
To build the docker image that contains the build system and its dependencies, run:

    $ ./setup.sh --build-docker

## Build the Linux Kernel
To clone, patch and compile the Linux kernel, run:

    $ ./setup.sh --build-linux

The resulting Debian package is automatically copied to the `kiwi-pqc-ima` build tree.

## Build the Images
Building the evaluation images is divided into three steps, (1) creating the rootfs, (2) building the unsigned image, and (3) signing the image with the respective cipher.
For example, to build the image for the ext4, ML-DSA44, Apache Web Server test case, run:

    $ cd kiwi-pqc-ima
    $ make rootfs_apache                # build the apache rootfs
    $ make raw_ext4_apache              # build the unsigned (raw) image
    $ make image_ext4_mldsa44_apache    # sign the image with ML-DSA44

To build all Web server images at once, replace the last command with:

    $ make apache

The last step can be sped up by signing images in parallel by using the `-j` flag.

The desktop images are built analogously by replacing `apache` with `gnome` in the above commands.

## Run the Benchmarks
To run the runtime benchmarks on all successfully built images, run:

    $ cd kiwi-pqc-ima
    $ sudo ./runqemu.sh -d out-kiwi/2024_12_30/apache  # to run the web server benchmarks
    $ sudo ./runqemu.sh -d out-kiwi/2024_12_30/gnome   # to run the desktop benchmarks

This will run the benchmarks in virtual machines and then extract the measurement results.
The results are stored in the `measurements` subdirectory.

## Reproduce the paper figures
To reproduce the figures shown in the paper, launch a jupyter server and follow the provided notebooks:

    $ ./setup.sh --run-jupyter

The respective notebooks can be found in the `jupyter` subdirectory:
- jupyter/ima_usage.ipynb: Disk space usage
- jupyter/boottime.ipynb: Boot time
- jupyter/app_start.ipynb: Chrome browser start time

The notebooks contain instruction how to adjust the file paths to switch between provided data and reproduced results.
