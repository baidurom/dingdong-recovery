#!/system/bin/sh
# Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

# set default composition for MSM7627A
case $soc_id in
     90 | 91 | 92 | 97 | 101 | 102 | 103 | 136)
        comp_7x27A=`getprop debug.composition.7x27A.type`
        setprop debug.composition.type $comp_7x27A
        setprop ro.hw_plat 7x27A
        buildid=`cat /sys/devices/system/soc/soc0/build_id`
        offset_1=0
        offset_2=6
        length=1
        is_unicorn=7
        dsp_lpa_enabled=2
        modemid_1=${buildid:$offset_1:$length}
        modemid_2=${buildid:$offset_2:$length}
        if [ "$modemid_1" = "$is_unicorn" ] && [ "$modemid_2" -gt "$dsp_lpa_enabled" ]; then
           setprop lpa.decode true
           setprop audio.decoder_override_check true
           setprop use.non-omx.mp3.decoder true
        else
           setprop lpa.decode false
        fi
    ;;
esac

# set default composition for MSM7625A
case $soc_id in
     88 | 89 | 96 | 98 | 99 | 100 | 131 | 132 | 133 | 135)
        comp_7x25A=`getprop debug.composition.7x25A.type`
        setprop debug.composition.type $comp_7x25A
        setprop ro.hw_plat 7x25A
        buildid=`cat /sys/devices/system/soc/soc0/build_id`
        offset_1=0
        offset_2=6
        length=1
        is_unicorn=7
        dsp_lpa_enabled=2
        modemid_1=${buildid:$offset_1:$length}
        modemid_2=${buildid:$offset_2:$length}
        if [ "$modemid_1" = "$is_unicorn" ] && [ "$modemid_2" -gt "$dsp_lpa_enabled" ]; then
           setprop lpa.decode true
           setprop audio.decoder_override_check true
           setprop use.non-omx.mp3.decoder true
        else
           setprop lpa.decode false
        fi
    ;;
esac

# set default composition for MSM8625
case $soc_id in
     127 | 128 | 129 | 137)
        comp_8x25=`getprop debug.composition.8x25.type`
        setprop debug.composition.type $comp_8x25
        setprop ro.hw_plat 8x25
        setprop lpa.decode true
        setprop audio.decoder_override_check true
        setprop use.non-omx.mp3.decoder true
        setprop ro.qc.sdk.audio.fluencetype fluence
    ;;
esac
