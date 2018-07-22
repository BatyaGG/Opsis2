#include <jni.h>
#include <string>
#include <opencv2/core/mat.hpp>
#include "Macros.h"
#include "FacialExpressionAnalysis.h"
#include <android/log.h>
//
//#define APPNAME "Thiswillwork"

FacialExpressionAnalysis fea = FacialExpressionAnalysis();
//cv::Mat facepoints;

void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
    if (!AStr) {
        ARes.clear();
        return;
    }

    const char *s = AEnv->GetStringUTFChars(AStr,NULL);
    ARes=s;
    AEnv->ReleaseStringUTFChars(AStr,s);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_opsis_opsis2_FaceTrackerActivity_initialize(
        JNIEnv *env,
        jobject /* this */,
        jstring appFolder) {
    std::string stdAppFolder;
    GetJStringContent(env, appFolder, stdAppFolder);
    fea.initialize(1, stdAppFolder);

//    return reinterpret_cast<jlong>(&fea);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_opsis_opsis2_FaceGraphic_stringFromJNI(
        JNIEnv *env,
        jobject /* this */, jlong ImageAddress, jlong facePointsAddress) {

    float score;
    double avi[3], ypr[3];
    std::string expr_word[2];
    cv::Rect bbox;

    //-------------------------------------------------------//
//    std::ostringstream loga;
//    loga << FEA;

//    __android_log_write(ANDROID_LOG_ERROR, "NATIVA",loga.str().c_str());
//    FacialExpressionAnalysis &fea = *(FacialExpressionAnalysis *) FEA;

//    FacialExpressionAnalysis** add = reinterpret_cast<FacialExpressionAnalysis**>(FEA);
//    FacialExpressionAnalysis fea = **add;

//    FacialExpressionAnalysis* fea = reinterpret_cast<FacialExpressionAnalysis*>(FEA);

//    __android_log_write(ANDROID_LOG_ERROR, "NATIVA","Blagopolu4no zacepilis");

//    FacialExpressionAnalysis fea;
//    fea.initialize(1);
//    fea.initialize(1);
    cv::Mat &matAddress = *(cv::Mat *) ImageAddress;
    cv::Mat &facepoints = *(cv::Mat *) facePointsAddress;
    bbox = cv::Rect(0, 0, matAddress.cols, matAddress.rows);
    std:: stringstream asd;


    FEA::RESULT res = fea.calc_expression(matAddress, bbox, false, avi, ypr, expr_word, score, facepoints);
    asd << facepoints;
    __android_log_write(ANDROID_LOG_ERROR, "NATIVA", asd.str().c_str());
    std::ostringstream sstream;
    sstream << res << "\n" << avi[0] << "\n" << avi[1] << "\n" << avi[2] << "\n" << ypr[0] << "\n" << ypr[1] << "\n" << ypr[2] << "\n" << expr_word[0] << " " << expr_word[1] << "\n" << score;

    return env->NewStringUTF(sstream.str().c_str());
}
