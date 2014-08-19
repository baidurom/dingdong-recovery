#!/system/bin/sh
# Copyright (c) 2011, Code Aurora Forum. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Code Aurora nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

soc_id=`cat /sys/devices/system/soc/soc0/id`
boot_serialno=`getprop ro.boot.serialno`

# set default dpi for MSM7227AA SKU3
case "$boot_serialno" in
      "7227AAQRD3")
         dpi_msm7227aa_sku3="160"
         setprop ro.sf.lcd_density $dpi_msm7227aa_sku3
         setprop ro.hw_plat 7x27A
    ;;
esac

# set default dpi for MSM7627A
case $soc_id in
     90 | 91 | 92 | 97 | 101 | 102 | 103)
        dpi_7x27A=`getprop ro.sf.7x27A.lcd_density`
        setprop ro.sf.lcd_density $dpi_7x27A
        setprop ro.hw_plat 7x27A
    ;;
esac

# set default dpi for 8625 SKU7
case "$boot_serialno" in
      "MSM8225QRD7")
         dpi_msm7627a_sku7="160"
         setprop ro.sf.lcd_density $dpi_msm7627a_sku7
         setprop ro.hw_plat 8x25
    ;;
esac

# set default dpi for MSM8625
case $soc_id in
     127 | 128 | 129)
        dpi_8x25=`getprop ro.sf.8x25.lcd_density`
        setprop ro.sf.lcd_density $dpi_8x25
        setprop ro.hw_plat 8x25
    ;;
esac


