LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    Client.cpp                              \
    DisplayDevice.cpp                       \
    EventThread.cpp                         \
    Layer.cpp                               \
    LayerBase.cpp                           \
    LayerDim.cpp                            \
    LayerScreenshot.cpp                     \
    DisplayHardware/FramebufferSurface.cpp  \
    DisplayHardware/GraphicBufferAlloc.cpp  \
    DisplayHardware/HWComposer.cpp          \
    DisplayHardware/PowerHAL.cpp            \
    GLExtensions.cpp                        \
    MessageQueue.cpp                        \
    SurfaceFlinger.cpp                      \
    SurfaceTextureLayer.cpp                 \
    Transform.cpp                           \
    

LOCAL_CFLAGS:= -DLOG_TAG=\"SurfaceFlinger\"
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

ifeq ($(TARGET_BOARD_PLATFORM),omap3)
	LOCAL_CFLAGS += -DNO_RGBX_8888
endif
ifeq ($(TARGET_BOARD_PLATFORM),omap4)
	LOCAL_CFLAGS += -DHAS_CONTEXT_PRIORITY
endif
ifeq ($(TARGET_BOARD_PLATFORM),s5pc110)
	LOCAL_CFLAGS += -DHAS_CONTEXT_PRIORITY
	LOCAL_CFLAGS += -DNEVER_DEFAULT_TO_ASYNC_MODE
endif

ifeq ($(TARGET_DISABLE_TRIPLE_BUFFERING),true)
	LOCAL_CFLAGS += -DTARGET_DISABLE_TRIPLE_BUFFERING
endif

ifneq ($(NUM_FRAMEBUFFER_SURFACE_BUFFERS),)
  LOCAL_CFLAGS += -DNUM_FRAMEBUFFER_SURFACE_BUFFERS=$(NUM_FRAMEBUFFER_SURFACE_BUFFERS)
endif

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdl \
	libhardware \
	libutils \
	libEGL \
	libGLESv1_CM \
	libbinder \
	libui \
	libgui

# --- MediaTek ---------------------------------------------------------------
LOCAL_SRC_FILES += \
	mediatek/SurfaceFlinger.cpp \
	mediatek/Layer.cpp \
	mediatek/LayerBase.cpp \
	mediatek/LayerDim.cpp \
	mediatek/LayerScreenshot.cpp \
	mediatek/DisplayHardware/HWComposer.cpp \
	mediatek/SurfaceTextureLayer.cpp \
	mediatek/SurfaceFlingerWatchDog.cpp

ifeq ($(strip $(TARGET_BUILD_VARIANT)), user)
	LOCAL_CFLAGS += -DMTK_USER_BUILD
endif

ifeq ($(MTK_GPU_SUPPORT), yes)
	LOCAL_CFLAGS += -DMTK_GPU_SUPPORT
endif

ifneq ($(MTK_TABLET_HARDWARE), )
	MTK_HWC_CHIP = $(shell echo $(MTK_TABLET_HARDWARE) | tr A-Z a-z )
else
	MTK_HWC_CHIP = $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
endif

ifeq ($(MTK_HWC_SUPPORT), yes)
	LOCAL_REQUIRED_MODULES += hwcomposer.$(MTK_HWC_CHIP)
endif

ifeq ($(MTK_HWC_SUPPORT_V0), yes)
	LOCAL_CFLAGS += -DMTK_HWC_SUPPORT_V0
	LOCAL_REQUIRED_MODULES += hwcomposer.$(MTK_HWC_CHIP)
endif

ifeq ($(MTK_TRIPLE_FRAMEBUFFER_SUPPORT),yes)
	LOCAL_CFLAGS += -DNUM_FRAMEBUFFER_SURFACE_BUFFERS=3
endif

LOCAL_SHARED_LIBRARIES += libskia

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/mmumapper \
	external/skia/include/core \
	external/skia/include/images
# ----------------------------------------------------------------------------

LOCAL_MODULE:= libsurfaceflinger

include $(BUILD_SHARED_LIBRARY)

###############################################################
# uses jni which may not be available in PDK
ifneq ($(wildcard libnativehelper/include),)
include $(CLEAR_VARS)
LOCAL_CFLAGS:= -DLOG_TAG=\"SurfaceFlinger\"

LOCAL_SRC_FILES:= \
    DdmConnection.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdl

LOCAL_MODULE:= libsurfaceflinger_ddmconnection

include $(BUILD_SHARED_LIBRARY)
endif # libnativehelper
