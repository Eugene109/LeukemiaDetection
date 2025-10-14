#pragma once

class Sahi {
	// TODO: write a slicer that runs and parses YOLO object detection across a large wsi
	// 1. read off of a wsi using openslide
	// 2. run multiple yolo detections on slices
	// 3. parse yolo output and recombine slices
	// 4. output global coordinate detections
public:
	YoloModel* cellDetector;
	Sahi(std::wstring modelPath) {
		cellDetector = new YoloModel(modelPath);
		cellDetector->CompileModel();
		cellDetector->inputTensorW;
		cellDetector->inputTensorH;
	}
	~Sahi() {
		delete cellDetector;
	}

	void Run(openslide_t* slide) {}
};