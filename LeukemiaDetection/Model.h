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
	Sahi* sahiModel;
	
	void InitSlideImg(LPWSTR filename) {
		slideImg = new SlideImageModel(filename, 640,640,52,18);
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

	void InitCellDetector() {
		sahiModel = new Sahi();
		//sahiModel->cellDetector->StartSession();
	}
	void CompileCellDetector() {
		sahiModel->cellDetector->CompileModel();
	}
	void StartOrtSession(){
		sahiModel->cellDetector->StartSession();
	}

public:
	uint32_t* imgBuff;
	Bitmap* segmentBitmap;
	void RunCellDetector() {
		cellDetector->StartSession();
		imgBuff = new uint32_t[640 * 640];
		openslide_read_region(slideImg->slide, imgBuff, slideImg->xPos, slideImg->yPos, 0, slideImg->seg_w, slideImg->seg_h);
		segmentBitmap = new Bitmap((int)slideImg->seg_w, (int)slideImg->seg_h, (int)slideImg->seg_w* 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
		cellResults = cellDetector->Run(imgBuff);
		sahiModel->cleanBorders(cellResults);
		sahiModel->NMS(cellResults);

		/*auto start = std::chrono::steady_clock::now();
		sahiModel->NMS(cellResults);
		auto end = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);*/
		//sprintf_s(sahiModel->debug, "Detections after NMS: %d, %dms\n", (int)cellResults.size(), (int)elapsed.count());
		//OutputDebugStringA(sahiModel->debug);
	}
	void RunSlicedDetection() {
		sahiModel->Run(slideImg->slide, 0);
	}
public:
	friend class Controller;
	Model() {
		cellDetector = nullptr;
		slideImg = nullptr;
		cellResults = std::vector<yoloDetectionResult>();
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
	std::vector<yoloDetectionResult> getCellResults() {
		return cellResults;
	}
};