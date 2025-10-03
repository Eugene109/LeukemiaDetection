#pragma once

class Sahi {
	// TODO: write a slicer that runs and parses YOLO object detection across a large wsi
	// 1. read off of a wsi using openslide
	// 2. run multiple yolo detections on slices
	// 3. parse yolo output and recombine slices
	// 4. output global coordinate detections
public:
	Sahi() {}
	~Sahi() {}
};