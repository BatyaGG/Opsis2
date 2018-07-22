#ifndef FACE_LANDMARK_MODEL_H
#define FACE_LANDMARK_MODEL_H


#include <string>
#include <vector>
#include <iostream>

//#include "easylogging++.h"

namespace FEA
{
	class FaceLandmarkModel
	{
	public:

		FaceLandmarkModel()
		{
			model_names.push_back("ADSC");
			model_names.push_back("FACEMAX");
			model_names.push_back("INTRAFACE");
		}
		~FaceLandmarkModel(){}

		enum ModelType {
			ADSC = 0,   // to use ADSC's point detector
			FACEMAX,    // to use Facemax point detector
			INTRAFACE   // to use the IntraFace point detector. Same weights as Facemax.
		};


		inline ModelType get_modelType(){ return model; }

		inline std::string get_modelName(){ return model_names[model]; }
		
		void set_modelType(ModelType shm)
		{ 
			model = shm; 
//			LOG(INFO) << "Face Landmark model: " << get_modelName();
		}

	private:
		ModelType model;
		std::vector<std::string> model_names;
	};
}

#endif