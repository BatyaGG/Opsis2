#ifndef FacialExpressionAnalysis_H_
#define FacialExpressionAnalysis_H_


#include "Expression.h"
#include "PointDetector.h"
#include "Licence.h"
#include "Helper.h"

using namespace FEA;

class FacialExpressionAnalysis
{
public:
	FacialExpressionAnalysis(){}
	~FacialExpressionAnalysis(){}

		
	FEA::RESULT initialize(int VERBOSE_LEVEL_ = 1, std::string folder = " ");
	FEA::RESULT calc_expression(cv::Mat& img, cv::Rect& bbox, bool isTrack, double avi[], double ypr[], std::string expr_word[], float& confidence_score, cv::Mat& facepoints);

private:
	FEA::PointDetector pd;
	FEA::Expression expr;
	FEA::Licence lic;
	cv::CascadeClassifier face_cascade;

	const float confidence_score_threshold = 0.5;
	const int min_face_size = 50;

	// previous facepoints and score
	struct prev
	{
		cv::Mat facepoints;
		float score;

		prev()
		{
			score = 0;
		}
	}prev;
};

#endif