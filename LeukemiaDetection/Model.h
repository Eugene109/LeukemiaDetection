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
		slideImg = new SlideImageModel(filename, 640,640,50,17);
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

public:
	uint32_t* imgBuff;
	Bitmap* segmentBitmap;
	void RunCellDetector() {
		imgBuff = new uint32_t[640 * 640 ];
		openslide_read_region(slideImg->slide, imgBuff, slideImg->xPos, slideImg->yPos, 0, slideImg->seg_w, slideImg->seg_h);
		segmentBitmap = new Bitmap((int)slideImg->seg_w, (int)slideImg->seg_h, (int)slideImg->seg_w* 4, PixelFormat32bppARGB, (BYTE*)imgBuff);

		cellResults = cellDetector->Run(segmentBitmap);
		//sahiModel->Run(slideImg->slide, 0);
		sahiModel->TestIouCircle();
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