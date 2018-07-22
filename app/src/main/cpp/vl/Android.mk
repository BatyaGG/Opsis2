LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS := -DVL_DISABLE_SSE2 -DVL_DISABLE_AVX
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.c)
FILE_LIST := $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_LDLIBS += -llog -lm
#LOCAL_SRC_FILES := aib.c array.c covdet.c dsift.c fisher.c generic.c getopt_long.c gmm.c hikmeans.c hog.c homkermap.c host.c ikmeans.c imopv.c kdtree.c kmeans.c lbp.c liop.c mathop.c mser.cto cma pgm.c quickshift.c random.c rodrigues.c scalespace.c sift.c slic.c stringop.c svm.c svmdataset.c imopv_sse2.c mathop_sse2.c mathop_avx vlad.c mathop_avx.c
LOCAL_MODULE := vl_feat
include $(BUILD_SHARED_LIBRARY)