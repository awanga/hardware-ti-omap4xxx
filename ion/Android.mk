# only include if running on an omap4 platform
ifeq ($(TARGET_BOARD_PLATFORM),omap4)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := ion.c
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libion_ti
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils liblog
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

endif
