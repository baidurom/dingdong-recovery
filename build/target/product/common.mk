# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


#
# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This is a generic product that isn't specialized for a specific device.
# It includes the base Android platform.

TARGET_ARCH := arm

PRODUCT_PACKAGES := \
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
    libaudio.primary.default \
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
    AccountAndSyncSettings \
    DeskClock \
    AlarmProvider \
    Bluetooth \
    Calculator \
    Calendar \
    CertInstaller \
    DrmProvider \
    Email \
    FusedLocation \
    TelephonyProvider \
    Exchange2 \
    Settings \
    Sync \
    SystemUI \
    Updater \
    CalendarProvider \
    ccci_mdinit \
    ccci_fsd \
    permission_check \
    SyncProvider \
    mediatek-res \
    disableapplist.txt \
    resmonwhitelist.txt \
    AtciService \
    MTKThermalManager \
    thermal_manager \
    thermal \
    CDS_INFO \
    ApplicationGuide \
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
    mt6620_fm_cust.cfg \
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
    gralloc.mt8125.so \
    gralloc.mt8389.so \
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
    libMtkOmxAdpcmDec \
    libMtkOmxWmaDec \
    libMtkOmxAMRNBDec \
    libMtkOmxAMRWBDec \
    libvoicerecognition_jni \
    libvoicerecognition \
    libasf \
    libasfextractor \
    audio.primary.default \
    audio_policy.stub \
    audio_policy.default \
    libaudio.primary.default \
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
    libmnlp_mt6628 \
    libmnlp_mt6620 \
    libmnlp_mt3332 \
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
    libext \
    libext4 \
    libext6 \
    libxtables \
    libip4tc \
    libip6tc \
    ipod \
    libipod \
    ipohctl \
    boot_logo_updater\
    boot_logo\
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
    robotium \
    libc_malloc_debug_mtk \
    dpfd \
    libaal \
    aal \
    SchedulePowerOnOff \
    BatteryWarning \
    pq \
    wlan_loader \
    ext4_resize \
    libphonemotiondetector_jni \
    libphonemotiondetector \
    libmotionrecognition \
    send_bug \
    poad \
    libMtkOmxRawDec \
    libperfservice \
    libperfservice_test \
    libJniAtvService \
    libhwinfo

ifeq ($(strip $(MTK_FLV_PLAYBACK_SUPPORT)), yes)
    PRODUCT_PACKAGES += libflv \
    libflvextractor
endif
# Vanzo:songlixin on: Tue, 19 Jul 2011 11:11:12 +0800
# Add some extra apps
  PRODUCT_PACKAGES += UpdateSystem \
    nvram_backup_binder \
    FactoryMode \
    Torch \
    TaskManager2 \
    Ringdroid \
    WallpaperChooser

define search_and_add_to_product_package
    $(foreach file1,$(1), \
    $(eval PRODUCT_PACKAGES+= $(basename $(notdir $(wildcard $(file1)/*.$(strip $(2)))))))
endef

define search_and_add_to_product_package2
    $(foreach file1,$(1), \
    $(eval PRODUCT_PACKAGES+= $(notdir $(wildcard $(file1)/*.$(strip $(2))))))
endef

$(call search_and_add_to_product_package, vendor/google vendor/google/3rdapp/custom vendor/google/3rdapp vendor/google/app vendor/google/superuser, apk)
$(call search_and_add_to_product_package, vendor/google/framework, jar)
$(call search_and_add_to_product_package2, vendor/google/permissions, xml)
$(call search_and_add_to_product_package2, vendor/google/userdata/custom vendor/google/userdata, apk)
$(call search_and_add_to_product_package, vendor/google/lib, so)

PRODUCT_PACKAGES += bootanimation.zip shutanimation.zip bootaudio.mp3 shutaudio.mp3

# End of Vanzo:songlixin

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
    PRODUCT_PACKAGES += nfcservice
endif
#
ifeq ($(strip $(MTK_CMAS_SUPPORT)), yes)
  PRODUCT_PACKAGES += CMASReceiver \
                      CmasEM
endif
#
ifeq ($(strip $(MTK_COMBO_SUPPORT)), yes)

BUILD_MT6620 := false
BUILD_MT6628 := false
 
  
  PRODUCT_PACKAGES += WMT.cfg \
    6620_launcher \
    6620_wmt_concurrency \
    stp_dump3

ifneq ($(filter MT6620E3,$(MTK_COMBO_CHIP)),)
    BUILD_MT6620 := true
endif

ifneq ($(filter MT6620,$(MTK_COMBO_CHIP)),)
    BUILD_MT6620 := true
endif

ifneq ($(filter MT6628,$(MTK_COMBO_CHIP)),)
    BUILD_MT6628 := true
endif

ifeq ($(BUILD_MT6620), true)
  PRODUCT_PACKAGES += mt6620_patch_e3_hdr.bin \
    mt6620_patch_e3_0_hdr.bin \
    mt6620_patch_e3_1_hdr.bin \
    mt6620_patch_e3_2_hdr.bin \
    mt6620_patch_e3_3_hdr.bin \
    mt6620_patch_e6_hdr.bin
    
  ifneq ($(filter mt6620_ant_m1,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m1.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m2,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m2.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m3,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m3.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m4,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m4.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m5,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m5.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m6,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m6.cfg
  endif
    
  ifneq ($(filter mt6620_ant_m7,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6620_ant_m7.cfg
  endif
           
endif

ifeq ($(strip $(MTK_GAMELOFT_GLL_APP)), yes)
      PRODUCT_PACKAGES += GLLive_CN_qHD_960x540
endif

ifeq ($(strip $(MTK_GAMELOFT_LBC_APP)), yes)
      PRODUCT_PACKAGES += LittleBigCity_CN_qHD_960x540
endif

ifeq ($(strip $(MTK_CMCC_MOBILEMARKET_SUPPORT)), yes)
      PRODUCT_PACKAGES += CMCC_MobileMarket
endif

ifeq ($(strip $(MTK_GAMELOFT_SD_APP)), yes)
      PRODUCT_PACKAGES += SharkDash_CN_qHD_960x540
endif
#MTK_VIDEO_THUMBNAIL_PLAY_SUPPORT
ifeq ($(strip $(MTK_VIDEO_THUMBNAIL_PLAY_SUPPORT)),yes)
  PRODUCT_PACKAGES += libjtranscode
endif

ifeq ($(BUILD_MT6628), true)
  PRODUCT_PACKAGES += mt6628_patch_e1_hdr.bin \
    mt6628_patch_e2_hdr.bin \
    mt6628_patch_e2_0_hdr.bin \
    mt6628_patch_e2_1_hdr.bin 
  
  ifneq ($(filter mt6628_ant_m1,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m1.cfg
  endif
  
  ifneq ($(filter mt6628_ant_m2,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m2.cfg
  endif
    
  ifneq ($(filter mt6628_ant_m3,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m3.cfg
  endif
    
  ifneq ($(filter mt6628_ant_m4,$(CUSTOM_HAL_ANT)),)
    PRODUCT_PACKAGES += mt6628_ant_m4.cfg
  endif
  
endif   

ifeq ($(strip $(MTK_VOICE_UNLOCK_SUPPORT)),yes)
  PRODUCT_PACKAGES += VoiceUnlock
endif
    

endif

ifeq ($(strip $(MTK_EMMC_SUPPORT)), yes)
  PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=EXT4
else
  ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)), yes)
    PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=UBIFS
  else
    PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=YAFFS
  endif
endif

ifeq ($(strip $(MTK_DATAUSAGE_SUPPORT)), yes)
  ifeq ($(strip $(MTK_DATAUSAGELOCKSCREENCLIENT_SUPPORT)), yes)
    PRODUCT_PACKAGES += DataUsageLockScreenClient 
  endif
endif

ifeq ($(strip $(MTK_ENABLE_MD1)),yes)
  PRODUCT_PACKAGES += modem.img
  ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
    PRODUCT_PACKAGES += catcher_filter.bin
  endif
endif


PRODUCT_COPY_FILES += \
   system/extras/bugmailer/bugmailer.sh:system/bin/bugmailer.sh \
    system/extras/bugmailer/send_bug:system/bin/send_bug

ifeq ($(strip $(MTK_ENABLE_MD2)),yes)
  PRODUCT_PACKAGES += modem_sys2.img
  ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
    PRODUCT_PACKAGES += catcher_filter_sys2.bin
  endif
endif

ifeq ($(strip $(MTK_ISMS_SUPPORT)), yes)
  PRODUCT_PACKAGES += ISmsService
endif

ifeq ($(strip $(MTK_ETWS_SUPPORT)), yes)
  PRODUCT_PACKAGES += CellBroadcastReceiver
endif

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  ifeq ($(strip $(MTK_NFC_MT6605)), yes)
    PRODUCT_PACKAGES += libmtknfc_mw
  endif
endif

ifeq ($(strip $(MTK_MTKLOGGER_SUPPORT)), yes)
  PRODUCT_PACKAGES += MTKLogger
endif

ifeq ($(strip $(MTK_SPECIFIC_SM_CAUSE)), yes)
  PRODUCT_PROPERTY_OVERRIDES += \
  ril.specific.sm_cause=1
else
  PRODUCT_PROPERTY_OVERRIDES += \
  ril.specific.sm_cause=0
endif 

ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
  PRODUCT_PACKAGES += SDKGallery
else
  PRODUCT_PACKAGES += Gallery2
endif

ifeq ($(strip $(MTK_BAIDU_MAP_SUPPORT)), yes)
      PRODUCT_PACKAGES += Baidu_Map
endif
ifeq ($(strip $(MTK_BAIDU_SEARCH_BAR_SUPPORT)), yes)
      PRODUCT_PACKAGES += Baidu_Search_Bar
endif

ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
  PRODUCT_PACKAGES += Provision
endif

ifeq ($(strip $(HAVE_CMMB_FEATURE)), yes)
  PRODUCT_PACKAGES += CMMBPlayer 
endif

ifeq ($(strip $(MTK_DATA_TRANSFER_APP)), yes)
  PRODUCT_PACKAGES += DataTransfer
endif

ifeq ($(strip $(MTK_MDM_APP)),yes)
  PRODUCT_PACKAGES += MediatekDM
endif

ifeq ($(strip $(MTK_VT3G324M_SUPPORT)),yes)
  PRODUCT_PACKAGES += libmtk_vt_client \
                      libmtk_vt_em \
                      libmtk_vt_utils \
                      libmtk_vt_service \
                      libmtk_vt_swip \
                      vtservice
endif

ifeq ($(strip $(MTK_OOBE_APP)),yes)
  PRODUCT_PACKAGES += OOBE
endif

ifeq ($(strip $(MTK_MEDIA3D_APP)), yes)
    PRODUCT_PACKAGES += Media3D
endif

ifdef MTK_WEATHER_PROVIDER_APP
  ifneq ($(strip $(MTK_WEATHER_PROVIDER_APP)), no)
    PRODUCT_PACKAGES += MtkWeatherProvider
  endif
endif

ifeq ($(strip $(MTK_VOICE_UNLOCK_SUPPORT)),yes)
    PRODUCT_PACKAGES += VoiceCommand
else
        ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
            PRODUCT_PACKAGES += VoiceCommand	
        endif
endif

PRODUCT_PACKAGES += CustomProperties

ifeq ($(strip $(MTK_ENABLE_VIDEO_EDITOR)),yes)
#  PRODUCT_PACKAGES += VideoEditor
endif

ifeq ($(strip $(MTK_CALENDAR_IMPORTER_APP)), yes)
  PRODUCT_PACKAGES += CalendarImporter 
endif

ifeq ($(strip $(MTK_THEMEMANAGER_APP)), yes)
  PRODUCT_PACKAGES += theme-res-mint \
                      theme-res-mocha \
                      theme-res-raspberry \
                      libtinyxml
endif

ifeq ($(strip $(MTK_VIDEOPLAYER_APP)), yes)
  PRODUCT_PACKAGES += VideoPlayer
endif

ifeq ($(strip $(MTK_VIDEOPLAYER2_APP)), yes)
  PRODUCT_PACKAGES += VideoPlayer2
endif

ifeq ($(strip $(MTK_GALLERY3D_APP)), yes)
  PRODUCT_PACKAGES += Gallery3D
endif

ifeq ($(strip $(MTK_LOG2SERVER_APP)), yes)
  PRODUCT_PACKAGES += Log2Server \
                      Excftpcommonlib \
                      Excactivationlib \
                      Excadditionnallib \
                      Excmaillib

endif

ifeq ($(strip $(MTK_GALLERY_APP)), yes)
  PRODUCT_PACKAGES += Gallery
endif

ifeq ($(strip $(MTK_INPUTMETHOD_PINYINIME_APP)), yes)
  PRODUCT_PACKAGES += PinyinIME
  PRODUCT_PACKAGES += libjni_pinyinime
endif

ifeq ($(strip $(MTK_CAMERA_APP)), yes)
  PRODUCT_PACKAGES += CameraOpen
else
  PRODUCT_PACKAGES += Camera
endif

ifeq ($(strip $(MTK_VIDEO_FAVORITES_WIDGET_APP)), yes)
  ifneq ($(strip $(MTK_TABLET_PLATFORM)), yes)
    ifneq (,$(filter hdpi xhdpi,$(MTK_PRODUCT_LOCALES)))
      PRODUCT_PACKAGES += VideoFavorites \
                          libjtranscode
    endif
  endif
endif

ifneq (,$(filter km_KH,$(MTK_PRODUCT_LOCALES)))
  PRODUCT_PACKAGES += Mondulkiri.ttf
endif
ifneq (,$(filter my_MM,$(MTK_PRODUCT_LOCALES)))
  PRODUCT_PACKAGES += Padauk.ttf
endif

ifeq ($(strip $(MTK_VIDEOWIDGET_APP)),yes)
  PRODUCT_PACKAGES += MtkVideoWidget
endif

ifeq ($(strip $(MTK_BSP_PACKAGE)),yes)
  PRODUCT_PACKAGES += Stk
else
  PRODUCT_PACKAGES += Stk1
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += EngineerMode \
                      EngineerModeSim \
                      libem_bt_jni \
                      libem_support_jni \
                      libem_gpio_jni \
                      libem_modem_jni \
                      libem_usb_jni \
                      libem_wifi_jni
  ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
      PRODUCT_PACKAGES += libem_nfc_jni
  endif
endif

ifeq ($(strip $(MTK_RCSE_SUPPORT)), yes)
    PRODUCT_PACKAGES += Rcse
    PRODUCT_PACKAGES += Provisioning
endif

ifeq ($(strip $(MTK_GPS_SUPPORT)), yes)
  PRODUCT_PACKAGES += YGPS
endif

ifeq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)), yes)
  PRODUCT_PACKAGES += Stereo3DWallpaper                                          
endif

ifeq ($(strip $(MTK_DATAREG_APP)),yes)
  PRODUCT_PACKAGES += DataReg
  PRODUCT_PACKAGES += DataRegSecrets
  PRODUCT_PACKAGES += DataRegDefault.properties
endif

ifeq ($(strip $(MTK_GPS_SUPPORT)), yes)
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT6620)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=1
  endif
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT6628)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=1
  endif
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT3332)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=0
  endif
endif

ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)),yes)
  PRODUCT_PACKAGES += mkfs_ubifs \
                      ubinize                       
endif

ifeq ($(strip $(MTK_EXTERNAL_MODEM_SLOT)),2)
  PRODUCT_PROPERTY_OVERRIDES += \
  ril.external.md=2
else
  ifeq ($(strip $(MTK_EXTERNAL_MODEM_SLOT)),1)
    PRODUCT_PROPERTY_OVERRIDES += \
    ril.external.md=1
  else
    PRODUCT_PROPERTY_OVERRIDES += \
    ril.external.md=0
  endif
endif

ifeq ($(strip $(MTK_LIVEWALLPAPER_APP)), yes)
  PRODUCT_PACKAGES += LiveWallpapers \
                      LiveWallpapersPicker \
                      MagicSmokeWallpapers \
                      VisualizationWallpapers \
                      Galaxy4 \
                      HoloSpiralWallpaper \
                      NoiseField \
                      PhaseBeam 
endif

ifeq ($(strip $(MTK_VLW_APP)), yes)
  PRODUCT_PACKAGES += MtkVideoLiveWallpaper
endif

ifeq ($(strip $(MTK_SNS_SUPPORT)), yes)
  PRODUCT_PACKAGES += SNSCommon \
                      SnsContentProvider \
                      SnsWidget \
                      SnsWidget24 \
                      SocialStream       
  ifeq ($(strip $(MTK_SNS_KAIXIN_APP)), yes)
    PRODUCT_PACKAGES += KaiXinAccountService
  endif
  ifeq ($(strip $(MTK_SNS_RENREN_APP)), yes)
    PRODUCT_PACKAGES += RenRenAccountService
  endif
  ifeq ($(strip $(MTK_SNS_FACEBOOK_APP)), yes)
    PRODUCT_PACKAGES += FacebookAccountService
  endif
  ifeq ($(strip $(MTK_SNS_FLICKR_APP)), yes)
    PRODUCT_PACKAGES += FlickrAccountService
  endif
  ifeq ($(strip $(MTK_SNS_TWITTER_APP)), yes)
    PRODUCT_PACKAGES += TwitterAccountService
  endif
  ifeq ($(strip $(MTK_SNS_SINAWEIBO_APP)), yes)
    PRODUCT_PACKAGES += WeiboAccountService
  endif
endif

ifeq ($(strip $(MTK_SYSTEM_UPDATE_SUPPORT)), yes)
  PRODUCT_PACKAGES += SystemUpdate \
                      SystemUpdateAssistant
endif

ifeq ($(strip $(MTK_DATADIALOG_APP)), yes)
  PRODUCT_PACKAGES += DataDialog
endif

ifeq ($(strip $(MTK_DATA_TRANSFER_APP)), yes)
  PRODUCT_PACKAGES += DataTransfer
endif

ifeq ($(strip $(MTK_FM_SUPPORT)), yes)
  PRODUCT_PACKAGES += FMRadio
endif

ifeq (MT6620_FM,$(strip $(MTK_FM_CHIP)))
    PRODUCT_PROPERTY_OVERRIDES += \
        fmradio.driver.chip=1
endif

ifeq (MT6626_FM,$(strip $(MTK_FM_CHIP)))
    PRODUCT_PROPERTY_OVERRIDES += \
        fmradio.driver.chip=2
endif

ifeq (MT6628_FM,$(strip $(MTK_FM_CHIP)))
    PRODUCT_PROPERTY_OVERRIDES += \
        fmradio.driver.chip=3
endif

ifeq ($(strip $(MTK_BT_SUPPORT)), yes)
  PRODUCT_PACKAGES += MtkBt \
        libbtcusttable \
        libbtcust \
        libmtkbtextadp \
        libextpbap \
        libextavrcp \
        libextopp \
        libextsys \
        libextftp \
        libmtkbtextadpa2dp \
        libmtka2dp \
        libextbip \
        libextbpp \
        libexthid \
        libextsimap \
        libextjsr82 \
        libmtkbtextpan \
        libextmap \
        libmtkbtextspp \
        libexttestmode \
        libpppbtdun \
        libextopp_jni \
        libexthid_jni \
        libextpan_jni \
        libextftp_jni \
        libextbpp_jni \
        libextbip_jni \
        libextpbap_jni \
        libextavrcp_jni \
        libextsimap_jni \
        libextdun_jni \
        libextmap_jni \
        libextsys_jni \
        btlogmask \
        btconfig \
        libbtpcm \
        libbtsniff \
        mtkbt
    ifeq ($(strip $(MTK_WLANBT_SINGLEANT)), yes)
        PRODUCT_PACKAGES += libwifiwmt
    endif
endif

ifeq ($(strip $(MTK_DT_SUPPORT)),yes)
    ifneq ($(strip $(EVDO_DT_SUPPORT)),yes)
        ifeq ($(strip $(MTK_MDLOGGER_SUPPORT)),yes)
            PRODUCT_PACKAGES += \
                ExtModemLog \
                libextmdlogger_ctrl_jni \
                libextmdlogger_ctrl \
                extmdlogger
        endif
    endif
endif

ifeq ($(strip $(MTK_ACWFDIALOG_APP)), yes)
  PRODUCT_PACKAGES += AcwfDialog
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += EngineerMode \
                      MobileLog
endif

ifeq ($(strip $(HAVE_MATV_FEATURE)),yes)
  PRODUCT_PACKAGES += MtvPlayer \
                      MATVEM    \
                      com.mediatek.atv.adapter
endif

ifneq ($(strip $(MTK_LCM_PHYSICAL_ROTATION)),)
  PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.hwrotation=$(MTK_LCM_PHYSICAL_ROTATION)
endif
# Vanzo:songlixin on: Thu, 04 Aug 2011 19:08:05 +0800
# To overlay strings
PRODUCT_PACKAGE_OVERLAYS += vendor/overlay_trans
PRODUCT_PACKAGE_OVERLAYS += vendor/overlay_res
# End of Vanzo:songlixin


ifeq ($(strip $(MTK_SHARE_MODEM_CURRENT)),2)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.current.share_modem=2
else
  ifeq ($(strip $(MTK_SHARE_MODEM_CURRENT)),1)
    PRODUCT_PROPERTY_OVERRIDES += \
      ril.current.share_modem=1
  else
    PRODUCT_PROPERTY_OVERRIDES += \
      ril.current.share_modem=0
  endif
endif

ifeq ($(strip $(MTK_FM_TX_SUPPORT)), yes)
  PRODUCT_PACKAGES += FMTransmitter
endif

ifeq ($(strip $(MTK_SOUNDRECORDER_APP)),yes)
  PRODUCT_PACKAGES += SoundRecorder
endif

ifeq ($(strip $(MTK_DM_APP)),yes)
  PRODUCT_PACKAGES += dm 
endif

ifeq ($(strip $(MTK_WEATHER3D_WIDGET)), yes)
  ifneq ($(strip $(MTK_TABLET_PLATFORM)), yes)
    ifneq (,$(filter hdpi xhdpi,$(MTK_PRODUCT_LOCALES)))
      PRODUCT_PACKAGES += Weather3DWidget
    endif
  endif
endif

ifeq ($(strip $(MTK_LAUNCHERPLUS_APP)),yes)
  PRODUCT_PACKAGES += LauncherPlus \
                      MoreApp
  PRODUCT_PROPERTY_OVERRIDES += \
    launcherplus.allappsgrid=2d
endif

ifeq ($(strip $(MTK_LAUNCHER_ALLAPPSGRID)), yes)
  PRODUCT_PROPERTY_OVERRIDES += \
          launcher2.allappsgrid=3d_20
endif

ifeq ($(strip $(MTK_LOCKSCREEN_TYPE)),2)
  PRODUCT_PACKAGES += MtkWallPaper
endif

ifneq ($(strip $(MTK_LOCKSCREEN_TYPE)),)
  PRODUCT_PROPERTY_OVERRIDES += \
    curlockscreen=$(MTK_LOCKSCREEN_TYPE)
endif

ifeq ($(strip $(MTK_IME_SUPPORT)),yes)
  PRODUCT_PACKAGES += MediatekIME
endif

ifeq ($(strip $(MTK_ANDROIDFACTORYMODE_APP)),yes)
  PRODUCT_PACKAGES += AndroidFactoryMode
endif

ifeq ($(strip $(MTK_OMA_DOWNLOAD_SUPPORT)),yes)
  PRODUCT_PACKAGES += Browser \
                      DownloadProvider
endif

ifeq ($(strip $(MTK_OMACP_SUPPORT)),yes)
  PRODUCT_PACKAGES += Omacp
endif

ifeq ($(strip $(MTK_WIFI_P2P_SUPPORT)),yes)
  PRODUCT_PACKAGES += \
    WifiContactSync \
    WifiP2PWizardy \
    FileSharingServer \
    FileSharingClient \
    UPnPAV \
    WifiWsdsrv \
    bonjourExplorer
endif
#adupsfota start
ifeq ($(strip $(ADUPS_FOTA_SUPPORT)), yes)
    PRODUCT_PACKAGES += AdupsFota \
                        AdupsFotaReboot
endif
#adupsfota end

ifeq ($(strip $(MTK_MDLOGGER_SUPPORT)),yes)
  PRODUCT_PACKAGES += \
    dualmdlogger \
    mdlogger
endif

ifeq ($(strip $(CUSTOM_KERNEL_TOUCHPANEL)),generic)
  PRODUCT_PACKAGES += Calibrator
endif

ifeq ($(strip $(MTK_FILEMANAGER_APP)), yes)
  PRODUCT_PACKAGES += FileManager
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += ActivityNetwork
endif

ifneq ($(findstring OP03, $(strip $(OPTR_SPEC_SEG_DEF))),)
  PRODUCT_PACKAGES += SimCardAuthenticationService
endif

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  PRODUCT_PACKAGES += NxpSecureElement
endif

ifeq ($(strip $(MTK_APKINSTALLER_APP)), yes)
  PRODUCT_PACKAGES += APKInstaller
endif

ifeq ($(strip $(MTK_SMSREG_APP)), yes)
  PRODUCT_PACKAGES += SmsReg
endif

ifeq ($(strip $(GEMINI)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
    ro.mediatek.gemini_support=true
else
  PRODUCT_PROPERTY_OVERRIDES += \
    ro.mediatek.gemini_support=false
endif

ifeq ($(strip $(MTK_ENGINEERMODE_INTERNAL_APP)), yes)
  PRODUCT_PACKAGES += InternalEngineerMode
endif

ifeq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)), yes)
  PRODUCT_PACKAGES += Stereo3DWallpaper
endif

ifeq ($(strip $(MTK_WEATHER3D_WIDGET)), yes)
    PRODUCT_PACKAGES += Weather3DWidget
endif

ifeq ($(strip $(MTK_YMCAPROP_SUPPORT)),yes)
  PRODUCT_COPY_FILES += mediatek/packages/yahoo_tracking/ymca.properties:system/yahoo/com.yahoo.mobile.client.android.news/ymca.properties
endif

ifeq ($(MTK_BACKUPANDRESTORE_APP),yes)
  PRODUCT_PACKAGES += BackupAndRestore
endif

ifeq ($(strip $(MTK_NOTEBOOK_SUPPORT)),yes)
  PRODUCT_PACKAGES += NoteBook 
endif

ifeq ($(strip $(MTK_BWC_SUPPORT)), yes)
    PRODUCT_PACKAGES += libbwc
endif

# Todos is a common feature on JB
PRODUCT_PACKAGES += Todos

ifeq ($(strip $(MTK_DT_SUPPORT)),yes)
  PRODUCT_PACKAGES += ip-up \
                      ip-down \
                      ppp_options \
                      chap-secrets \
                      init.gprs-pppd
endif

ifdef OPTR_SPEC_SEG_DEF
  ifneq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
    OPTR := $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SPEC := $(word 2,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SEG  := $(word 3,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    $(call inherit-product-if-exists, mediatek/operator/$(OPTR)/$(SPEC)/$(SEG)/optr_apk_config.mk)

# Todo:
# obsolete this section's configuration for operator project resource overlay
# once all operator related overlay resource moved to custom folder
    PRODUCT_PACKAGE_OVERLAYS += mediatek/operator/$(OPTR)/$(SPEC)/$(SEG)/OverLayResource
# End

    PRODUCT_PROPERTY_OVERRIDES += \
      ro.operator.optr=$(OPTR) \
      ro.operator.spec=$(SPEC) \
      ro.operator.seg=$(SEG)
  endif
endif

ifeq ($(strip $(GEMINI)), yes)
  ifeq ($(OPTR_SPEC_SEG_DEF),NONE)
    PRODUCT_PACKAGES += StkSelection
  endif
  ifeq (OP01,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    PRODUCT_PACKAGES += StkSelection
  endif
  ifndef OPTR_SPEC_SEG_DEF
    PRODUCT_PACKAGES += StkSelection
  endif
endif

ifeq ($(strip $(MTK_DATAREG_APP)),yes)
  PRODUCT_PACKAGES += DataReg
  PRODUCT_PACKAGES += DataRegSecrets
  PRODUCT_PACKAGES += DataRegDefault.properties
endif

ifeq (yes,$(strip $(MTK_FD_SUPPORT)))
# Only support the format: n.m (n:1 or 1+ digits, m:Only 1 digit) or n (n:integer)
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.radio.fd.counter=15
                
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.radio.fd.off.counter=5
        
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.radio.fd.r8.counter=15
                
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.radio.fd.off.r8.counter=5     
endif

ifeq ($(strip $(MTK_COMBO_SUPPORT)), yes)
    PRODUCT_PROPERTY_OVERRIDES += persist.mtk.wcn.combo.chipid=-1
endif

ifeq ($(strip $(MTK_WVDRM_SUPPORT)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true
  PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libdrmwvmplugin \
    libwvm \
    libWVStreamControlAPI_L3 \
    libwvdrm_L3
else
  PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=false
endif

ifeq ($(strip $(MTK_DRM_APP)),yes)
  PRODUCT_PACKAGES += \
    libdrmmtkplugin \
    drm_chmod \
    libdcfdecoderjni
endif

ifeq (yes,$(strip $(MTK_FM_SUPPORT)))
    PRODUCT_PROPERTY_OVERRIDES += \
        fmradio.driver.enable=1
else
    PRODUCT_PROPERTY_OVERRIDES += \
        fmradio.driver.enable=0
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

ifeq (yes,$(strip $(MTK_NFC_SUPPORT)))
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
                        frameworks/base/nfc-extras/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml
  PRODUCT_PACKAGES += Nfc \
		      Tag \
                      nfcc.default
  PRODUCT_PROPERTY_OVERRIDES += \
    ro.nfc.port=I2C
endif

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  ifeq ($(strip $(MTK_NFC_APP_SUPPORT)), yes)
    PRODUCT_PACKAGES += NFCTagMaster
  endif
endif

ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
  PRODUCT_PACKAGES += SRSTruMedia
  PRODUCT_PACKAGES += libsrsprocessing
endif

ifeq ($(strip $(MTK_WEATHER_WIDGET_APP)), yes)
    PRODUCT_PACKAGES += MtkWeatherWidget
endif

ifeq ($(strip $(MTK_WORLD_CLOCK_WIDGET_APP)), yes)
    PRODUCT_PACKAGES += MtkWorldClockWidget
endif

ifeq ($(strip $(MTK_FIRST_MD)),1)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.first.md=1
endif
ifeq ($(strip $(MTK_FIRST_MD)),2)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.first.md=2
endif

ifeq ($(strip $(MTK_FLIGHT_MODE_POWER_OFF_MD)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.flightmode.poweroffMD=1
else
    PRODUCT_PROPERTY_OVERRIDES += \
      ril.flightmode.poweroffMD=0
endif

ifeq ($(strip $(MTK_FIRST_MD)),1)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.first.md=1
endif
ifeq ($(strip $(MTK_FIRST_MD)),2)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.first.md=2
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),0)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=0
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),1)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=1
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),2)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=2
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),3)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=3
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),4)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=4
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),5)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=5
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),6)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=6
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),7)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=7
endif
ifeq ($(strip $(MTK_TELEPHONY_MODE)),8)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.telephony.mode=8
endif

ifeq ($(strip $(MTK_RADIOOFF_POWER_OFF_MD)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
    ril.radiooff.poweroffMD=1
else
    PRODUCT_PROPERTY_OVERRIDES += \
      ril.radiooff.poweroffMD=0
endif

ifeq ($(strip $(MTK_AGPS_APP)), yes)
  PRODUCT_PACKAGES += LocationEM
  PRODUCT_COPY_FILES += mediatek/frameworks/base/epo/etc/epo_conf.xml:system/etc/epo_conf.xml
  PRODUCT_COPY_FILES += mediatek/frameworks/base/agps/etc/agps_profiles_conf.xml:system/etc/agps_profiles_conf.xml
endif

PRODUCT_PACKAGES += libsec
PRODUCT_PACKAGES += sbchk
PRODUCT_PACKAGES += S_ANDRO_SFL.ini
PRODUCT_PACKAGES += S_SECRO_SFL.ini

PRODUCT_BRAND := alps
PRODUCT_MANUFACTURER := alps

PRODUCT_COPY_FILES += mediatek/frameworks/base/telephony/etc/apns-conf.xml:system/etc/apns-conf.xml
PRODUCT_COPY_FILES += mediatek/frameworks/base/telephony/etc/spn-conf.xml:system/etc/spn-conf.xml

# for USB Accessory Library/permission
# Mark for early porting in JB
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml
PRODUCT_PACKAGES += com.android.future.usb.accessory

# System property for MediaTek ANR pre-dump.
PRODUCT_PROPERTY_OVERRIDES += dalvik.vm.mtk-stack-trace-file=/data/anr/mtk_traces.txt

ifeq ($(strip $(MTK_WLAN_SUPPORT)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
    mediatek.wlan.chip=$(MTK_WLAN_CHIP)

  PRODUCT_PROPERTY_OVERRIDES += \
    mediatek.wlan.module.postfix="_"$(shell echo $(strip $(MTK_WLAN_CHIP)) | tr A-Z a-z)
endif

ifeq ($(strip $(MTK_RILD_READ_IMSI)),yes)
  PRODUCT_PROPERTY_OVERRIDES += \
  ril.read.imsi=1
endif



#MT6575/77 MDP Packages
ifeq ($(MTK_PLATFORM),$(filter $(MTK_PLATFORM),MT6575 MT6575T MT6577))
	PRODUCT_PACKAGES += \
		mdpd \
		mdpserver \
		libmhalmdp
endif




$(call inherit-product, $(SRC_TARGET_DIR)/product/core.mk)
$(call inherit-product-if-exists, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product-if-exists, external/lohit-fonts/fonts.mk)
$(call inherit-product-if-exists, frameworks/base/data/keyboards/keyboards.mk)
$(call inherit-product-if-exists, mediatek/frameworks-ext/base/data/sounds/AudioMtk.mk)
$(call inherit-product-if-exists, frameworks/base/data/sounds/AllAudio.mk)
$(call inherit-product-if-exists, external/svox/pico/lang/all_pico_languages.mk)
$(call inherit-product-if-exists, mediatek/external/sip/sip.mk)
$(call inherit-product-if-exists, external/naver-fonts/fonts.mk)
$(call inherit-product-if-exists, external/cibu-fonts/fonts.mk)

ifeq ($(strip $(MTK_VOICE_UNLOCK_SUPPORT)),yes)
    $(call inherit-product-if-exists, mediatek/frameworks/base/voicecommand/cfg/voicecommand.mk)
else
        ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
            $(call inherit-product-if-exists, mediatek/frameworks/base/voicecommand/cfg/voicecommand.mk)	
        endif
endif
#
ifeq ($(strip $(MTK_NFC_OMAAC_SUPPORT)),yes)
  PRODUCT_PACKAGES += SmartcardService
  PRODUCT_PACKAGES += org.simalliance.openmobileapi
  PRODUCT_PACKAGES += org.simalliance.openmobileapi.xml
  PRODUCT_PACKAGES += libassd
endif
#
