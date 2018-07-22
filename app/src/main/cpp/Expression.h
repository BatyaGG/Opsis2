#ifndef Expression_h
#define Expression_h

#include <string.h>
#include <iostream>
#include <fstream>
//#include <Windows.h>

#include <opencv2/core/core.hpp>

#include "Helper.h"
#include "FaceLandmarkModel.h"
#include "Macros.h"

// Not important. Maintaining only as legacy code.
// v0.5: Logging and documentation added
#define EXPR_VERSION "v0.5"

namespace FEA
{
	// Expression Module
	class Expression
	{
	public:
		Expression();
		~Expression();

		// - Initialize the expression module
		RESULT init(FaceLandmarkModel& shape_model);
		RESULT INIT_STATUS;		


		//  - Estimate the expression and head pose</para>
		//
		// @param facepoints(in) facepoints (49 points currently)
		// @param avi (out) the arousal, valence and intensity values
		// @param ypr (out) the yaw, pitch and roll values
		// @param scale (out) the scale of the face
		// @param expression_word (out) the expression name and intensity
		RESULT estimate_expression(cv::Mat& facepoints, double avi[3], double ypr[3], std::string expression_word[2] = 0);
		RESULT estimate_expression(cv::Mat& facepoints, double avi[3], double ypr[3], double &scale, std::string expression_word[2] = 0);

		// - Return the version of the expression module
		std::string get_version();


	private:


		// - Calculate (custom) procrustes
		int custom_procrustes(cv::Mat& facepoints_in, cv::Mat& facepoints_simple, cv::Mat& facepoints_out);


		// - Calculate simple (custom) procrustes for headpose
		int custom_procrustes_simple(cv::Mat& facepoints_in, cv::Mat& facepoints_out, double& scale);
			

		// - Modify AVI values so that they remain within the Unit Circle
		void limitAVItoUnitCircle(double aviresult[3]);

		
		
		// - Load weight files
		bool loadWeights();
		void clearWeights();

		// - Weight files				
		std::vector<std::vector<double>> feature_template;
		cv::Mat ypr_weights_mat;
		cv::Mat frontalization_mat;
		cv::Mat emotion_mat;


		// - Expression calculation (Calculate AVI and YPR from facepoints)
		// Perform simple (custom) Procrustes and calculate YPR from the result
		// Perform full (custom) Procrustes
		// Frontalize the points
		// Calculate geometric features
		// Calculate expression using geometric features and emotion mat
		FEA::RESULT expression_calculation(cv::Mat& facepoints, double aviresult[3], double ypr[3], double& scale);

		// - Names and intensity of expressions
		// Intensity of expression: Based on radial distance A-V from origin
		// Name of expression: angle of line joining the point A-V and origin (imagine pie slices)
		void get_expression_names(double avi[3], std::string& expr_name, std::string& expr_intensity);
	};
}

#endif