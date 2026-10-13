LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := camredirect
LOCAL_SRC_FILES := module.cpp hook_helpers.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../../libs/lsplant/include $(LOCAL_PATH)/../../libs/shadowhook/include
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
