#!/system/bin/sh
# Copyright (c) 2012 Code Aurora Forum. All rights reserved.
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

#
# set rild-libpath using modem build_id
#
PATH=/sbin:/system/sbin:/system/bin:/system/xbin
export PATH
buildid=`cat /sys/devices/system/soc/soc0/build_id`
offset_1=0
offset_2=5
offset_3=4
length=1
is_strider=8
is_qmi_enabled=1
is_unicorn=7
is_unicorn_strider='S'
dsds=`getprop persist.multisim.config`
modemid_1=${buildid:$offset_1:$length}
modemid_2=${buildid:$offset_2:$length}
modemid_3=${buildid:$offset_3:$length}
if ([ "$modemid_1" = "$is_strider" ] && [ "$modemid_2" -gt "$is_qmi_enabled" ]) ||
       ([ "$modemid_1" = "$is_unicorn" ] && [ "$modemid_3" = "$is_unicorn_strider" ]); then
    setprop rild.libpath "/system/lib/libril-qc-qmi-1.so"
    if [ "$dsds" = "dsds" ]; then
        setprop ro.multi.rild true
        stop ril-daemon
        start ril-daemon
        start ril-daemon1
    else
        stop ril-daemon
        start ril-daemon
    fi
fi
