#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include <opencv2/core/core.hpp>

#include "Macros.h"
#include "FaceLandmarkModel.h"

extern std::string appFolder;
namespace FEA
{
	namespace Helper
	{
		extern bool is_ConfigValid;
		extern bool isTrack;
		extern std::vector<std::vector<std::string>> config_contents;
		
		bool loadConfig(FaceLandmarkModel& face_lm_model);
		bool loadFiles(std::string str, double(*p_fileValues)[2]);
		bool loadFiles(std::string str, std::vector<std::vector<double>>& p_fileValues);
		bool loadFiles(std::string str, std::vector<std::vector<std::string>>& p_fileValues);

		cv::Mat convertVecToMat(std::vector<std::vector<double>>& vec_in);
		cv::Mat convertVecToMat(std::vector<cv::Point2d>& vec_in);

		std::string get_resultStr(RESULT result);
	};
}

#endif