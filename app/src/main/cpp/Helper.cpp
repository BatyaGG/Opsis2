#include "Helper.h"
#include "Macros.h"
#include "FaceLandmarkModel.h"
#include <android/log.h>

using namespace FEA;

bool Helper::is_ConfigValid = false;
bool Helper::isTrack = false;
std::vector<std::vector<std::string>> Helper::config_contents(5);

std::string Helper::get_resultStr(RESULT result)
{
	//Logger::set_level(Logger::LOG_LEVEL::LOG_DEBUG);
	//Logger::log(Logger::LOG_ERROR, "A");
	switch (result)
	{
	case 0: return "Result OK";
	case 1: return "Config file failed to load";
	case 2: return "Face landmark model initialization failed";
	case 3: return "Expression module initialization failed";
	case 4: return "Face detector model initialization failed";
	case 5: return "Invalid input";
	case 6: return "Calculation error";
	case 7: return "No face available";
	case 8: return "Invalid image";
	case 9: return "Confidence score below threshold";
	case 10: return "Licence error";
	default: return "Unknown error";
	}
}

// - Load training files into array
bool Helper::loadFiles(std::string str, double(*p_fileValues)[2])
{
    __android_log_write(ANDROID_LOG_ERROR, "HELPER", "1 poletel");
	std::ifstream fileToload(str);

	//double fileValues[51][2];

	std::string line;
	if (fileToload.is_open())
	{
		int i = 0;
		while (getline(fileToload, line))
		{

			std::stringstream linestream(line);
			std::string		 values;

			int j = 0;
			while (std::getline(linestream, values, ','))
//				p_fileValues[i][j++] = stod(values);
			i++;
		}

		//(*p_fileValues)[0]=fileValues[0];
		return true;
	}

//	LOG(FATAL) << "Unable to open - " << str;
	return false;
}

// - Load training files into vector
//template <typename T>
bool Helper::loadFiles(std::string str, std::vector<std::vector<double>>& p_fileValues)
{
    __android_log_write(ANDROID_LOG_ERROR, "HELPER", "2 poletel");
	std::ifstream fileToload(str);

	std::vector<double> rowValues;

	std::string line;
	if (fileToload.is_open())
	{
		
		while (std::getline(fileToload, line))
		{

			std::stringstream linestream(line);
			std::string		 values;

			while (std::getline(linestream, values, ','))
				rowValues.push_back(std::atof(values.c_str()));

			p_fileValues.push_back(rowValues);
			rowValues.clear();
		}

		return true;
	}

//	LOG(FATAL) << "Unable to open - " << str;
	return false;
}

bool Helper::loadFiles(std::string str, std::vector<std::vector<std::string>>& p_fileValues)
{
    __android_log_write(ANDROID_LOG_ERROR, "HELPER", "3 poletel");
	std::ifstream fileToload(appFolder + str);
	std::vector<std::string> rowValues;

	std::string line;
	if (fileToload.is_open())
	{

		while (std::getline(fileToload, line))
		{
			std::stringstream linestream(line);
			std::string		 values;

			while (std::getline(linestream, values, ','))
				rowValues.push_back(values);

			p_fileValues.push_back(rowValues);
			rowValues.clear();
		}

		return true;
	}
    __android_log_write(ANDROID_LOG_ERROR, "HELPER", "UNABLE TO OPEN STR");
//	LOG(FATAL) << "Unable to open - " << str;
	return false;
}


// - Load FEA config (expression weight files and Face landmark model filename)
bool Helper::loadConfig(FaceLandmarkModel& face_lm_model)
{
	config_contents.clear();

	std::string configFileName;

	if (face_lm_model.get_modelType() == FaceLandmarkModel::FACEMAX || face_lm_model.get_modelType() == FaceLandmarkModel::INTRAFACE)
		configFileName = "config.facemax";
	else if (face_lm_model.get_modelType() == FaceLandmarkModel::ADSC)
		configFileName = "config.adsc";

	__android_log_write(ANDROID_LOG_ERROR, "HELPER CONFIG FILE NAME", configFileName.c_str());

	is_ConfigValid = loadFiles(configFileName, config_contents); // load the config file

	if (is_ConfigValid)
	{
		if (config_contents.empty()
			|| Helper::config_contents[0][0].compare("dir") != 0
			|| Helper::config_contents[1][0].compare("pose") != 0
			|| Helper::config_contents[2][0].compare("emotion") != 0
			|| Helper::config_contents[3][0].compare("frontalization") != 0
			|| Helper::config_contents[4][0].compare("model") != 0
			)
		{
			is_ConfigValid = false;
            __android_log_write(ANDROID_LOG_ERROR,"HELPER", "Config file couldn't be parsed");
//			LOG(ERROR) << "Config file couldn't be parsed - " << configFileName;
		}
	}
	else{
//		LOG(ERROR) << "Couldn't load config file - " << configFileName;
        __android_log_write(ANDROID_LOG_ERROR, "HELPER", "Couldn't load config file");
    }


	return is_ConfigValid;

}//*/



// - Convert vector (double) to cv::Mat
cv::Mat Helper::convertVecToMat(std::vector<std::vector<double>>& vec_in)
{

	cv::Mat resultMat(vec_in.size(), vec_in[0].size(), CV_64F);

	for (int i = 0; i < vec_in.size(); i++)
	{
		for (int j = 0; j < vec_in[0].size(); j++)
		{
			resultMat.at<double>(i, j) = vec_in[i][j];
		}
	}

	return resultMat;
}

// - Convert vector (cv::Point2d) to cv::Mat
cv::Mat Helper::convertVecToMat(std::vector<cv::Point2d>& vec_in)
{

	cv::Mat resultMat(vec_in.size(), 2, CV_64F);

	for (int i = 0; i < vec_in.size(); i++)
	{
		resultMat.at<double>(i, 0) = (double)vec_in[i].x;
		resultMat.at<double>(i, 1) = (double)vec_in[i].y;
	}

	return resultMat;
}

