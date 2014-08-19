LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	BitTube.cpp \
	BufferQueue.cpp \
	ConsumerBase.cpp \
	DisplayEventReceiver.cpp \
	IDisplayEventConnection.cpp \
	ISensorEventConnection.cpp \
	ISensorServer.cpp \
	ISurfaceTexture.cpp \
	Sensor.cpp \
	SensorEventQueue.cpp \
	SensorManager.cpp \
	SurfaceTexture.cpp \
	SurfaceTextureClient.cpp \
	ISurfaceComposer.cpp \
	ISurface.cpp \
	ISurfaceComposerClient.cpp \
	IGraphicBufferAlloc.cpp \
	LayerState.cpp \
	Surface.cpp \
	SurfaceComposerClient.cpp \
	DummyConsumer.cpp \
	CpuConsumer.cpp \
	BufferItemConsumer.cpp \
	GuiConfig.cpp

LOCAL_SHARED_LIBRARIES := \
	libbinder \
	libcutils \
	libEGL \
	libGLESv2 \
	libsync \
	libui \
	libutils \

# --- MediaTek -------------------------------------------------------------------------------------
LOCAL_SRC_FILES += \
	mediatek/Surface.cpp \
	mediatek/BufferQueue.cpp \
	mediatek/FpsCounter.cpp

ifeq ($(MTK_EMULATOR_SUPPORT), yes)
	LOCAL_CFLAGS += -DEMULATOR_SUPPORT
	LOCAL_SRC_FILES += mediatek/SurfaceTexture_emulator.cpp
else
	LOCAL_CFLAGS := -DLOG_TAG=\"SurfaceTexture\"
	LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
	LOCAL_SRC_FILES += mediatek/SurfaceTexture.cpp

	ifeq ($(MTK_DP_FRAMEWORK), yes)
        LOCAL_CFLAGS += -DUSE_DP
		LOCAL_SHARED_LIBRARIES += libdpframework
		LOCAL_SRC_FILES += mediatek/SurfaceTexture_dpHal.cpp
		LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/dpframework/inc
	else
		LOCAL_CFLAGS += -DUSE_MDP
		LOCAL_SRC_FILES += mediatek/SurfaceTexture_mHal.cpp
		LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/external/mhal/inc
		LOCAL_SHARED_LIBRARIES += libdl
	endif # MTK_DP_FRAMEWORK
endif # MTK_EMULATOR_SUPPORT

ifeq ($(MTK_FENCE_SUPPORT), yes)
	#LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif

ifeq ($(MTK_NATIVE_FENCE_SUPPORT), yes)
	#LOCAL_CFLAGS += -DUSE_NATIVE_FENCE_SYNC
endif

ifeq ($(MTK_PQ_SUPPORT), yes)
	LOCAL_CFLAGS += -DMTK_PQ_SUPPORT
endif

ifeq ($(MTK_75DISPLAY_ENHANCEMENT_SUPPORT), yes)
	LOCAL_CFLAGS += -DMTK_75DISPLAY_ENHANCEMENT_SUPPORT
endif
# --------------------------------------------------------------------------------------------------:

LOCAL_MODULE:= libgui

ifeq ($(TARGET_BOARD_PLATFORM), omap4)
	LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif
ifeq ($(TARGET_BOARD_PLATFORM), s5pc110)
	LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif
ifeq ($(TARGET_BOARD_PLATFORM), exynos5)
	LOCAL_CFLAGS += -DUSE_NATIVE_FENCE_SYNC
	LOCAL_CFLAGS += -DUSE_WAIT_SYNC
endif
ifneq ($(filter generic%,$(TARGET_DEVICE)),)
    # Emulator build
    LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif

ifeq ($(TARGET_BOARD_PLATFORM), msm8960)
	LOCAL_CFLAGS += -DUSE_NATIVE_FENCE_SYNC
endif

include $(BUILD_SHARED_LIBRARY)

ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call first-makefiles-under,$(LOCAL_PATH))
endif
