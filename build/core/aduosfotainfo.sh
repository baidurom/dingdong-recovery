#!/bin/bash

echo ""
echo "# begin adupsfota properties"
#oem info
echo "ro.adups.fota.oem=Vanzo"
#model info, Settings->About phone->Model number
echo "ro.adups.fota.device=$(grep "ro.product.model=" "$1"|awk -F "=" '{print $NF}' )"
#version number, Settings->About phone->Build number
#echo "ro.adups.fota.version=`date +%Y%m%d-%H%M`"
echo "ro.adups.fota.version=$(grep "ro.build.display.id=" "$1"|awk -F "=" '{print $NF}' )"
echo "# begin adupsfota properties"
