#include "PointDetector.h"
#include <android/log.h>
using namespace FEA;

PointDetector::PointDetector()
{
	// Initialize VLFEAT hog
	vlHog = vl_hog_new(VlHogVariant::VlHogVariantUoctti, 8, false);
	vl_hog_set_use_bilinear_orientation_assignments(vlHog, true);

	model_initialized = false;
	
}

PointDetector::~PointDetector()
{
	vl_hog_delete(vlHog);;
}

// - Returns whether the model is initialized successfully
bool PointDetector::get_model_init_status()
{
	return model_initialized;
}

// - Initialize
RESULT PointDetector::init()
{
    __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR", "Init is runned");
	if (Helper::is_ConfigValid)
		return loadModel(Helper::config_contents[4][1]);
	
	return loadModel();
}

// - Load the SDM Model
RESULT PointDetector::loadModel(std::string model_filename)
{
	std::chrono::steady_clock::time_point ss1 = std::chrono::steady_clock::now();	

//	LOG(INFO) << "Initializing Face Point Detector module";
//	VLOG(1) << "Face Point Detector model: " << model_filename;
    std::stringstream temp;
    temp << appFolder << "SDM_model_detection_tracking.yml";
    model_filename = temp.str();
    __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR Module Name", model_filename.c_str());

	cv::FileStorage fs_model;

	fs_model.open(model_filename, cv::FileStorage::READ);
	__android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR Module", "OPENED");
	if (!fs_model.isOpened())
	{
		model_initialized = false;
        __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR", "SDM Model file could not be opened. Exiting");
//		LOG(FATAL) << "SDM Model file could not be opened. Exiting";
		return INVALID_INPUT;
	}

	sdm_model.clear();

	sdm_model.max_iter = (int)fs_model["max_iter"];
	sdm_model.points_n = (int)fs_model["points"];
	sdm_model.layers_n = (int)fs_model["layers"];
	sdm_model.bbox_size.width = (int)fs_model["bbox_w"];
	sdm_model.bbox_size.height = (int)fs_model["bbox_h"];
	sdm_model.bbox_frame = (int)fs_model["bbox_frame"];

	sdm_model.normalized_bbox_size = cv::Size(sdm_model.bbox_size.width + sdm_model.bbox_frame * 2, sdm_model.bbox_size.height + sdm_model.bbox_frame * 2);


	fs_model["desc_size"] >> sdm_model.desc_size;	
	
	fs_model["mean_shape"] >> sdm_model.mean_shape;

	// Detection
	for (int i = 1; i <= sdm_model.layers_n; i++)
	{
		std::stringstream s_stream;
		s_stream << "detectionWeights_layer" << i;

		cv::Mat layer_i;
		fs_model[s_stream.str()] >> layer_i;

		sdm_model.detection_layers.push_back(layer_i);
	}

	// Tracking	
	for (int i = 1; i <= sdm_model.layers_n; i++)
	{
		std::stringstream s_stream;
		s_stream << "trackingWeights_layer" << i;

		cv::Mat layer_i;
		fs_model[s_stream.str()] >> layer_i;

		sdm_model.tracking_layers.push_back(layer_i);
	}


	fs_model["score_weight"] >> sdm_model.score_weight;
	sdm_model.score_winsize = (int)fs_model["score_winsize"];


	std::chrono::steady_clock::time_point ss2 = std::chrono::steady_clock::now();

//	VLOG(2) << "\n# SDM Model Details: #"
//		<< "\nBBox size: " << sdm_model.bbox_size
//		<< "\nNormalized BBox size: " << sdm_model.normalized_bbox_size
//		<< "\nDesc window sizes: " << sdm_model.desc_size
//		<< "\nLayers: " << sdm_model.detection_layers.size()
//		<< "\nLayer Size: " << sdm_model.detection_layers[0].rows << "," << sdm_model.detection_layers[0].cols
//		<< "\nScore winsize=" << sdm_model.score_winsize;
	

//	VLOG(2) << "Time taken to load model: " << std::chrono::duration_cast <std::chrono::milliseconds> (ss2 - ss1).count() << " ms";

	fs_model.release();	

	calcInitialShape();

	model_initialized = sdm_model.is_valid();

    __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR Module Initialized???", "" + model_initialized);
	if (model_initialized)
        __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR Module Initialized???", "SKOREE VSEGO DA");
		return OK;

//	LOG(FATAL) << "Model is not valid. Exiting.";
    __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR Module", "EBANULOS' VSIO");
	return FACE_LANDMARK_MODEL_NOT_INITIALIZED;
}

// - Calculate initial shape
// This is run only once, during initialization
// Obtain the Initial Shape in Normalized BBox space
RESULT PointDetector::calcInitialShape()
{
	if (sdm_model.mean_shape.empty())
		return FACE_LANDMARK_MODEL_NOT_INITIALIZED;

	cv::Rect bbox(cv::Point(0, 0), sdm_model.bbox_size);

	float bbox_center_x = bbox.x + bbox.width / 2.0;
	float bbox_center_y = bbox.y + bbox.height * (1.15 / 2.0);

	float bbox_scale_x = bbox.width * 2.0;
	float bbox_scale_y = bbox.height * 2.0;

	cv::Mat bbox_center_mat, bbox_scale_mat;
	bbox_center_mat = cv::Mat::zeros(2, sdm_model.points_n, CV_32F);
	bbox_scale_mat = cv::Mat::zeros(2, sdm_model.points_n, CV_32F);

	bbox_center_mat.row(0).setTo(bbox_center_x + sdm_model.bbox_frame);
	bbox_center_mat.row(1).setTo(bbox_center_y + sdm_model.bbox_frame);

	bbox_scale_mat.row(0).setTo(bbox_scale_x);
	bbox_scale_mat.row(1).setTo(bbox_scale_y);

	cv::Mat mean_shape_scaled;
	cv::multiply(sdm_model.mean_shape, bbox_scale_mat, mean_shape_scaled);

	// initialize the mean shape
	sdm_model.mean_shape_normalized = mean_shape_scaled + bbox_center_mat;

//	LOG(TRACE) << "Initial shape calculated";

	return OK;
}

// - Extend bounding box
// Extends the bounding box so that the original bounding box corresponds to the Model Bbox, and extended bbox corresponds to model's Normalized BBox
cv::Rect PointDetector::extendBoundingBox(cv::Rect bbox)
{
	// Extend bounding box on all sides
	double extend_percent_w = ((double)sdm_model.bbox_frame / (double)sdm_model.bbox_size.width);
	double extend_percent_h = ((double)sdm_model.bbox_frame / (double)sdm_model.bbox_size.height);
	cv::Rect bbox_extended;
	bbox_extended.x = bbox.x - extend_percent_w * bbox.width;
	bbox_extended.y = bbox.y - extend_percent_h * bbox.height;
	bbox_extended.width = bbox.width * (1 + extend_percent_w*2);
	bbox_extended.height = bbox.height * (1 + extend_percent_h*2);

	return bbox_extended;
}

// - Crop face
// Crop the face from the image according to the given bbox
// Extend the image border as required using reflection
void PointDetector::cropFace(cv::Mat& frame, cv::Mat& frame_cropped, cv::Rect& bbox_extended)
{
	int bottom = 0,
		right = 0,
		left = 0,
		top = 0;

	cv::Rect crop_bbox = bbox_extended;

	if (bbox_extended.x < 0)
	{
		left = abs(bbox_extended.x);
		crop_bbox.x = 0;
	}
	if (bbox_extended.y < 0)
	{
		top = abs(bbox_extended.y);
		crop_bbox.y = 0;
	}
	if (bbox_extended.br().x > frame.cols - 1)
		right = bbox_extended.br().x - frame.cols;
	if (bbox_extended.br().y > frame.rows - 1)
		bottom = bbox_extended.br().y - frame.rows;

	cv::Mat frame_tmp;
	cv::copyMakeBorder(frame, frame_tmp, top, bottom, left, right, cv::BORDER_REFLECT);

	frame_cropped = frame_tmp(crop_bbox);
	
	cv::resize(frame_cropped, frame_cropped, sdm_model.normalized_bbox_size);

}


// - Estimate facepoints
// Starting from Initial Shape Normalized estimate the facepoints and calculate confidence score
// For each layer, calculate HOG features, multiply the features with the layer weights, calculate and apply Procrustes transform between Mean Shape and normalized facepoints
// Since these are just displacements of the points, add them to the original points and continue
int PointDetector::estimateFacepoints(cv::Mat& frame_cropped, cv::Mat& facepoints_normalized, float& score)
{	
	facepoints_normalized = sdm_model.mean_shape_normalized.clone();

	int j = 0;
	for (int j = 0; j < sdm_model.layers_n; j++)
	{

		cv::Mat features(1, 5489, CV_32F);
		calcHogFeatures(frame_cropped, facepoints_normalized, features, 2, sdm_model.desc_size.at<float>(j));

		// score after layer 3
		if (j == sdm_model.layers_n - 1)
		{
			cv::Mat score_mat = features * sdm_model.score_weight;
			score = score_mat.at<float>(0, 0);
			if (score > 1) score = 1;
			if (score < 0) score = 0;
		}

		cv::Mat disp_normSpace(1, 98, CV_32FC1); // displacements in norm space
		
		if (Helper::isTrack)
			disp_normSpace = features *  sdm_model.tracking_layers[j];
		else
			disp_normSpace = features *  sdm_model.detection_layers[j];
		
		cv::Mat disp_normSpace_reshaped(49, 2, CV_32F); // reshaped version
		disp_normSpace_reshaped = disp_normSpace.reshape(1, 2).t();

		Procrustes pr;
		pr.procrustes(facepoints_normalized.t(), sdm_model.mean_shape.t());
		
		cv::Mat disp_normBbox = pr.scale * disp_normSpace_reshaped * pr.rotation; // displacements in norm bbox
				
		facepoints_normalized += disp_normBbox.t();
		
	}	

	// Score	
	/*cv::Mat features(1, 5489, CV_32F);
	calcHogFeatures(frame_cropped, facepoints_normalized, features, 2,sdm_model.score_winsize);
	cv::Mat score_mat = features * sdm_model.score_weight;
	score = score_mat.at<float>(0, 0);

	if (score > 1) score = 1;
	if (score < 0) score = 0;//*/


	return OK;
}

// - Calculate HOG features
// Crop HOG windows for each point and calculate HOG features for each region
// Arrange HOG features in a vector which matches the MATLAB version
int PointDetector::calcHogFeatures(cv::Mat& frame_cropped, cv::Mat& facepoints, cv::Mat& features, int nsb, int winsize)
{

	double cellsize = winsize / nsb;

		
	cv::Mat hogFeature, hogDescriptors;

	for (int i = 0; i < facepoints.cols; i++)
	{
		cv::Point2f landmark_pt(facepoints.at<float>(0, i), facepoints.at<float>(1, i));

		cv::Rect bbox(floor(landmark_pt.x - ((float)winsize - 1.0) / 2.0), floor(landmark_pt.y - ((float)winsize - 1.0) / 2.0), winsize - 1, winsize - 1);

		//std::cout << "\n" << landmark_pt.x << "  KP boxes " << bbox;
		// Sanity checks
		/*if (bbox.x < 0)
			bbox.x = 0;
		else if (bbox.x >= frame_cropped.cols - 1)
			bbox.x = frame_cropped.cols - 2;

		if (bbox.br().x < 1)
			bbox.width = 1;
		else if (bbox.br().x >= frame_cropped.cols)
			bbox.width = frame_cropped.cols - bbox.x - 1;

		if (bbox.y < 0)
			bbox.y = 0;
		else if (bbox.y >= frame_cropped.rows - 1)
			bbox.y = frame_cropped.rows - 2;

		if (bbox.br().y < 1)
			bbox.height = 1;
		else if (bbox.br().y >= frame_cropped.rows)
			bbox.height = frame_cropped.rows - bbox.y - 1;//*/
		
		int x1 = bbox.x;
		int x2 = bbox.x + bbox.width;
		int y1 = bbox.y;
		int y2 = bbox.y + bbox.height;

		if (x1<1)
			x1 = 1;
		else if (x1>frame_cropped.cols - 1)
			x1 = frame_cropped.cols;
		
		if (x2<1)
			x2 = 1;
		else if (x2>frame_cropped.cols-1)
			x2 = frame_cropped.cols;
		

		if (y1<1)
			y1 = 1;
		else if (y1>frame_cropped.rows-1)
			y1 = frame_cropped.rows;
		
		if (y2<1)
			y2 = 1;
		else if (y2>frame_cropped.rows-1)
			y2 = frame_cropped.rows;
		
		--x1; --y1;
		bbox = cv::Rect(x1, y1, x2 - x1, y2 - y1);//*/
		
		cv::Mat frame_cropped2 = frame_cropped(bbox); 
		
		if (frame_cropped2.rows != winsize || frame_cropped2.cols != winsize)
			cv::resize(frame_cropped2, frame_cropped2, cv::Size(winsize, winsize), 0.0, 0.0 , CV_INTER_LINEAR);
		frame_cropped2.convertTo(frame_cropped2, CV_32FC1);

		vl_hog_put_image(vlHog, (float*)frame_cropped2.data, frame_cropped2.cols, frame_cropped2.rows, 1, cellsize);

		int ww = static_cast<int>(vl_hog_get_width(vlHog)); // assert ww == hh == numCells
		int hh = static_cast<int>(vl_hog_get_height(vlHog));
		int dd = static_cast<int>(vl_hog_get_dimension(vlHog)); // assert ww=hogDim1, hh=hogDim2, dd=hogDim3

		cv::Mat hogArray(1, ww*hh*dd, CV_32FC1); // safer & same result. Don't use C-style memory management.

		vl_hog_extract(vlHog, hogArray.ptr<float>(0));
		
		//hogFeature.push_back(hogArray);

		cv::Mat hogDescriptor(hh*ww*dd, 1, CV_32FC1);

		// Stack the third dimensions of the HOG descriptor of this patch one after each other in a column-vector:
		for (int j = 0; j < dd; ++j) 
		{
			cv::Mat hogFeatures(hh, ww, CV_32FC1, hogArray.ptr<float>(0) + j*ww*hh); // Creates the same array as in Matlab. I might have to check this again if hh!=ww (non-square)
			hogFeatures = hogFeatures.t(); // necessary because the Matlab reshape() takes column-wise from the matrix while the OpenCV reshape() takes row-wise.
			hogFeatures = hogFeatures.reshape(0, hh*ww); // make it to a column-vector
			cv::Mat currentDimSubMat = hogDescriptor.rowRange(j*ww*hh, j*ww*hh + ww*hh);
			hogFeatures.copyTo(currentDimSubMat);
		}
		hogDescriptor = hogDescriptor.t(); // now a row-vector		
		hogDescriptors.push_back(hogDescriptor);
	}
	
		// concatenate all the descriptors for this sample vertically (into a row-vector):
	hogDescriptors = hogDescriptors.reshape(0, hogDescriptors.cols * facepoints.cols).t();
	// add a bias row (affine part)
	cv::Mat bias = cv::Mat::ones(1, 1, CV_32FC1);
	cv::hconcat(bias, hogDescriptors, features);

	return OK;
}

// - Preprocessing steps
void PointDetector::preprocess(cv::Mat& input_mat, cv::Mat& output_mat)
{
		cv::cvtColor(input_mat, output_mat, cv::COLOR_BGR2GRAY);
}

// - Core detection
// Extend face bounding box by same amount as Normalized bbox is from bbox
// Crop face using the extended bounding box
// Perform preprocessing (only coversion to grayscale currently)
// Estimate the facepoints, starting from initial shape
// Transform the facepoints from Normalized bbox space to Image space
RESULT PointDetector::detection(cv::Mat& frame, cv::Rect& bbox, cv::Mat& facepoints, float& score)
{
	if (!model_initialized) {
        __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR",
                            "FACE_LANDMARK_MODEL_NOT_INITIALIZED");
        return FACE_LANDMARK_MODEL_NOT_INITIALIZED;
    }
	if (frame.empty() || bbox.area() < 30) {
        __android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR",
                            "INVALID_INPUT");
        return INVALID_INPUT;
    }
	cv::Rect bbox_extended = extendBoundingBox(bbox);

//	LOG(TRACE) << "Extended bounding box " << bbox_extended.x << "," << bbox_extended.y << "," << bbox_extended.width << "," << bbox_extended.height;

	cv::Mat frame_cropped;
	cropFace(frame, frame_cropped, bbox_extended);

//	LOG(TRACE) << "Face crop done";

	cv::Mat frame_cropped_prepocessed;
	preprocess(frame_cropped, frame_cropped_prepocessed);

//	LOG(TRACE) << "Preprocessing done";

	cv::Mat facepoints_normalized;
	estimateFacepoints(frame_cropped_prepocessed, facepoints_normalized, score);

//	LOG(TRACE) << "Estimate facepoints done";

	// Transform to Image Space
	transform_NormalizedBbox_To_Image(facepoints_normalized, facepoints, bbox_extended);
		
//	LOG(TRACE) << "Transform points to image done";

	return OK;
}

// - Detect points
// Set boolean to mark this is detection
// Perform detection()
RESULT PointDetector::detect_points(cv::Mat& frame, cv::Rect& bbox, cv::Mat& facepoints, float& score)
{
	Helper::isTrack = false;
	__android_log_write(ANDROID_LOG_ERROR, "POINT DETECTOR",
						"REACHED DETECT_POINTS");
	return detection(frame, bbox, facepoints, score);
}

// - Do the tracking calculation before detection
// Convert previous facepoints to Normalized bbox space
// Calculate rotation matrix between Initial Shape (vertical) and Normalized Previous Facepoints
// Rotate the image according to the rotation matrix to make the face vertical
// Perform detection()
// Translate facepoints to centroid, rotate the points, (un)translate the points back to original position
RESULT PointDetector::track_points(cv::Mat& frame, cv::Mat& prev_facepoints, cv::Mat& facepoints, float& score)
{	
	if (!model_initialized)
		return FACE_LANDMARK_MODEL_NOT_INITIALIZED;

	if (frame.empty() || prev_facepoints.empty())
		return INVALID_INPUT;

	cv::Rect bbox;
	cv::Point2f centroid; // this will be the center of rotation

	getBoundingBoxFromFacepoints(prev_facepoints, bbox, centroid);
//	LOG(TRACE) << "Bounding box from facepoints estimated";

	cv::Rect bbox_extended = extendBoundingBox(bbox);
	
	cv::Mat centroid_mat = cv::Mat::zeros(2, 49, CV_32F);
	centroid_mat.row(0).setTo(centroid.x);
	centroid_mat.row(1).setTo(centroid.y);
	
	// normalize prev_fp
	cv::Mat prev_facepoints_normalized;
	transform_NormalizedBbox_To_Image(prev_facepoints, prev_facepoints_normalized, bbox_extended, true);

//	LOG(TRACE) << "Transform points from image to Normalized space done";
	
	// find rotation matrix between previous points and initial shape (vertical)
	Procrustes pr;
	pr.procrustes(prev_facepoints_normalized.t(), sdm_model.mean_shape_normalized.t());
	
	cv::Mat inv_rotmat(2, 2, CV_32F);
	inv_rotmat = pr.rotation.clone();
	inv_rotmat.at<float>(0, 1) = -1 * inv_rotmat.at<float>(0, 1);
	inv_rotmat.at<float>(1, 0) = -1 * inv_rotmat.at<float>(1, 0);

	
	cv::Mat rot_mat_warpAffine(2, 3, CV_32F);
	rot_mat_warpAffine.setTo(0);	
	pr.rotation.copyTo(rot_mat_warpAffine.colRange(0, 2));	
	rot_mat_warpAffine.at<float>(0, 2) = ((1 - rot_mat_warpAffine.at<float>(0, 0)) * centroid.x - rot_mat_warpAffine.at<float>(0, 1) * centroid.y);
	rot_mat_warpAffine.at<float>(1, 2) = (rot_mat_warpAffine.at<float>(0, 1) * centroid.x + (1 - rot_mat_warpAffine.at<float>(0, 0)) * centroid.y);//*/
	
	// rotate image to keep face vertical
	cv::Mat frame_rot = frame.clone();
	cv::warpAffine(frame, frame_rot, rot_mat_warpAffine, frame.size());

//	LOG(TRACE) << "Warp affine done on image";
	
	Helper::isTrack = true;
	detection(frame_rot, bbox, facepoints, score);
	
//	LOG(TRACE) << "Detection done";
	
	facepoints = facepoints - centroid_mat; // center facepoints around rotation center
	facepoints = inv_rotmat * facepoints;   // perform rotation
	facepoints = facepoints + centroid_mat; // translate back to original position
	
//	LOG(TRACE) << "Facepoints brought back to image space, rotated translated etc";

	return OK;
}


// - Get bounding box from facepoints
// Estimate a bounding box from the given faceponts which resembles Viola Jones bounding box as closely possible
void PointDetector::getBoundingBoxFromFacepoints(cv::Mat& facepoints, cv::Rect& bbox, cv::Point2f& centroid)
{
	
	// Centroid
	int pts[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 25, 34, 44 };
	cv::Mat nondeform_pts(2, 13, CV_32F);
	for (int k = 0; k < 13; k++)
		facepoints.col(pts[k]).copyTo(nondeform_pts.col(k));

	cv::Mat fp_x = nondeform_pts.row(0);
	cv::Mat fp_y = nondeform_pts.row(1);
	cv::Scalar c_x = cv::mean(fp_x);
	cv::Scalar c_y = cv::mean(fp_y);
	centroid = cv::Point2f(c_x[0], c_y[0]);

	// scale
	double scale = 0.0;
	for (int k = 0; k < nondeform_pts.cols; k++)
	{
		cv::Point2f fp(nondeform_pts.col(k));
		cv::Point2f diff = fp - centroid;
		scale += cv::sqrt(diff.x*diff.x + diff.y*diff.y);
	}
	
	double half_width= scale * 0.32;
	double y_bias = scale  * 0.05; // move top-left y-coordinate up by this much
	
	bbox = cv::Rect(centroid.x - half_width, centroid.y - half_width - y_bias, half_width * 2, half_width * 2);	
}

// - Transformation between Normalized bbox and image
// Perform transformation between Normalized bbox space and image space
// Also perform inverse transform
void PointDetector::transform_NormalizedBbox_To_Image(cv::Mat& src, cv::Mat& dst, cv::Rect& bbox, bool inverseTransform)
{
	cv::Mat bbox_center_mat, bbox_scale_mat;
	bbox_center_mat = cv::Mat::zeros(2, sdm_model.points_n, CV_32F);
	bbox_scale_mat = cv::Mat::zeros(2, sdm_model.points_n, CV_32F);
		
	// Translation
	bbox_center_mat.row(0).setTo(bbox.x);
	bbox_center_mat.row(1).setTo(bbox.y);


	if (inverseTransform)
	{	
		// scale
		bbox_scale_mat.row(0).setTo((float)sdm_model.normalized_bbox_size.width / (float)bbox.width);
		bbox_scale_mat.row(1).setTo((float)sdm_model.normalized_bbox_size.height / (float)bbox.height);

		// translate then scale
		dst = src - bbox_center_mat;
		cv::multiply(dst, bbox_scale_mat, dst);
	}
	else
	{
		// scale
		bbox_scale_mat.row(0).setTo(((float)bbox.width) / sdm_model.normalized_bbox_size.width);
		bbox_scale_mat.row(1).setTo(((float)bbox.height) / sdm_model.normalized_bbox_size.height);

		// scale then translate
		cv::multiply(src, bbox_scale_mat, dst);
		dst = dst + bbox_center_mat;
	}
}
