#pragma once

#include "SlideImageModel.h"
#include "OnnxModel.h"
#include "VisionModel.h"
#include "YoloModel.h"
#include "Sahi.h"

class Controller;
class Model {
protected:
	SlideImageModel* slideImg;
	YoloModel* cellDetector;
	std::vector<yoloDetectionResult> cellResults;
	
	void InitSlideImg(LPWSTR filename) {
		slideImg = new SlideImageModel(filename, 640,640,2,2);
	}
	void setSlideLevel(int level) {
		if (slideImg)
			slideImg->setLevel(level);
	}

	void MoveSlide(int dx, int dy) {
		if (slideImg)
			slideImg->move(dx, dy);
	}
	void Reframe() {
		if (slideImg)
			slideImg->reframe();
	}

	void InitCellDetector(LPWSTR filename) {
		cellDetector = new YoloModel(filename);
	}
	void CompileCellDetector() {
		cellDetector->CompileModel();
	}
	void RunCellDetector() {
		cellResults = cellDetector->Run(slideImg->segmentBitmap);
	}
public:
	friend class Controller;
	Model() {
		cellDetector = nullptr;
		slideImg = nullptr;
		cellResults = std::vector<yoloDetectionResult>();
	}
	YoloModel* getCellDetector() {
		return cellDetector;
	}
	SlideImageModel* getSlideImg() {
		return slideImg;
	}
	std::vector<yoloDetectionResult> getCellResults() {
		return cellResults;
	}
};