#! /system/bin/sh

PROP="persist.multisim.config"
modem_type=`cat /sys/devices/system/soc/soc0/modem_type`
case "$modem_type" in
	"MULTI_NO_DSDS")
	setprop $PROP ""
	;;

	"MULTI")
	setprop $PROP dsds
	;;

	"UMTS")
	setprop $PROP dsds
	;;

esac
