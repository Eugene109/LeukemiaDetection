#pragma once
#include "YoloModel.h"
#include <openslide.h>

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
		if (cellDetector)  delete cellDetector;

		if (imgBuff)  delete[] imgBuff;
		//if (segmentBitmap)  delete segmentBitmap;
	}
	bool inPolygon(int x, int y) {
		return true;
	}


	char debug[100] = { 0 };
	void printVector(std::vector<yoloDetectionResultOld> arr) {
		for (const yoloDetectionResultOld& r : arr) {
			sprintf_s(debug, "%d,%d %dx%d  class %d\n",(int)r.box.X, (int)r.box.Y, (int)r.box.Width, (int)r.box.Height, r.classId);
			OutputDebugStringA(debug);
		}
	}

	uint32_t* imgBuff = nullptr;
	Bitmap* segmentBitmap = nullptr;
	void Run(openslide_t* slide, int lvl = 0) {
		int startX = 100;
		int startY = 27;
		
		std::vector<yoloDetectionResultOld> globalDetections;
		
		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {
				int gridX = startX + x;
				int gridY = startY + y;
				if (inPolygon(gridX, gridY)) {
					int offsetX = gridX * seg_w / 2;
					int offsetY = gridY * seg_h / 2;
					openslide_read_region(slide, imgBuff, offsetX, offsetY, lvl, seg_w, seg_h);
					delete segmentBitmap;
					segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
					auto sliceDetections = cellDetector->Run(segmentBitmap, offsetX, offsetY);
					printVector(sliceDetections);
					globalDetections.insert(globalDetections.end(), sliceDetections.begin(), sliceDetections.end());
				}
			}
		}
		OutputDebugStringW(L"\nGLOBAL:\n");
		printVector(globalDetections);
	}
	void TestIouCircle() {
		yoloDetectionResult d1 = yoloDetectionResult{ 0,0,32,32, 0.5,1 };
		yoloDetectionResult d2 = yoloDetectionResult{ 0,1,32,32, 0.5,1 };
		float iou = iouCircle(d1, d2);
		sprintf_s(debug, "iou:%f\n", iou);
		OutputDebugStringA(debug);
	}
#include <chrono>
	void TestIouCircleSpeed() {
		yoloDetectionResult d1 = yoloDetectionResult{ 0,0,32,32, 0.5,1 };
		yoloDetectionResult d2 = yoloDetectionResult{ 0,15,10,10, 0.5,1 };
		auto start = std::chrono::high_resolution_clock::now();
		for (int a = 0; a < 1647923*2; a++) {
			float iou = iouCircle(d1, d2);
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = end - start; // Duration in seconds (default for double)

		sprintf_s(debug, "iou time:%fs\n", (float)duration.count());
		OutputDebugStringA(debug);
	}


	//       .
	//      / \
	//     / B  \   
	//  a /       \ c
	//   /          \
	//  / C        A  \
	// -----------------
	//         b  

	//Area = hypotenuse*hypotenuse/4 *sin(2 theta)

#include <cmath>
	float dist(yoloDetectionResult a, yoloDetectionResult b) {
		return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
	}

	const float PI = 3.1415926535;
	float iouCircle(yoloDetectionResult d1, yoloDetectionResult d2) {
		int r1 = (d1.w + d1.h) / 4; // estimated radius of d1 - average of the half-width and half-height
		int r2 = (d2.w + d2.h) / 4;
		float b = dist(d1, d2); // distance from d1 to d2
		if (b > r1 + r2) return 0; // two circles do not touch; 0
		
		if (r2 - r1 >= b) // d1 is enclosed by d2
			return (float)r1 * r1 / r2 * r2;
		if (r1 - r2 >= b) // d2 is enclosed by d1
			return (float)r2 * r2 / r1 * r1;

		if (b < r1 + r2 && r1 < r2 + b && r2 < r1 + b) { // fun little algorithm ( check my math here: https://www.desmos.com/calculator/0iocsnbbjs )
			//OutputDebugStringA("now running circle intersection\n");

			float C = acos((r1 * r1 + b * b - r2 * r2) / (2 * r1 * b));
			float A = acos((r2 * r2 + b * b - r1 * r1) / (2 * r2 * b));
			//sprintf_s(debug, "C:%f, A:%f\n", C,A);
			//OutputDebugStringA(debug);
			float arcArea1 = r1 * r1 * C;
			float arcArea2 = r2 * r2 * A;

			float tri1 = (r1 * r1 / 4) * sin(2 * C); // sometimes there's negative area here it works perfectly don't think too hard
			float tri2 = (r2 * r2 / 4) * sin(2 * A);
			float intersectArea = (arcArea1 + arcArea2) - (tri1 + tri2) * 2;
			float unionArea = (3.1415926535 * (r1 * r1 + r2 * r2)) - intersectArea; // intersect area is double-counted by summing circle areas

			//sprintf_s(debug, "I:%f, U:%f\n", intersectArea, unionArea);
			//OutputDebugStringA(debug);
			return intersectArea / unionArea;
		}
		return iouAABB(d1, d2); // 'default' case to handle wacky edge cases I didn't think of
	}
	int max(int a, int b) {
		if (a > b)
			return a;
		return b;
	}
	int min(int a, int b) {
		if (a < b)
			return a;
		return b;
	}
	float iouAABB(yoloDetectionResult d1, yoloDetectionResult d2) { // simple min/max find intersection of Axis-Aligned Bounding Boxes (AABB)
		int minX1 = d1.x - d1.w / 2;
		int minY1 = d1.y - d1.h / 2;
		int maxX1 = d1.x + d1.w / 2;
		int maxY1 = d1.y + d1.h / 2;

		int minX2 = d2.x - d2.w / 2;
		int minY2 = d2.y - d2.h / 2;
		int maxX2 = d2.x + d2.w / 2;
		int maxY2 = d2.y + d2.h / 2;
		
		if (maxX1 < minX2 || maxX2 < minX1) {
			return 0;
		}
		if (maxY1 < minY2 || maxY2 < minY1) {
			return 0;
		}
		int minXInt = max(minX1, minX2);
		int maxXInt = min(maxX1, maxX2);
		int minYInt = max(minY1, minY2);
		int maxYInt = min(maxY1, maxY2);
		float intersectArea = (maxXInt - minXInt) * (maxYInt - minYInt);
		float unionArea = d1.w * d1.h + d2.w * d2.h - intersectArea;
		return intersectArea / unionArea;
	}
	float iouAABBOld(yoloDetectionResultOld d1, yoloDetectionResultOld d2) {
		
	}
};