#!/bin/bash
device_name=$1

function usage()
{
	echo " usage: $0 device_name"
	echo "        $0 maguro"
	exit 0
}

if [[ "$#" != "1" ]];then
    usage
fi

if [[ "$device_name" == "crespo" ]]; then
    adb reboot bootloader;
    sudo fastboot flash recovery out/patch_device/$device_name/recovery.img;
    sudo fastboot boot out/patch_device/$device_name/recovery.img;
    exit 0;
fi

adb root
sleep 2
adb shell mount /data

if [[ "$device_name" == "a820" ]] || [[ "$device_name" == "s720" ]] || [[ "$device_name" == "3c" ]]; then
    recovery_p=/dev/recovery
    echo "adb shell dd if=/data/local/tmp/recovery.img of=$recovery_p"
    adb push out/patch_device/$device_name/recovery.img /data/local/tmp/recovery.img
    adb shell dd if=/data/local/tmp/recovery.img of=$recovery_p
    adb shell su -c "dd if=/data/local/tmp/recovery.img of=$recovery_p"
    adb reboot recovery
    exit 0;
fi
exit 0;
