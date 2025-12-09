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
	std::vector<yoloDetectionResultOld> cellResults;
	Sahi* sahiModel;
	
	void InitSlideImg(LPWSTR filename) {
		slideImg = new SlideImageModel(filename, 640,640,100,27);
	}
	void SetImageSegment(int x, int y) {
		if (slideImg)
			slideImg->setSegment(x, y);
	}
	void NextImageSegment() {
		if (slideImg)
			slideImg->nextSegment();
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
		OutputDebugStringW(filename);
		cellDetector = new YoloModel(filename);
		sahiModel = new Sahi(filename);
	}
	void CompileCellDetector() {
		cellDetector->CompileModel();
	}
	void RunCellDetector() {
		//cellResults = cellDetector->Run(slideImg->segmentBitmap);
		//sahiModel->Run(slideImg->slide, 0);
		sahiModel->TestIouCirlceSpeed();
	}
public:
	friend class Controller;
	Model() {
		cellDetector = nullptr;
		slideImg = nullptr;
		cellResults = std::vector<yoloDetectionResultOld>();
	}
	~Model() {
		if (cellDetector) {
			delete cellDetector;
			cellDetector = nullptr;
		}
		if (slideImg) {
			delete slideImg;
			slideImg = nullptr;
		}
		if (sahiModel) {
			delete sahiModel;
			sahiModel = nullptr;
		}
	}
	YoloModel* getCellDetector() {
		return cellDetector;
	}
	SlideImageModel* getSlideImg() {
		return slideImg;
	}
	std::vector<yoloDetectionResultOld> getCellResults() {
		return cellResults;
	}
};