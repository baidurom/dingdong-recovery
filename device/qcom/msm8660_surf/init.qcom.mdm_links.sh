#!/system/bin/sh
# Copyright (c) 2011, Code Aurora Forum. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of Code Aurora Forum, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

# No path is set up at this point so we have to do it here.
PATH=/sbin:/system/sbin:/system/bin:/system/xbin
export PATH

mkdir /system/etc/firmware/misc_mdm
chmod 771  /system/etc/firmware/misc_mdm
chown system.system /system/etc/firmware/misc_mdm
mount -t vfat -o ro,shortname=lower /dev/block/mmcblk0p19 /system/etc/firmware/misc_mdm

MISC_MDM=/system/etc/firmware/misc_mdm/image
cd $MISC_MDM
ln -s $MISC_MDM/amss.mbn /system/etc/firmware/amss.mbn 2>/dev/null
ln -s $MISC_MDM/dsp1.mbn /system/etc/firmware/dsp1.mbn 2>/dev/null
ln -s $MISC_MDM/dsp2.mbn /system/etc/firmware/dsp2.mbn 2>/dev/null
ln -s $MISC_MDM/dbl.mbn  /system/etc/firmware/dbl.mbn  2>/dev/null
ln -s $MISC_MDM/osbl.mbn /system/etc/firmware/osbl.mbn 2>/dev/null
ln -s $MISC_MDM/efs1.mbn /system/etc/firmware/efs1.mbn 2>/dev/null
ln -s $MISC_MDM/efs2.mbn /system/etc/firmware/efs2.mbn 2>/dev/null
ln -s $MISC_MDM/efs3.mbn /system/etc/firmware/efs3.mbn 2>/dev/null

cd /
