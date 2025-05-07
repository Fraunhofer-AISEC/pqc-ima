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

# https://stackoverflow.com/a/39124162
word-underscore = $(word $2,$(subst _, ,$1))

DESTDIR = out-kiwi/2024_12_30
DESCRIPTION = appliance

FALCON = falcon512 falcon1024
MLDSA = mldsa44 mldsa65 mldsa87
SPHINCSF = \
	sphincssha2128fsimple \
	sphincssha2192fsimple \
	sphincssha2256fsimple \
	sphincsshake128fsimple \
	sphincsshake192fsimple \
	sphincsshake256fsimple
SPHINCSS = \
	sphincssha2128ssimple \
	sphincssha2192ssimple \
	sphincssha2256ssimple \
	sphincsshake128ssimple \
	sphincsshake192ssimple \
	sphincsshake256ssimple
HSSLMS = hsslms

raws_apache: raw_ext4_apache raw_btrfs_apache raw_xfs_apache raw_f2fs_apache raw_bcachefs_apache
apache: ext4_apache btrfs_apache xfs_apache bcachefs_apache f2fs_apache
hsslms_apache: image_ext4_hsslms_apache image_btrfs_hsslms_apache image_xfs_hsslms_apache image_f2fs_hsslms_apache image_bcachefs_hsslms_apache

ext4_falcon_apache: $(foreach file,$(FALCON),image_ext4_$(file)_apache)
ext4_mldsa_apache: $(foreach file,$(MLDSA),image_ext4_$(file)_apache)
ext4_sphincsf_apache: $(foreach file,$(SPHINCSF),image_ext4_$(file)_apache)
ext4_sphincss_apache: $(foreach file,$(SPHINCSS),image_ext4_$(file)_apache)
ext4_apache: image_ext4_rsa-4096_apache image_ext4_prime256v1_apache ext4_falcon_apache ext4_mldsa_apache ext4_sphincsf_apache ext4_sphincss_apache

btrfs_falcon_apache: $(foreach file,$(FALCON),image_btrfs_$(file)_apache)
btrfs_mldsa_apache: $(foreach file,$(MLDSA),image_btrfs_$(file)_apache)
btrfs_sphincsf_apache: $(foreach file,$(SPHINCSF),image_btrfs_$(file)_apache)
btrfs_sphincss_apache: $(foreach file,$(SPHINCSS),image_btrfs_$(file)_apache)
btrfs_apache: image_btrfs_rsa-4096_apache image_btrfs_prime256v1_apache btrfs_falcon_apache btrfs_mldsa_apache btrfs_sphincsf_apache btrfs_sphincss_apache

xfs_falcon_apache: $(foreach file,$(FALCON),image_xfs_$(file)_apache)
xfs_mldsa_apache: $(foreach file,$(MLDSA),image_xfs_$(file)_apache)
xfs_sphincsf_apache: $(foreach file,$(SPHINCSF),image_xfs_$(file)_apache)
xfs_sphincss_apache: $(foreach file,$(SPHINCSS),image_xfs_$(file)_apache)
xfs_apache: image_xfs_rsa-4096_apache image_xfs_prime256v1_apache xfs_falcon_apache xfs_mldsa_apache xfs_sphincsf_apache xfs_sphincss_apache

bcachefs_falcon_apache: $(foreach file,$(FALCON),image_bcachefs_$(file)_apache)
bcachefs_mldsa_apache: $(foreach file,$(MLDSA),image_bcachefs_$(file)_apache)
bcachefs_sphincsf_apache: $(foreach file,$(SPHINCSF),image_bcachefs_$(file)_apache)
bcachefs_sphincss_apache: $(foreach file,$(SPHINCSS),image_bcachefs_$(file)_apache)
bcachefs_apache: image_bcachefs_rsa-4096_apache image_bcachefs_prime256v1_apache bcachefs_falcon_apache bcachefs_mldsa_apache bcachefs_sphincsf_apache bcachefs_sphincss_apache
#.NOTPARALLEL: bcachefs_apache

f2fs_falcon_apache: $(foreach file,$(FALCON),image_f2fs_$(file)_apache)
f2fs_mldsa_apache: $(foreach file,$(MLDSA),image_f2fs_$(file)_apache)
f2fs_sphincsf_apache: $(foreach file,$(SPHINCSF),image_f2fs_$(file)_apache)
f2fs_sphincss_apache: $(foreach file,$(SPHINCSS),image_f2fs_$(file)_apache)
f2fs_apache: image_f2fs_rsa-4096_apache image_f2fs_prime256v1_apache f2fs_falcon_apache f2fs_mldsa_apache f2fs_sphincsf_apache f2fs_sphincss_apache
#.NOTPARALLEL: f2fs_apache


ext4_falcon_gnome: $(foreach file,$(FALCON),image_ext4_$(file)_gnome)
ext4_mldsa_gnome: $(foreach file,$(MLDSA),image_ext4_$(file)_gnome)
ext4_sphincsf_gnome: $(foreach file,$(SPHINCSF),image_ext4_$(file)_gnome)
ext4_sphincss_gnome: $(foreach file,$(SPHINCSS),image_ext4_$(file)_gnome)
#ext4_hsslms_gnome: $(foreach file,$(HSSLMS),image_ext4_$(file)_gnome)
ext4_gnome: image_ext4_rsa-4096_gnome image_ext4_prime256v1_gnome ext4_falcon_gnome ext4_mldsa_gnome ext4_sphincsf_gnome ext4_sphincss_gnome

btrfs_falcon_gnome: $(foreach file,$(FALCON),image_btrfs_$(file)_gnome)
btrfs_mldsa_gnome: $(foreach file,$(MLDSA),image_btrfs_$(file)_gnome)
btrfs_sphincsf_gnome: $(foreach file,$(SPHINCSF),image_btrfs_$(file)_gnome)
btrfs_sphincss_gnome: $(foreach file,$(SPHINCSS),image_btrfs_$(file)_gnome)
btrfs_gnome: image_btrfs_rsa-4096_gnome image_btrfs_prime256v1_gnome btrfs_falcon_gnome btrfs_mldsa_gnome btrfs_sphincsf_gnome btrfs_sphincss_gnome

xfs_falcon_gnome: $(foreach file,$(FALCON),image_xfs_$(file)_gnome)
xfs_mldsa_gnome: $(foreach file,$(MLDSA),image_xfs_$(file)_gnome)
xfs_sphincsf_gnome: $(foreach file,$(SPHINCSF),image_xfs_$(file)_gnome)
xfs_sphincss_gnome: $(foreach file,$(SPHINCSS),image_xfs_$(file)_gnome)
xfs_gnome: image_xfs_rsa-4096_gnome image_xfs_prime256v1_gnome xfs_falcon_gnome xfs_mldsa_gnome xfs_sphincsf_gnome xfs_sphincss_gnome

bcachefs_falcon_gnome: $(foreach file,$(FALCON),image_bcachefs_$(file)_gnome)
bcachefs_mldsa_gnome: $(foreach file,$(MLDSA),image_bcachefs_$(file)_gnome)
bcachefs_sphincsf_gnome: $(foreach file,$(SPHINCSF),image_bcachefs_$(file)_gnome)
bcachefs_sphincss_gnome: $(foreach file,$(SPHINCSS),image_bcachefs_$(file)_gnome)
bcachefs_gnome: image_bcachefs_rsa-4096_gnome image_bcachefs_prime256v1_gnome bcachefs_falcon_gnome bcachefs_mldsa_gnome bcachefs_sphincsf_gnome bcachefs_sphincss_gnome

f2fs_falcon_gnome: $(foreach file,$(FALCON),image_f2fs_$(file)_gnome)
f2fs_mldsa_gnome: $(foreach file,$(MLDSA),image_f2fs_$(file)_gnome)
f2fs_sphincsf_gnome: $(foreach file,$(SPHINCSF),image_f2fs_$(file)_gnome)
f2fs_sphincss_gnome: $(foreach file,$(SPHINCSS),image_f2fs_$(file)_gnome)
f2fs_gnome: image_f2fs_rsa-4096_gnome image_f2fs_prime256v1_gnome f2fs_falcon_gnome f2fs_mldsa_gnome f2fs_sphincsf_gnome f2fs_sphincss_gnome

raws_gnome: raw_ext4_gnome raw_btrfs_gnome raw_xfs_gnome raw_f2fs_gnome raw_bcachefs_gnome
gnome: ext4_gnome btrfs_gnome xfs_gnome bcachefs_gnome f2fs_gnome
hsslms_gnome: image_ext4_hsslms_gnome image_btrfs_hsslms_gnome image_xfs_hsslms_gnome image_f2fs_hsslms_gnome image_bcachefs_hsslms_gnome

all: apache gnome

roots: rootfs_apache rootfs_gnome
raws: raws_apache raws_gnome

DOCKER_RUN = docker run --privileged --rm -v /dev:/dev -v $(abspath $(dir $(MAKEFILE_LIST))..):/mnt -w /mnt/kiwi-pqc-ima pqc-ima

# rootfs_apache or rootfs_gnome
rootfs_%:
	$(DOCKER_RUN) \
	kiwi-ng --profile ext4-$* \
		system prepare \
		--description $(DESCRIPTION) \
		--root $(DESTDIR)/$*/rootfs


# e.g. image_raw_ext4_apache
.SECONDEXPANSION:
raw_%: #rootfs_$$(call word-underscore,$$*,2)
	@echo "Building raw image $*"
	$(DOCKER_RUN) \
	kiwi-ng \
		--profile $(call word-underscore,$*,1)-$(call word-underscore,$*,2) \
		system create \
		--root $(DESTDIR)/$(call word-underscore,$*,2)/rootfs \
		--target-dir $(DESTDIR)/$(call word-underscore,$*,2)/$*
	$(DOCKER_RUN) \
	bash fix_bcachefs.sh $(DESTDIR) $(call word-underscore,$*,1) $(call word-underscore,$*,2)
	$(DOCKER_RUN) \
	pigz --force --rsyncable --keep $(DESTDIR)/$(call word-underscore,$*,2)/$*/kiwi-pqc-ima.x86_64-12.1.raw

# currently e.g. image_btrfs_mldsa44_apache
image_%: #raw_$$(call word-underscore,$$*,1)_$$(call word-underscore,$$*,3)
	@echo "Signing image $*"
	$(DOCKER_RUN) \
	bash sign_image.sh $(DESTDIR) $(call word-underscore,$*,1) $(call word-underscore,$*,2) $(call word-underscore,$*,3)
