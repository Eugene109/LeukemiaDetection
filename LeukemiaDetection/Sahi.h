#pragma once

class Sahi {
	// TODO: write a slicer that runs and parses YOLO object detection across a large wsi
	// 1. read off of a wsi using openslide
	// 2. run multiple yolo detections on slices
	// 3. parse yolo output and recombine slices
	// 4. output global coordinate detections
public:
	YoloModel* cellDetector;
	int seg_w;
	int seg_h;
	Sahi(std::wstring modelPath) {
		cellDetector = new YoloModel(modelPath);
		cellDetector->CompileModel();
		seg_w = cellDetector->inputTensorW;
		seg_h = cellDetector->inputTensorH;
		imgBuff = new uint32_t[seg_w * seg_h];
		segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
	~Sahi() {
		delete cellDetector;

		delete[] imgBuff;
		delete segmentBitmap;
	}
	bool inPolygon(int x, int y) {
		return true;
	}

	uint32_t* imgBuff = nullptr;
	Bitmap* segmentBitmap = nullptr;
	void Run(openslide_t* slide, int lvl = 0) {
		int startX = 100;
		int startY = 27;
		
		std::vector<yoloDetectionResult> globalDetections;
		
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				int gridX = startX + x;
				int gridY = startY + y;
				if (inPolygon(gridX, gridY)) {
					int offsetX = gridX * seg_w / 2;
					int offsetY = gridY * seg_h / 2;
					openslide_read_region(slide, imgBuff, offsetX, offsetY, lvl, seg_w, seg_h);
					delete segmentBitmap;
					segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
					auto sliceDetections = cellDetector->Run(segmentBitmap, offsetX, offsetY);
					globalDetections.insert(globalDetections.end(), sliceDetections.begin(), sliceDetections.end());
				}
			}
		}
	}
};