# List of apps and optional libraries (Java and native) to put in the add-on system image.
#PRODUCT_PACKAGES := \
#	PlatformLibraryClient \
#	com.example.android.platform_library \
#	libplatform_library_jni

PRODUCT_PACKAGES := \
        SDKGallery \
        mediatek-res \
        thememap.xml \
        libI420colorconvert \
        libvcodec_utility \
        libvcodec_oal \
        libmp4enc_xa.ca7 \
        libvideoeditorplayer \
        libvideoeditor_osal \
        libvideoeditor_3gpwriter \
        libvideoeditor_mcs \
        libvideoeditor_core \
        libvideoeditor_stagefrightshells \
        libvideoeditor_videofilters \
        libvideoeditor_jni \
        audio.primary.default \
        audio_policy.stub \
        local_time.default \
        libaudiocustparam \
        libh264dec_xa.ca9 \
        libh264dec_xb.ca9 \
        libh264dec_customize \
        libmp4dec_sa.ca9 \
        libmp4dec_sb.ca9 \
        libmp4dec_customize \
        libvp8dec_xa.ca9 \
        libmp4enc_xa.ca9 \
        libmp4enc_xb.ca9 \
        libh264enc_sa.ca9 \
        libh264enc_sb.ca9 \
        libvcodec_oal \
        libvc1dec_sa.ca9 \
        init.factory.rc \
        audio_policy.default \
        libaudio.a2dp.default \
        libMtkVideoTranscoder \
        libMtkOmxCore \
        libMtkOmxOsalUtils \
        libMtkOmxVdec \
        libMtkOmxVenc \
        libaudiodcrflt \
        libaudiosetting \
        librtp_jni \
        mfv_ut \
        libstagefrighthw \
        libstagefright_memutil \
        factory.ini \
        libmtdutil \
        libminiui \
        factory \
        libaudio.usb.default \
        ccci_mdinit \
        ccci_fsd \
        permission_check \
        CellConnService \
        MTKAndroidSuiteDaemon \
        libfmjni \
        libfmmt6616 \
        libfmmt6626 \
        libfmmt6620 \
        libfmmt6628 \
        libfmar1000 \
        libfmcust \
        fm_cust.cfg \
        mt6628_fm_rom.bin \
        mt6628_fm_v1_patch.bin \
        mt6628_fm_v1_coeff.bin \
        mt6628_fm_v2_patch.bin \
        mt6628_fm_v2_coeff.bin \
        mt6628_fm_v3_patch.bin \
        mt6628_fm_v3_coeff.bin \
        mt6628_fm_v4_patch.bin \
        mt6628_fm_v4_coeff.bin \
        mt6628_fm_v5_patch.bin \
        mt6628_fm_v5_coeff.bin \
        ami304d \
        akmd8963 \
        akmd8975 \
        geomagneticd \
        orientationd \
        memsicd \
        msensord \
        bmm050d \
        magd \
        sensors.mt6577 \
        sensors.mt6589 \
        sensors.default\
        libhwm \
        lights.default \
        libft \
        meta_tst \
        GoogleOtaBinder \
        dm_agent_binder \
        libvdmengine.so \
        libvdmfumo.so \
        libvdmlawmo.so \
        libvdmscinv.so \
        libvdmscomo.so \
        dhcp6c \
        dhcp6ctl \
        dhcp6c.conf \
        dhcp6cDNS.conf \
        dhcp6s \
        dhcp6s.conf \
        dhcp6c.script \
        dhcp6cctlkey \
        libblisrc \
        libifaddrs \
        libbluetoothdrv \
        libbluetooth_mtk \
        libbluetoothem_mtk \
        libbluetooth_relayer \
        libmeta_bluetooth \
        mt6620_patch_hdr.bin \
        mt6620_patch_e3_hdr.bin \
        mt6620_patch_e3_0_hdr.bin \
        mt6620_patch_e3_1_hdr.bin \
        mt6620_patch_e3_2_hdr.bin \
        mt6620_patch_e3_3_hdr.bin \
        mt6620_patch_e6_hdr.bin \
        mt6628_patch_e1_hdr.bin \
        mt6628_patch_e2_hdr.bin \
        WMT.cfg \
        6620_launcher \
        6620_wmt_concurrency \
        stp_dump3 \
        mobile_log_d \
        libmobilelog_jni \
        libaudio.r_submix.default \
        libaudio.usb.default \
        libnbaio \
        libaudioflinger \
        libmeta_audio \
        sysctl \
        sysctld \
        liba3m \
        libja3m \
        libmmprofile \
        libmmprofile_jni \
        mediatek-common \
        mediatek-framework \
        mediatek-op \
        libtvoutjni \
        libtvoutpattern \
        libmtkhdmi_jni \
        aee \
        aee_aed \
        aee_core_forwarder \
        aee_dumpstate \
        rtt \
        libaed.so \
        libmediatek_exceptionlog\
        camera.default \
        xlog \
        liblog \
        shutdown \
        WIFI_RAM_CODE \
        WIFI_RAM_CODE_E6 \
        WIFI_RAM_CODE_MT6628 \
        muxreport \
        rild \
        mtk-ril \
        librilmtk \
        libutilrilmtk \
        gsm0710muxd \
        rildmd2 \
        mtk-rilmd2 \
        librilmtkmd2 \
        gsm0710muxdmd2 \
        md_minilog_util \
        wbxml \
        wappush \
        thememap.xml \
        libBLPP.so \
        rc.fac \
        mtkGD \
        pvrsrvctl \
        libEGL_mtk.so \
        libGLESv1_CM_mtk.so \
        libGLESv2_mtk.so \
        gralloc.mt6577.so \
        gralloc.mt6589.so \
        libusc.so \
        libglslcompiler.so \
        libIMGegl.so \
        libpvr2d.so \
        libsrv_um.so \
        libsrv_init.so \
        libPVRScopeServices.so \
        libpvrANDROID_WSEGL.so \
        libFraunhoferAAC \
        libMtkOmxAudioEncBase \
        libMtkOmxAmrEnc \
        libMtkOmxAwbEnc \
        libMtkOmxAacEnc \
        libMtkOmxVorbisEnc \
        libMtkOmxAdpcmEnc \
        libMtkOmxMp3Dec \
        libMtkOmxAacDec \
        libMtkOmxG711Dec \
        libMtkOmxVorbisDec \
        libMtkOmxAudioDecBase \
        libMtkOmxWmaDec \
        libMtkOmxAMRNBDec \
        libMtkOmxAMRWBDec \
        libasf \
        libasfextractor \
        audio.primary.default \
        audio_policy.stub \
        audio_policy.default \
        libaudio.a2dp.default \
        libaudio-resampler \
        local_time.default \
        libaudiocustparam \
        libaudiodcrflt \
        libaudiosetting \
        librtp_jni \
        libmatv_cust \
        libmtkplayer \
        libatvctrlservice \
        matv \
        libMtkOmxApeDec \
        libMtkOmxFlacDec \
        ppp_dt \
        power.default \
        libdiagnose \
        netdiag \
        mnld \
        libmnlp \
        gps.default\
        libmnl.a \
        libsupl.a \
        libhotstill.a \
        libagent.a \
        libsonivox \
        iAmCdRom.iso \
        libmemorydumper \
        memorydumper \
        libvt_custom \
        libamrvt \
        libvtmal \
        racoon \
        libipsec \
        libpcap \
        mtpd \
        netcfg \
        pppd \
        pppd_dt \
        dhcpcd \
        dhcpcd.conf \
        dhcpcd-run-hooks \
        20-dns.conf \
        95-configured \
        radvd \
        radvd.conf \
        dnsmasq \
        netd \
        ndc \
        libiprouteutil \
        libnetlink \
        tc \
        libext2_profile \
        e2fsck \
        libext2_blkid \
        libext2_e2p \
        libext2_com_err \
        libext2fs \
        libext2_uuid \
        mke2fs \
        tune2fs \
        badblocks \
        resize2fs \
        libnvram \
        libnvram_daemon_callback \
        libfile_op \
        nvram_agent_binder \
        nvram_daemon \
        make_ext4fs \
        sdcard \
        libext \
        libext4 \
        libext6 \
        libxtables \
        libip4tc \
        libip6tc \
        ipod \
        libipod \
        bootanimation\
        libtvoutjni \
        libtvoutpattern \
        libmtkhdmi_jni \
        sdiotool \
        superumount \
        libsched \
        fsck_msdos_mtk \
        cmmbsp \
        libcmmb_jni \
        libc_malloc_debug_mtk \
        dpfd \
        libaal \
        aal \
        CustomProperties

# Manually copy the optional library XML files in the system image.
#PRODUCT_COPY_FILES := \
#    device/sample/frameworks/PlatformLibrary/com.example.android.platform_library.xml:system/etc/permissions/com.example.android.platform_library.xml

# name of the add-on
PRODUCT_SDK_ADDON_NAME := banyan_addon

# Copy the manifest and hardware files for the SDK add-on.
# The content of those files is manually created for now.
PRODUCT_SDK_ADDON_COPY_FILES :=

ifneq ($(strip $(BUILD_MTK_SDK)),toolset)
PRODUCT_SDK_ADDON_COPY_FILES += \
    mediatek/device/banyan_addon/manifest.ini:manifest.ini \
    mediatek/device/banyan_addon/hardware.ini:hardware.ini \
    $(call find-copy-subdir-files,*,mediatek/device/banyan_addon/skins,skins)
endif

ifneq ($(strip $(BUILD_MTK_SDK)),api)
PRODUCT_SDK_ADDON_COPY_FILES += \
    $(call find-copy-subdir-files,*,mediatek/frameworks/banyan/tools,tools)
endif

# Copy the jar files for the optional libraries that are exposed as APIs.
PRODUCT_SDK_ADDON_COPY_MODULES :=

# Name of the doc to generate and put in the add-on. This must match the name defined
# in the optional library with the tag
#    LOCAL_MODULE:= mediatek-sdk
# in the documentation section.
PRODUCT_SDK_ADDON_DOC_MODULES := mediatek-sdk

PRODUCT_SDK_ADDON_COPY_HOST_OUT :=

# mediatek-android.jar stub library is generated separately (defined in
# mediatek/frameworks/banyan_addon/Android.mk) and copied to MTK
# SDK pacakge by using "PRODUCT_SDK_ADDON_COPY_HOST_OUT".
ifneq ($(strip $(BUILD_MTK_SDK)),toolset)
PRODUCT_SDK_ADDON_COPY_HOST_OUT += \
    framework/mediatek-android.jar:libs/mediatek-android.jar \
    framework/mediatek-compatibility.jar:libs/mediatek-compatibility.jar \
    ../../../mediatek/frameworks/banyan/README.txt:libs/README.txt \
    bin/emulator:emulator/linux/emulator \
    bin/emulator-arm:emulator/linux/emulator-arm \
    bin/emulator-x86:emulator/linux/emulator-x86 \
    bin/emulator.exe:emulator/windows/emulator.exe \
    bin/emulator-arm.exe:emulator/windows/emulator-arm.exe \
    bin/emulator-x86.exe:emulator/windows/emulator-x86.exe
endif

#
# MediaTek resource overlay configuration
#
$(foreach cf,$(RESOURCE_OVERLAY_SUPPORT), \
  $(eval # do NOT modify the overlay resource paths order) \
  $(eval # 1. project level resource overlay) \
  $(eval _project_overlay_dir := $(MTK_ROOT_CUSTOM)/$(TARGET_PRODUCT)/resource_overlay/$(cf)) \
  $(if $(wildcard $(_project_overlay_dir)), \
    $(eval PRODUCT_PACKAGE_OVERLAYS += $(_project_overlay_dir)) \
    , \
   ) \
  $(eval # 2. operator spec. resource overlay) \
  $(eval _operator_overlay_dir := $(MTK_ROOT_CUSTOM)/$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF)))/resource_overlay/$(cf)) \
  $(if $(wildcard $(_operator_overlay_dir)), \
    $(eval PRODUCT_PACKAGE_OVERLAYS += $(_operator_overlay_dir)) \
    , \
   ) \
  $(eval # 3. product line level resource overlay) \
  $(eval _product_line_overlay_dir := $(MTK_ROOT_CUSTOM)/$(PRODUCT)/resource_overlay/$(cf)) \
  $(if $(wildcard $(_product_line_overlay_dir)), \
    $(eval PRODUCT_PACKAGE_OVERLAYS += $(_product_line_overlay_dir)) \
    , \
   ) \
  $(eval # 4. common level(v.s android default) resource overlay) \
  $(eval _common_overlay_dir := $(MTK_ROOT_CUSTOM)/common/resource_overlay/$(cf)) \
  $(if $(wildcard $(_common_overlay_dir)), \
    $(eval PRODUCT_PACKAGE_OVERLAYS += $(_common_overlay_dir)) \
    , \
   ) \
 )

PRODUCT_COPY_FILES += mediatek/frameworks/base/telephony/etc/apns-conf-emulator.xml:system/etc/apns-conf.xml
PRODUCT_COPY_FILES += mediatek/frameworks/base/telephony/etc/spn-conf.xml:system/etc/spn-conf.xml

# load audio files
$(call inherit-product-if-exists, frameworks/base/data/sounds/AllAudio.mk)
$(call inherit-product-if-exists, external/svox/pico/lang/all_pico_languages.mk)

# This add-on extends the default sdk product.
$(call inherit-product, $(SRC_TARGET_DIR)/product/sdk.mk)


# Real name of the add-on. This is the name used to build the add-on.
# Use 'make PRODUCT-<PRODUCT_NAME>-sdk_addon' to build the add-on.
PRODUCT_NAME := banyan_addon
PRODUCT_DEVICE := banyan_addon
PRODUCT_BRAND := banyan_addon
