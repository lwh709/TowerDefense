LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp


# 遍历目录及子目录的函数
define walk
    $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

# 遍历Classes目录
ALLFILES = $(call walk, $(LOCAL_PATH)/../../Classes)

FILE_LIST := hellocpp/main.cpp

# 从所有文件中提取出所有.cpp文件
FILE_LIST += $(filter %.cpp, $(ALLFILES))

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes


LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocosdenshion_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocos_extension_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocostudio_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocos_ui_static
LOCAL_WHOLE_STATIC_LIBRARIES += box2d_static
LOCAL_WHOLE_STATIC_LIBRARIES += chipmunk_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocos_network_static

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../Classes  \
						   $(LOCAL_PATH)/../extensions  
						   
include $(BUILD_SHARED_LIBRARY)

$(call import-module,2d)
$(call import-module,audio/android)
$(call import-module,extensions)
$(call import-module,editor-support/cocostudio)
$(call import-module,ui)
$(call import-module,Box2D)
$(call import-module,chipmunk)
$(call import-module,network)