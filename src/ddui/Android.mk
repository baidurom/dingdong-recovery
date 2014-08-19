LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

libdd_common_includes = $(LOCAL_PATH)/include $(LOCAL_PATH)/src
libdd_common_src_files :=libs/png/png.c \
    libs/png/pngerror.c \
    libs/png/pnggccrd.c \
    libs/png/pngget.c \
    libs/png/pngmem.c \
    libs/png/pngpread.c \
    libs/png/pngread.c \
    libs/png/pngrio.c \
    libs/png/pngrtran.c \
    libs/png/pngrutil.c \
    libs/png/pngset.c \
    libs/png/pngtrans.c \
    libs/png/pngvcrd.c \
    libs/minutf8/minutf8.c \
    libs/freetype/autofit/autofit.c \
    libs/freetype/base/basepic.c \
    libs/freetype/base/ftapi.c \
    libs/freetype/base/ftbase.c \
    libs/freetype/base/ftbbox.c \
    libs/freetype/base/ftbitmap.c \
    libs/freetype/base/ftdbgmem.c \
    libs/freetype/base/ftdebug.c \
    libs/freetype/base/ftglyph.c \
    libs/freetype/base/ftinit.c \
    libs/freetype/base/ftpic.c \
    libs/freetype/base/ftstroke.c \
    libs/freetype/base/ftsynth.c \
    libs/freetype/base/ftsystem.c \
    libs/freetype/cff/cff.c \
    libs/freetype/pshinter/pshinter.c \
    libs/freetype/psnames/psnames.c \
    libs/freetype/raster/raster.c \
    libs/freetype/sfnt/sfnt.c \
    libs/freetype/smooth/smooth.c \
    libs/freetype/truetype/truetype.c \
	src/edify/lex.yy.c \
	src/edify/parser.c \
	src/edify/expr.c \
    src/libs/dd_array.c \
    src/libs/dd_freetype.c \
    src/libs/dd_graph.c \
    src/libs/dd_input.c \
    src/libs/dd_languages.c \
    src/libs/dd_libs.c \
    src/libs/dd_png.c \
    src/controls/dd_control_button.c \
    src/controls/dd_control_virtualkeys.c \
    src/controls/dd_control_check.c \
    src/controls/dd_control_optbox.c \
    src/controls/dd_control_textbox.c \
    src/controls/dd_control_checkbox.c \
    src/controls/dd_control_menublock.c \
    src/controls/dd_control_menubox.c \
    src/controls/dd_control_sdmenu.c \
    src/controls/dd_control_title.c \
    src/controls/dd_controls.c \
    src/controls/dd_control_threads.c \
    src/main/dd_ui.c \
    src/main/dd.c \
    src/main/dd_common_ui.c \
    src/main/dd_sd_file.c \
    src/main/dd_menu_sd.c \
    src/main/dd_menu_power.c \
    src/main/dd_menu_mount.c \
    src/main/dd_menu_wipe.c \
    src/main/dd_menu_backup.c \
    src/main/dd_menu_advanced.c \
    src/main/dd_menu_node.c \
    src/main/dd_install_package.c \
    src/main/dd_progress.c \
	src/main/dd_lang_init.c


LOCAL_SRC_FILES := \
    $(libdd_common_src_files)
LOCAL_C_INCLUDES += $(libdd_common_includes)
LOCAL_CFLAGS := $(MYDEFINE_CFLAGS)

LOCAL_STATIC_LIBRARIES += libc libm
LOCAL_MODULE := libdd

include $(BUILD_STATIC_LIBRARY)

