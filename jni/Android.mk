LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := anagram
LOCAL_SRC_FILES := agram_wc.c anagram.c anagrams.c astr.c is_anagram.c is_within.c lcwc.c lettercounts.c wc.c word.c words_from.c
LOCAL_CFLAGS := -DPLATFORM='"$(TARGET_ARCH_ABI)"'

include $(BUILD_SHARED_LIBRARY)
