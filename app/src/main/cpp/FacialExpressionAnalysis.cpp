#include <opencv2/core/types.hpp>
#include "FacialExpressionAnalysis.h"
#include <android/log.h>

//#define APPNAME "Thiswillwork"


using namespace FEA;

bool largestFace(cv::Rect r1, cv::Rect r2)
{
	return r1.area() > r2.area();
}

std::string appFolder;

FEA::RESULT FacialExpressionAnalysis::initialize(int VERBOSE_LEVEL_, std::string folder)
{

    appFolder = folder;
//    // ##### Logging configuration ####
//    el::Configurations conf("logconf.txt");
//    el::Loggers::reconfigureLogger("default", conf);
//    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
//    el::Loggers::setVerboseLevel(VERBOSE_LEVEL_);
//    // ##########################
    __android_log_write(ANDROID_LOG_ERROR, "FACEIAL BLA", "initialization opened");
    __android_log_write(ANDROID_LOG_ERROR, "FACEIAL BLA", appFolder.c_str());
    FEA::RESULT init = OK;

    // Expiry check-----------------------------------------------------
//    FEA::RESULT expiryCheck = lic.check_licence();
//    if (expiryCheck != OK)
//        return expiryCheck;
//    LOG(INFO) << "Licence ok";
    // --------------------------------------------------------------------

    FaceLandmarkModel sm;
    sm.set_modelType(FEA::FaceLandmarkModel::ModelType::ADSC);

    init = expr.init(sm);
    if (init != OK)
        return init;

    init = pd.init();
    if (init != OK)
        return init;
//    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "My Log", 1);
//    __android_log_write(ANDROID_LOG_INFO, "ASDASD", "CHECKHECHK");
//    std::string face_cascade_name = "/storage/emulated/0/haarcascade_frontalface_alt2.xml";
//	if (!face_cascade.load(face_cascade_name))
//	{
//        __android_log_write(ANDROID_LOG_ERROR, "FACEIAL BLA", "Face detection Haar Cascade file could not be loaded");
////		LOG(WARNING) << "Face detection Haar Cascade file could not be loaded";
//	}
//	LOG(TRACE) << "FEA trace";
	return init;
}


FEA::RESULT FacialExpressionAnalysis::calc_expression(cv::Mat& img, cv::Rect& bbox, bool isTrack, double avi[], double ypr[], std::string expr_word[], float& confidence_score, cv::Mat& facepoints)
{
//	cv::Mat facepoints;
	confidence_score = 0.0f;
	FEA::RESULT result = OK;

//   INITIALIZATION
//    FaceLandmarkModel sm;
//    sm.set_modelType(FEA::FaceLandmarkModel::ModelType::ADSC);

//    expr.init(sm);

//    pd.init();

//    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "My Log", 1);
//    __android_log_write(ANDROID_LOG_INFO, "ASDASD", "CHECKHECHK");
//    std::string face_cascade_name = "/storage/emulated/0/haarcascade_frontalface_alt2.xml";
//    if (!face_cascade.load(face_cascade_name))
//    {
//        __android_log_write(ANDROID_LOG_ERROR, "FACEIAL BLA", "Face detection Haar Cascade file could not be loaded");
////		LOG(WARNING) << "Face detection Haar Cascade file could not be loaded";
//    }


//
//    pd.init();
//    std::string face_cascade_name = "/storage/emulated/0/haarcascade_frontalface_alt2.xml";
//    face_cascade.load(face_cascade_name);
	// only detection
//		if (bbox.area() == 0) // perform face detection
//		{
//			if (face_cascade.empty()) {
//                __android_log_write(ANDROID_LOG_ERROR, "FACIAL BLA",
//                                    "FACE_DETECTOR_MODEL_NOT_INITIALIZED1");
//                return FACE_DETECTOR_MODEL_NOT_INITIALIZED;
//            }else{
//                __android_log_write(ANDROID_LOG_ERROR, "FACIAL BLA",
//                                    "FACE_DETECTOR_MODEL_IS_INITIALIZED1");
//            }
//			std::vector<cv::Rect> faces;
//			face_cascade.detectMultiScale(img, faces, 1.2, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(min_face_size, min_face_size));
//			if (!faces.empty())
//			{
//				std::sort(faces.begin(), faces.end(), largestFace);
//				bbox = faces[0];
//			}
//		}
        std::ostringstream stringStream;
        stringStream << "BBOX is: " << bbox.area();
        stringStream << " Rows before is: " << facepoints.cols;

		result = pd.detect_points(img, bbox, facepoints, confidence_score);

        stringStream << " Rows after is: " << facepoints.cols;
        __android_log_write(ANDROID_LOG_ERROR, "FACIAL BLA", stringStream.str().c_str());
	prev.score = confidence_score;

	if (confidence_score < confidence_score_threshold)
	{
		prev.facepoints = cv::Mat(); // clear the points
		result = CONFIDENCE_SCORE_LOW;
	}
	else
	{

//        Expression expr;
//        expr.init(sm);

		prev.facepoints = facepoints;

		result = expr.estimate_expression(facepoints, avi, ypr, expr_word);
		//std::cout << "here: " << avi[0] << std::endl; //debug
	}


	return result;
}

