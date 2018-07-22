#ifndef _POINTDETECTOR_H_
#define _POINTDETECTOR_H_

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/contrib/contrib.hpp>

#include "Helper.h"
#include "Procrustes.h"


extern "C" {
#include "vl/generic.h"
#include "vl/hog.h"
}

namespace FEA
{
	// Face Point Detector
	class PointDetector {

	public:

		PointDetector();
		~PointDetector();

		// - Initialize
		RESULT init();

		// - Do the tracking calculation before detection
		// Convert previous facepoints to Normalized bbox space
		// Calculate rotation matrix between Initial Shape (vertical) and Normalized Previous Facepoints
		// Rotate the image according to the rotation matrix to make the face vertical
		// Perform detection()
		// Translate facepoints to centroid, rotate the points, (un)translate the points back to original position
		RESULT track_points(cv::Mat& frame, cv::Mat& prev_facepoints, cv::Mat& facepoints, float& score);

		// - Detect points
		// Set boolean to mark this is detection
		// Perform detection()
		RESULT detect_points(cv::Mat& frame, cv::Rect& bbox, cv::Mat& facepoints, float& score);



	private:

		// - Load the SDM Model
		RESULT loadModel(std::string model_filename = "models/SDM_model_detection_tracking.yml");

		// - Core detection
		// Extend face bounding box by same amount as Normalized bbox is from bbox
		// Crop face using the extended bounding box
		// Perform preprocessing (only coversion to grayscale currently)
		// Estimate the facepoints, starting from initial shape
		// Transform the facepoints from Normalized bbox space to Image space
		RESULT detection(cv::Mat& frame, cv::Rect& bbox, cv::Mat& facepoints, float& score);

		// - Calculate initial shape
		// This is run only once, during initialization
		// Obtain the Initial Shape in Normalized BBox space
		RESULT calcInitialShape();

		// - Estimate facepoints
		// Starting from Initial Shape Normalized estimate the facepoints and calculate confidence score
		// For each layer, calculate HOG features, multiply the features with the layer weights, calculate and apply Procrustes transform between Mean Shape and normalized facepoints
		// Since these are just displacements of the points, add them to the original points and continue
		int estimateFacepoints(cv::Mat& frame_cropped, cv::Mat& facepoints_normalized, float& score);

		// - Calculate HOG features
		// Crop HOG windows for each point and calculate HOG features for each region
		// Arrange HOG features in a vector which matches the MATLAB version
		int calcHogFeatures(cv::Mat& frame_cropped, cv::Mat& facepoints, cv::Mat& features, int nsb, int winsize);

		// - Preprocessing steps
		void preprocess(cv::Mat& input_mat, cv::Mat& output_mat);

		// - Extend bounding box
		// Extends the bounding box so that the original bounding box corresponds to the Model Bbox, and extended bbox corresponds to model's Normalized BBox
		cv::Rect extendBoundingBox(cv::Rect bbox);

		// - Crop face
		// Crop the face from the image according to the given bbox
		// Extend the image border as required using reflection
		void cropFace(cv::Mat& frame, cv::Mat& frame_cropped, cv::Rect& bbox_extended);

		// - Get bounding box from facepoints
		// Estimate a bounding box from the given faceponts which resembles Viola Jones bounding box as closely possible
		void getBoundingBoxFromFacepoints(cv::Mat& facepoints, cv::Rect& bbox, cv::Point2f& centroid);

		// - Transformation between Normalized bbox and image
		// Perform transformation between Normalized bbox space and image space
		// Also perform inverse transform
		void transform_NormalizedBbox_To_Image(cv::Mat& src, cv::Mat& dst, cv::Rect& bbox, bool inverseTransform = false);

		// - Returns whether the model is initialized successfully
		inline bool get_model_init_status();


		// - SDM Face Landmark Model
		struct SDM_Model{
			int max_iter;
			int points_n; // number of facepoints
			int layers_n; // number of layers
			cv::Mat mean_shape; // the initial/mean shape
			cv::Mat mean_shape_normalized; // the initial/mean shape in normalized bbox space
			std::vector<cv::Mat> detection_layers, tracking_layers; // detection and tracking layer weight matrices
			cv::Size bbox_size; // Size of bbox
			cv::Size normalized_bbox_size; // size of normalized bbox
			int bbox_frame; // size of frame around bbox to get normalized bbox
			cv::Mat desc_size; // HOG window size of each layer
			cv::Mat score_weight; // score weight matrix
			int score_winsize; // HOG window size of score

			// initialize
			SDM_Model()
			{
				max_iter = 0; points_n = 0; layers_n = 0;
				bbox_frame = 0; score_winsize = 0;
			}

			// clear the model
			void clear()
			{
				mean_shape.release();
				detection_layers.clear();
				tracking_layers.clear();
				desc_size.release();
				score_weight.release();
			}

			// verify model is valid
			inline bool is_valid()
			{
				return !(max_iter == 0 || points_n == 0 || layers_n == 0 || bbox_frame == 0 || score_winsize == 0
					|| mean_shape.empty() || detection_layers.empty() || tracking_layers.empty() || desc_size.empty() || score_weight.empty() || mean_shape_normalized.empty()
					|| bbox_size.area() == 0 || normalized_bbox_size.area() == 0
					);
			}

		}sdm_model;


		VlHog *vlHog;
		bool model_initialized;
	};
}
#endif