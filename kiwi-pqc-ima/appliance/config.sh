#!/bin/bash
#================
# FILE          : config.sh
#----------------
# PROJECT       : OpenSuSE KIWI Image System
# COPYRIGHT     : (c) 2006 SUSE LINUX Products GmbH. All rights reserved
#               :
# AUTHOR        : Marcus Schaefer <ms@suse.de>
#               :
# BELONGS TO    : Operating System images
#               :
# DESCRIPTION   : configuration script for Debian based
#               : operating systems
#               :
#               :
# STATUS        : BETA
#----------------
#======================================
# Functions...
#--------------------------------------
test -f /.kconfig && . /.kconfig
test -f /.profile && . /.profile

#======================================
# Greeting...
#--------------------------------------
echo "Configure image: [$kiwi_iname]..."

cd /var
rm -r run
ln -s ../run run
cd /

#======================================
# Activate services
#--------------------------------------
baseInsertService sshd

#======================================
# Setup default target, multi-user
#--------------------------------------
baseSetRunlevel 3

# Install packages
## Create directory
mkdir -p /tmp/packages
## Extract archive
tar -xzf /tmp/packages.tar.gz -C /tmp/packages
## Install all .deb files
for file in /tmp/packages/*.deb; do
    if [ -f "$file" ]; then
        dpkg -i "$file"
    fi
done

# Check if apache is installed
if systemctl cat -- apache2.service &> /dev/null; then
    systemctl enable apache2
fi

# Check if gnome display manager is installed
if systemctl cat -- gdm.service &> /dev/null; then
    # Enable gdm
    systemctl enable gdm3
    # Enable graphical boot
    ln -sf /lib/systemd/system/graphical.target /etc/systemd/system/default.target
fi

systemctl disable networking.service

if [[ -f "/usr/local/bin/test-hyperfine.sh" ]]
then
	systemctl --global enable test-hyperfine.timer
else
	systemctl enable measure-boottime.service
fi

rm /initrd*
rm /boot/initrd*
