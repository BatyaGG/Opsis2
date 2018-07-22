#include "Expression.h"
#include <cmath>
#include <android/log.h>

using namespace FEA;


Expression::~Expression()
{
}


Expression::Expression()
{	
	INIT_STATUS = FEA::RESULT::EXPRESSION_MODULE_NOT_INITIALIZED;
}

// - Clear the weight matrices so that they can be reinitialized
void Expression::clearWeights()
{
	feature_template.clear();
	ypr_weights_mat.release();
	frontalization_mat.release();
	emotion_mat.release();
}


// - Expression calculation (Calculate AVI and YPR from facepoints)
// Perform simple (custom) Procrustes and calculate YPR from the result
// Perform full (custom) Procrustes
// Frontalize the points
// Calculate geometric features
// Calculate expression using geometric features and emotion mat
FEA::RESULT Expression::expression_calculation(cv::Mat& facepoints, double aviresult[3], double ypr[3], double& scale)
{
    std::ostringstream facepointsString;
    facepointsString << facepoints.cols;
    __android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", facepointsString.str().c_str());
	std::vector <cv::Point2d> fpvec;
	for (int i = 0; i <facepoints.cols; i++)
	{
		cv::Point2d pt(facepoints.col(i));
		fpvec.push_back(pt);
	}
	
	// ### YPR Calculation ###
	cv::Mat procrustes_simple_mat;
	custom_procrustes_simple(facepoints, procrustes_simple_mat, scale);
	
//	LOG(TRACE) << "Returned from process_facepoints_simple";
    __android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", "Returned from process_facepoints_simple");
	
	std::vector<cv::Point2d> proc_simple;
	for (int i = 0; i <procrustes_simple_mat.cols; i++)
	{
		cv::Point2d pt(procrustes_simple_mat.col(i));
		proc_simple.push_back(pt);		
	}
	
	// reshape into row matrix
	cv::Mat procrustes_simple_mat_Row(1, 99, CV_64F);
	cv::Mat mat_array[] = { cv::Mat::ones(1, 1, CV_64F), procrustes_simple_mat.row(0), procrustes_simple_mat.row(1) };
	cv::hconcat(mat_array, 3, procrustes_simple_mat_Row);

    std::ostringstream stream1;
    stream1 << "Procurtes row: " << procrustes_simple_mat_Row.rows << " Procurtes cols: " << procrustes_simple_mat_Row.cols << " ypr rows: " << ypr_weights_mat.rows << " ypr cols: " << ypr_weights_mat.cols;
	__android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", stream1.str().c_str());
    cv::Mat  ypr_mat = procrustes_simple_mat_Row * ypr_weights_mat;
	ypr[0] = ypr_mat.at<double>(0);
	ypr[1] = ypr_mat.at<double>(1);
	ypr[2] = ypr_mat.at<double>(2);//*/
	// ######################

	
	cv::Mat facepoints_proc; // MATLAB equivalent : PP
	custom_procrustes(facepoints, procrustes_simple_mat, facepoints_proc);

//	LOG(TRACE) << "Returned from process_facepoints";

	//preparing feature vector
	cv::Mat facepoints_proc_row(1, 99, CV_64F);
	cv::Mat mat_array1[] = { cv::Mat::ones(1, 1, CV_64F), facepoints_proc.row(0), facepoints_proc.row(1) };
	cv::hconcat(mat_array1, 3, facepoints_proc_row);
	
	// Frontalize points
	cv::Mat front1 = facepoints_proc_row * frontalization_mat; // MATLAB equivalent : q
	cv::Mat facepoints_frontalized(2, 49, CV_64F);	
	facepoints_frontalized = front1.reshape(1, 2);
	
	
	cv::Mat Leye, Reye;	
	Leye = ((facepoints_frontalized.col(19) + facepoints_frontalized.col(20) + facepoints_frontalized.col(21) + facepoints_frontalized.col(22) + facepoints_frontalized.col(23) + facepoints_frontalized.col(24)) / 6);

	Reye = ((facepoints_frontalized.col(25) + facepoints_frontalized.col(26) + facepoints_frontalized.col(27) + facepoints_frontalized.col(28) + facepoints_frontalized.col(29) + facepoints_frontalized.col(30)) / 6);
	
	// distance between the eyes
	double e2e = 0.4;
	cv::Mat eye_distance = Reye - Leye;

	double d = sqrt(eye_distance.at<double>(0) * eye_distance.at<double>(0) + eye_distance.at<double>(1) * eye_distance.at<double>(1));
	double e2e_scale = e2e / d;

	cv::Mat fp_x = facepoints_frontalized.row(0);
	fp_x = fp_x * e2e_scale;
	
//	LOG(TRACE) << "Distance between eyes done";
    __android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", "Distance between eyes done");
	
	// geometric features
	cv::Mat geometric(1, feature_template.size() + 1, CV_64F);
	geometric.at<double>(0, 0) = 1;
	double d1;
	for (int i = 0; i < feature_template.size(); i++)
	{
		cv::Point2d pt1(facepoints_frontalized.col(feature_template[i][0] - 1));
		cv::Point2d pt2(facepoints_frontalized.col(feature_template[i][1] - 1));
		cv::Point2d diff = pt1 - pt2;
		d1 = sqrt(diff.x * diff.x + diff.y * diff.y);

		geometric.at<double>(0, i + 1) = d1 / e2e;
	}
	
	cv::Mat final_output = cv::Mat(1, 3, CV_64FC1);

	final_output = geometric * emotion_mat;
	
	aviresult[0] = final_output.at<double>(0, 0);
	aviresult[1] = final_output.at<double>(0, 1);
	aviresult[2] = final_output.at<double>(0, 2);
	
//	LOG(TRACE) << "Final AVI = " << aviresult[0] << "," << aviresult[1] << "," << aviresult[2];
	
	limitAVItoUnitCircle(aviresult);

//	LOG(TRACE) << "AVI limited to unit circle " << aviresult[0] << "," << aviresult[1] << "," << aviresult[2];
    std::ostringstream stream;
    stream << "AVI limited to unit circle " << aviresult[0] << "," << aviresult[1] << "," << aviresult[2];
	__android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", stream.str().c_str());
	return OK;
}


// - Modify AVI values so that they remain within the Unit Circle
void Expression::limitAVItoUnitCircle(double aviresult[3])
{
	double A = aviresult[0];
	double V = aviresult[1];

	double AA, VV;
	double II = sqrt(A*A + V*V);

	if (II > 1)
		II = 1;
	else if (II < 0)
		II = 0;

	double theta = std::abs(atan(A / V));

	AA = sin(theta) * II;
	VV = cos(theta) * II;

	if (A < 0)
		AA = -1 * AA;

	if (V < 0)
		VV = -1 * VV;

	aviresult[0] = AA;
	aviresult[1] = VV;
	aviresult[2] = II;

}


// - Calculate simple (custom) procrustes for headpose
int Expression::custom_procrustes_simple(cv::Mat& facepoints_in, cv::Mat& facepoints_out, double& scale)
{
	// Centroid
	int pts[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 25, 34, 44 };
	cv::Mat nondeform_pts(2, 13, CV_64F);
	for (int k = 0; k < 13; k++)
		facepoints_in.col(pts[k]).copyTo(nondeform_pts.col(k));

	cv::Mat fp_x = nondeform_pts.row(0);
	cv::Mat fp_y = nondeform_pts.row(1);
	cv::Scalar c_x = cv::mean(fp_x);
	cv::Scalar c_y = cv::mean(fp_y);
	cv::Point2d centroid = cv::Point2d(c_x[0], c_y[0]);

	cv::Mat centroid_mat(2, 49, CV_64F);	
	centroid_mat.row(0).setTo(centroid.x);
	centroid_mat.row(1).setTo(centroid.y);
	facepoints_out = facepoints_in - centroid_mat;	// difference between each facepoint and centroid

	//scale
	double temp_sum = 0;
	for (int i = 0; i < 13; i++)
	{
		cv::Point2d fp(facepoints_out.col(pts[i]));
		temp_sum += (fp.x*fp.x + fp.y*fp.y);
	}

	scale = sqrt(temp_sum / 13);
	facepoints_out = facepoints_out * (1 / (5 * scale));

	return OK;
}

// - Calculate (custom) procrustes
int Expression::custom_procrustes(cv::Mat& facepoints, cv::Mat& proc_simple, cv::Mat& facepoints_out)
{
	cv::Mat Leye, Reye;

	Leye = ((proc_simple.col(19) + proc_simple.col(20) + proc_simple.col(21) + proc_simple.col(22) + proc_simple.col(23) + proc_simple.col(24)) / 6);


	Reye = ((proc_simple.col(25) + proc_simple.col(26) + proc_simple.col(27) + proc_simple.col(28) + proc_simple.col(29) + proc_simple.col(30)) / 6);

	// distance between the eyes
	cv::Mat eye_distance = Reye - Leye;

	double rotation_angle;
	if (eye_distance.at<double>(0, 0) != 0)
	{
		double f = eye_distance.at<double>(1, 0) / eye_distance.at<double>(0, 0);
		rotation_angle = atan(f) * 180 / PI;
	}

	//Rotation matrix
	double cos_a = cos(rotation_angle*PI / 180);
	double sin_a = sin(rotation_angle*PI / 180);
	cv::Mat R = cv::Mat(2, 2, CV_64FC1);
	R.at<double>(0, 0) = cos_a;
	R.at<double>(0, 1) = -1 * sin_a;
	R.at<double>(1, 0) = sin_a;
	R.at<double>(1, 1) = cos_a;
		
	facepoints_out = proc_simple.t() * R;
	facepoints_out = facepoints_out.t();
	return OK;
}


// - Names and intensity of expressions
// Intensity of expression: Based on radial distance A-V from origin
// Name of expression: angle of line joining the point A-V and origin (imagine pie slices)
void Expression::get_expression_names(double avi[], std::string& expr_name, std::string& expr_intensity)
{
	std::string expr_list[24] =
	{
		"pleased", "happy", "delighted", "excited", "astonished", "aroused", // first quarter		
		"tensed", "alarmed", "afraid", "annoyed", "distressed", "frustrated", "miserable", //second quarter		
		"sad", "gloomy", "depressed", "bored", "droopy", "tired", "sleepy", // third quarter
		"calm", "serene", "content", "satisfied"  // fourth quarter
	};

	std::string expr_intensity_list[4] = { "Slightly", "Moderately", "Very", "Extremely" };

	double th;
	if (avi[1] == 0)
	{
		if (avi[0] >= 0)
			th = 90;
		else
			th = 270;
	}
	else
	{
		th = atan(avi[0] / avi[1]);

		// process the angle
		th = th * (180 / PI); // convert to degrees

		if (avi[1] < 0)
			th = 180 + th;
		else if (avi[0] < 0)
			th = 360 + th;
	}


	double dist = sqrt((avi[0] * avi[0]) + (avi[1] * avi[1]));

	if (dist < 0.2)
	{
		expr_name = "";
		expr_intensity = "Neutral";
	}
	else
	{


		if (dist < 0.4)
			expr_intensity = expr_intensity_list[0];
		else if (dist < 0.6)
			expr_intensity = expr_intensity_list[1];
		else if (dist < 0.8)
			expr_intensity = expr_intensity_list[2];
		else
			expr_intensity = expr_intensity_list[3];


		// first quarter has 7 emotions => 90 degrees/7 = 13 degrees for each zone. First and last emotions take up half a zone each
		if (th < 16 || th > 354)
			expr_name = expr_list[0];
		else if (th < 34)
			expr_name = expr_list[1];
		else if (th < 62.5)
			expr_name = expr_list[2];
		else if (th < 78.5)
			expr_name = expr_list[3];
		else if (th < 93)
			expr_name = expr_list[4];//---
		else if (th < 104)
			expr_name = expr_list[5];
		else if (th < 115)
			expr_name = expr_list[6];
		else if (th < 126)
			expr_name = expr_list[7]; // 'aroused'. Overlap between first and second quarter
		else if (th < 137)
			expr_name = expr_list[8];
		else if (th < 148)
			expr_name = expr_list[9];
		else if (th < 159)
			expr_name = expr_list[10];
		else if (th < 170)
			expr_name = expr_list[11];
		else if (th < 181)
			expr_name = expr_list[12];
		else if (th < 192)
			expr_name = expr_list[13];
		else if (th < 203)
			expr_name = expr_list[14];
		else if (th < 215)
			expr_name = expr_list[15];
		else if (th < 230)
			expr_name = expr_list[16];
		else if (th < 245)
			expr_name = expr_list[17];
		else if (th < 260)
			expr_name = expr_list[18];
		else if (th < 280)
			expr_name = expr_list[19];
		else if (th < 300)
			expr_name = expr_list[20];
		else if (th < 320)
			expr_name = expr_list[21];
		else if (th < 340)
			expr_name = expr_list[22];
		else if (th < 354)
			expr_name = expr_list[23];
		else
		{
			//fourth quarter left empty for now
			expr_name = "Unknown";
			expr_intensity = "";
		}	
	}
}

// - Initialize expression module
FEA::RESULT Expression::init(FaceLandmarkModel& face_lm_model)
{
//	LOG(INFO) << "Initializing Expression module (" << EXPR_VERSION << ")";
    __android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", "Expression init reached");
	clearWeights();

	Helper::loadConfig(face_lm_model);


	bool loadfiles_ok;

	if (Helper::is_ConfigValid)
		loadfiles_ok = loadWeights();
	else
	{
		INIT_STATUS = FEA::CONFIG_FILE_ERROR;
        __android_log_write(ANDROID_LOG_ERROR, "ESTIMATE EXPRESSION", "CONFIG FILE ERROR");
		return INIT_STATUS;
	}


	if (!loadfiles_ok)
	{
		INIT_STATUS = EXPRESSION_MODULE_NOT_INITIALIZED;
        __android_log_write(ANDROID_LOG_ERROR, "ESTIMATE EXPRESSION", "EXPRESSION MODULE ERROR");
		return INIT_STATUS;
	}
	

	INIT_STATUS = FEA::RESULT::OK;
	return INIT_STATUS;
}

// - Overloaded function to make "scale" optional
FEA::RESULT Expression::estimate_expression(cv::Mat& facepoints, double aviresult[3], double ypr[3], std::string expression_word[2])
{
	double scale = 0;
	return estimate_expression(facepoints, aviresult, ypr, scale, expression_word);
}

// - Estimate the expression
FEA::RESULT Expression::estimate_expression(cv::Mat& facepoints, double aviresult[3], double ypr[3], double& scale, std::string expression_word[2])
{
	if (INIT_STATUS != OK)
		__android_log_write(ANDROID_LOG_ERROR, "ESTIMATE EXPRESSION", "INIT STATUS IS NOT OK");
//		return INIT_STATUS;
	
	cv::Mat facepoints_double;
	facepoints.convertTo(facepoints_double, CV_64F);


    __android_log_write(ANDROID_LOG_ERROR, "EXPRESSION", "REached result calculation");
	FEA::RESULT expr = expression_calculation(facepoints_double, aviresult, ypr, scale);

	//std::cout << "here : " << aviresult[0]  << ", " << aviresult[1]<< std::endl; //debug

	if (expression_word != 0)
		get_expression_names(aviresult, expression_word[1], expression_word[0]);

	return expr;
}


std::string Expression::get_version()
{
	return  EXPR_VERSION;// VER_FILE_VERSION_STR;
}

// - Load weight files
bool Expression::loadWeights()
{
	std::string weight_files_dir, feature_template_f, ypr_weights_f, frontalization_f, emotion_f;

	if (Helper::is_ConfigValid)
	{
		weight_files_dir = Helper::config_contents[0][1];
		ypr_weights_f    = weight_files_dir + "/" + Helper::config_contents[1][1];
		emotion_f        = weight_files_dir + "/" + Helper::config_contents[2][1];
		frontalization_f = weight_files_dir + "/" + Helper::config_contents[3][1];
		
	}
	else
		return false;

	feature_template_f = weight_files_dir + "/" + "feature_template_FULL49.csv";



//	VLOG(2) << "\n# Expression module weights #"
//		<< "\nPose: " << ypr_weights_f
//		<< "\nFrontalization: " << frontalization_f
//		<< "\nEmotion: " << emotion_f;

	std::vector<std::vector<double>> ypr_weights;
	std::vector<std::vector<double>> frontalization;
	std::vector<std::vector<double>> emotion;

	bool loadfile_ok = Helper::loadFiles(frontalization_f, frontalization);

	if (loadfile_ok)
		loadfile_ok = Helper::loadFiles(feature_template_f, feature_template);
	else
		return false;

	if (loadfile_ok)
		loadfile_ok = Helper::loadFiles(emotion_f, emotion);
	else
		return false;


	if (loadfile_ok)
		loadfile_ok = Helper::loadFiles(ypr_weights_f, ypr_weights);
	else
		return false;


	if (!loadfile_ok)
		return false;

	ypr_weights_mat = Helper::convertVecToMat(ypr_weights);
	frontalization_mat = Helper::convertVecToMat(frontalization);
	emotion_mat = Helper::convertVecToMat(emotion);

	return true;

}
