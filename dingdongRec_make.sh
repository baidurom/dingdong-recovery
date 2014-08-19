#!/bin/bash
#set -x
TOOL_ROOT=`pwd`
DD_RECOVERY_VERSION=1.0.0

DEVICE_LIST=patch_device/devices.list
RELEASE_DIR=$TOOL_ROOT/out/release
UPLOAD_DIR=$TOOL_ROOT/out/upload
MT6577_REC_BIN=$TOOL_ROOT/patch_device/mt6577_rec/recovery
IS_MT6577=FALSE
MT6589_HAS_COMPILED=FALSE
MT6592_HAS_COMPILED=FALSE

function usage()
{
    echo ""
    echo "## NOTICE: MT6577 platform should put recovery bin in patch_device/mt6577_rec ##"
	echo "Usage:"
    echo "1) make one device"
    echo "     $0 device_name -p [platform] -t screentype(hdpi,xhdpi,xxhdpi)"
	echo "   Example:"
    echo "     $0 g520 -p mt6589 -t xhdpi"
    echo ""
    echo "2) make all device"
    echo "     $0 all"
    echo ""
    echo "3) clean the project"
    echo "     $0 clean"
    echo ""
	exit 1;
}

function mt6577_replace_recovery()
{
    echo ""
	echo "## Please put mt6577 recovery bin in patch_device/mt6577_rec"
    echo ""
    echo -n "## When finished, enter <yes|no>:"
    read answer
    case $answer in
        YES|yes|Y|y)
            echo "Ready, continue...";;
        *)
            exit 1;;
    esac
}

function judge_platform(){
    if [[ "$#" == "2" ]] && [[ "$1" == "-p" ]];then
        if [[ "$2" == "mt6589" ]];then
    	    COMPILE_DEVICE=a820
            IS_MT6577=FALSE
        elif [[ "$2" == "mt6592" ]];then
            COMPILE_DEVICE=Coolpad8297
            IS_MT6577=FALSE
        elif [[ "$2" == "mt6577" ]];then
            COMPILE_DEVICE=a820
            IS_MT6577=TRUE
        else
            echo ""
            echo "Invalid platform! now support mt6577, mt6589, mt6592."
            usage;
        fi
    else
        echo ""
        echo "Invalid input!"
    	usage;
    fi
}

function make_dingdong_recovery(){
    echo "make_dingdong_recovery() item1: $1, item2: $2, item3: $3"
    DEVICE=$1
    SCREEN_TYPE=$5
    PATCH_DIR=$TOOL_ROOT/patch_device/$DEVICE
    OUT_DIR=$TOOL_ROOT/out/patch_device/$DEVICE
    PATCH_REC_ROOT=$TOOL_ROOT/patch_device/$DEVICE/root
    OUT_SBIN=$OUT_DIR/root/RAMDISK/sbin
    OUT_RES=$OUT_DIR/root/RAMDISK/res
    CP_SBIN=$OUT_DIR/tmp/RAMDISK/sbin
    CP_RES=$OUT_DIR/tmp/RAMDISK/res
    DREC=$OUT_DIR/recovery.img
    RES_ICON_HDPI=src/res/icons-hdpi
    RES_ICON_XHDPI=src/res/icons-xhdpi
    RES_ICON_XXHDPI=src/res/icons-xxhdpi

    if [[ ! -d "$PATCH_DIR" ]];then
        echo ""
        echo "$DEVICE not supported now, please check!"
        exit 1;
    fi

    judge_platform $2 $3;

    MREC=out/target/product/$COMPILE_DEVICE/recovery.img
	echo ""
	echo ""
	echo ""
	echo "####################################################"
    echo "#   Making $DEVICE"
	echo "####################################################"
	echo ""
	echo ""

    ## -- Begin build tmp recovery.img -- ##
    echo "****************************************************"
    echo "** STEP 1 -- Building tmp recovery.img            **"
    echo "****************************************************"
    if [[ "$COMPILE_DEVICE" == "a820" ]] && [[ "$MT6589_HAS_COMPILED" == "TRUE" ]];then
        echo "A820 has builded, just go to next step!"
    elif [[ "$COMPILE_DEVICE" == "Coolpad8297" ]] && [[ "$MT6592_HAS_COMPILED" == "TRUE" ]];then
        echo "Coolpad8297 has builded, just go to next step!"
    else
        . build/envsetup.sh
        ./makeMtk -t $COMPILE_DEVICE recoveryimage

        if [[ "$?" != "0" ]]; then
            echo ""
            echo "ABORT, PLEASE CHECK!"
            echo ""
            exit 1;
        fi

        if [[ "$COMPILE_DEVICE" == "a820" ]];then
            MT6589_HAS_COMPILED=TRUE
        elif [[ "$COMPILE_DEVICE" == "Coolpad8297" ]];then
            MT6592_HAS_COMPILED=TRUE
        fi
    fi

    ## -- Clean the out files --##
    echo "****************************************************"
    echo "** STEP 2 -- Clean output files                   **"
    echo "****************************************************"
    rm -rf $OUT_DIR
    mkdir -p $OUT_DIR
    mkdir -p $RELEASE_DIR
    mkdir -p $UPLOAD_DIR

    ## -- Unpack the recovery.img --##
    echo "****************************************************"
    echo "** STEP 3 -- Unpack the recovery.img              **"
    echo "****************************************************"
    unpack_bootimg $MREC $OUT_DIR/tmp
    echo "cp -Rf $PATCH_REC_ROOT $OUT_DIR/"
    cp -Rf $PATCH_REC_ROOT $OUT_DIR/

    ## -- Replace files --##
    echo "****************************************************"
    echo "** STEP 4 -- Replace some files                   **"
    echo "****************************************************"
    cp $OUT_DIR/tmp/type.config $OUT_DIR/root/type.config
    rm -rf $OUT_RES
    rm $CP_SBIN/adbd
    cp -f $OUT_SBIN/adbd $CP_SBIN/adbd

    ##-- Check if mt6577
    if [[ "$IS_MT6577" == "TRUE" ]] && [[ -f "$MT6577_REC_BIN" ]];then
        echo "MT6577: replace recovery binary"
        echo "cp $MT6577_REC_BIN $CP_SBIN/recovery"
        cp $MT6577_REC_BIN $CP_SBIN/recovery
    fi

    rm -rf $OUT_SBIN
    cp -Rf $CP_SBIN $OUT_SBIN
    cp -Rf $CP_RES $OUT_RES
    rm -rf $OUT_RES/icons
    cp $PATCH_DIR/*.conf $OUT_RES/
    if [[ "$SCREEN_TYPE" == "hdpi" ]];then
        cp -Rf $RES_ICON_HDPI $OUT_RES/icons
    elif [[ "$SCREEN_TYPE" == "xhdpi" ]];then
        cp -Rf $RES_ICON_XHDPI $OUT_RES/icons
    elif [[ "$SCREEN_TYPE" == "xxhdpi" ]];then
        cp -Rf $RES_ICON_XXHDPI $OUT_RES/icons
    else
        echo "Can not find screen type: $5"
        exit 0;
    fi


    ## -- Repack the recovery.img --##
    echo "****************************************************"
    echo "** STEP 5 -- Repack the new recovery.img          **"
    echo "****************************************************"
    pack_bootimg $OUT_DIR/root $DREC

    cp $DREC $RELEASE_DIR/dingdongrecovery-$DD_RECOVERY_VERSION"-"$DEVICE.img

    ## -- $DREC make finished! --##
    echo "****************************************************"
    echo "** STEP 6 -- Clean tmp files                      **"
    echo "****************************************************"
    rm -rf $OUT_DIR/tmp
}

###### -- MAIN ENTRY -- ######

if [[ "$#" != "5" ]] && [[ "$1" != "all" ]] && [[ "$1" != "clean" ]];then
    usage;
fi

if [[ "$1" == "all" ]];then
    cat $DEVICE_LIST | while read item_device item_platform
    do
        make_dingdong_recovery $item_device -p $item_platform;
    done
elif [[ "$1" == "clean" ]];then
	. build/envsetup.sh
	./makeMtk clean
	rm -rf out
	echo ""
	echo "Clean done"
	echo ""
else
    make_dingdong_recovery $1 $2 $3 $4 $5;
fi

echo "****************************************************"
echo "**  ALL FINISHED                                  **"
echo "****************************************************"

exit 0;
