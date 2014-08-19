#
# ************IMPORTANT*******************************
# Baidu DSDS Features Default Definitions.
# Baidudsdsfeature makefile in /devices/baidu/common in only for sample.
# Different products or roms should redefine these values.
#
# Integration Steps:
# 1.copy this file to rom/product device directory and rename to
#   baidudsdsfeature.mk
# 2.call or include the copy in step1 in rom/product mk file.
# 3.redefine values in baidudsdsfeature.mk.
#
#*************IMPORTANT*******************************

# support dual sim or not, 
# if this value is true, one of BAIDU_FEATURE_MDSDS 
# and BAIDU_FEATURE_QDSDS should be true, and only one.
  BAIDU_FEATURE_DUALSIM := true

# support MTK dual sim or not
  BAIDU_FEATURE_MDSDS := true

# support Qualcomm sual sim or not
  BAIDU_FEATURE_QDSDS := false

# support Some OT features or not in Contacts,Mms and Phone
  BAIDU_FEATURE_OTENABLE := false


#************PROPERTY DEFINITION**********************
# build type definition, property value can be master, rom or product.
  PRODUCT_PROPERTY_OVERRIDES += \
        ro.baidu.config.buildtype=product

# dsds mode definition, property value can be single, cdma_gsm, umts_gsm or gsm_gsm.
# if BAIDU_FEATURE_DUALSIM is true, then this value can not be single
  PRODUCT_PROPERTY_OVERRIDES += \
        ro.baidu.config.dsdsmode=umts_gsm
	
PRODUCT_PACKAGES += \
    BaiduDualCardSetting \
	BaiduFM

# ---------------------- end -------------------------
