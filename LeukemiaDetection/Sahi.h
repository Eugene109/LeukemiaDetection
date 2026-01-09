#pragma once
#include "YoloModel.h"
#include <openslide.h>
#include <set>
#include <list>
#include <algorithm>

typedef std::vector<yoloDetectionResult>* detListPtr;

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

	uint32_t* imgBuff = nullptr;
	Sahi() {
		cellDetector = new YoloModel();
		cellDetector->CompileModel();
		seg_w = cellDetector->inputTensorW;
		seg_h = cellDetector->inputTensorH;
		imgBuff = new uint32_t[seg_w * seg_h];
	}
	~Sahi() {
		if (cellDetector)  delete cellDetector;

		if (imgBuff)  delete[] imgBuff;
		if (gridDetections)  delete[] gridDetections;
	}
	bool inPolygon(int x, int y) {
		return true;
	}


	char debug[500] = { 0 };
	void printset(std::vector<yoloDetectionResult> arr) {
		for (const yoloDetectionResult& r : arr) {
			sprintf_s(debug, "%d,%d %dx%d  class %d\n",(int)r.x, (int)r.y, (int)r.w, (int)r.h, r.classId);
			OutputDebugStringA(debug);
		}
	}


	void Run(openslide_t* slide, int lvl = 0) {
		cellDetector->StartSession();

		int startX = 100;
		int startY = 27;
		int numFramesX = 6;
		int numFramesY = 6;

		long long slide_w, slide_h;
		openslide_get_level0_dimensions(slide, &slide_w, &slide_h);
		grid_w = slide_w / (seg_w / 2);
		grid_h = slide_h / (seg_h / 2);
		gridDetections = new detListPtr[grid_w * grid_h];
		for (long long a = 0; a < grid_w * grid_h; a++) {
			gridDetections[a] = nullptr;
		}
		
		for (int x = 0; x < numFramesX-1; x++) { // each frame is two grid cells wide
			for (int y = 0; y < numFramesY-1; y++) {
				int gridX = startX + x;
				int gridY = startY + y;
				if (inPolygon(gridX, gridY)) {
					int offsetX = gridX * seg_w / 2;
					int offsetY = gridY * seg_h / 2;
					auto start = std::chrono::high_resolution_clock::now();
					openslide_read_region(slide, imgBuff, offsetX, offsetY, lvl, seg_w, seg_h);
					auto readEnd = std::chrono::high_resolution_clock::now();
					auto sliceDetections = cellDetector->Run(imgBuff, offsetX, offsetY);
					auto detEnd = std::chrono::high_resolution_clock::now();
					cleanBorders(sliceDetections);
					auto cleanEnd = std::chrono::high_resolution_clock::now();
					NMS(sliceDetections);
					auto nmsEnd = std::chrono::high_resolution_clock::now();
					splitDetections(sliceDetections);
					auto splitEnd = std::chrono::high_resolution_clock::now();

					auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(readEnd - start);
					sprintf_s(debug, "Slicing: %f read, %f detect, %f clean, %f nms, %f split\nEndDetections: %d\n",
						(float)std::chrono::duration_cast<std::chrono::microseconds>(readEnd - start).count() / 1000.0f,
						(float)std::chrono::duration_cast<std::chrono::microseconds>(detEnd - readEnd).count() / 1000.0f,
						(float)std::chrono::duration_cast<std::chrono::microseconds>(cleanEnd - detEnd).count() / 1000.0f,
						(float)std::chrono::duration_cast<std::chrono::microseconds>(nmsEnd - cleanEnd).count() / 1000.0f,
						(float)std::chrono::duration_cast<std::chrono::microseconds>(splitEnd - nmsEnd).count() / 1000.0f,
						sliceDetections.size()
						);
					OutputDebugStringA(debug);
				}
			}
		}
		cellDetector->CloseSession();

		for (int x = 0; x < numFramesX; x++) {
			for (int y = 0; y < numFramesY; y++) {
				int gridX = startX + x;
				int gridY = startY + y;
				int gridIndex = gridY * grid_w + gridX;
				if (inPolygon(gridX, gridY)) {
					auto start = std::chrono::high_resolution_clock::now();
					NMS(*gridDetections[gridIndex]);
					auto end = std::chrono::high_resolution_clock::now();

					auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
					sprintf_s(debug, "(%d,%d) nms: %fms detections: %d\n", x,y, elapsed.count(), gridDetections[gridIndex]->size());
					OutputDebugStringA(debug);
				}
			}
		}
	}

	void splitDetections(std::vector<yoloDetectionResult>& detections) {
		for (auto& det : detections) {
			int gridX = det.x / (seg_w / 2);
			int gridY = det.y / (seg_h / 2);
			int gridIndex = gridY * grid_w + gridX;
			if (gridDetections[gridIndex] == nullptr) {
				gridDetections[gridIndex] = new std::vector<yoloDetectionResult>();
			}
			gridDetections[gridIndex]->push_back(det);
			//gridDetections[gridIndex]->insert(det);
		}
	}

	void cleanBorders(std::vector<yoloDetectionResult> &detections) {
		for (auto it = detections.begin(); it != detections.end(); ) {
			if (!withinBorders(*it)) {
				it = detections.erase(it);
			}
			else {
				it++;
			}
		}
	}
	void NMS(std::vector<yoloDetectionResult>& detections) {
		std::sort(detections.begin(), detections.end(), [](yoloDetectionResult a, yoloDetectionResult b) {
			return a.confidence > b.confidence;
			});// sort descending
		for (auto it = detections.begin(); it != detections.end(); ) {
			auto jt = it;
			jt++;
			while (jt != detections.end()) {
				float iou = iouCircle(*it, *jt);
				if (iou > 0.8) {
					jt = detections.erase(jt);
				}
				else {
					jt++;
				}
			}
			it++;
		}
	}

	const int BORDER_THRESH = 10;
	inline bool withinBorders(yoloDetectionResult det) {
		int left = det.x % seg_w - det.w / 2;
		int right = det.x % seg_w + det.w / 2;
		int top = det.y % seg_h - det.h / 2;
		int bottom = det.y % seg_h + det.h / 2;
		return left >= BORDER_THRESH && right < (seg_w - BORDER_THRESH) && top >= BORDER_THRESH && bottom < (seg_h - BORDER_THRESH);
	}


	//             .
	//            / \
	//           / B  \   
	//        a /       \ c
	//         /          \
	//        / C        A  \
	//  (d1) ^---------------^ (d2)
	//               b

	//Area = hypotenuse*hypotenuse/4 *sin(2 theta)

#include <cmath>
	float dist(yoloDetectionResult a, yoloDetectionResult b) {
		return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
	}

	const float PI = 3.1415926535;
	float iouCircle(yoloDetectionResult d1, yoloDetectionResult d2) {
		float AABB_iou = iouAABB(d1, d2); // quick check to see if AABBs overlap at all
		if (AABB_iou == 0)
			return 0;

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
		int minXInt = std::max(minX1, minX2);
		int maxXInt = std::min(maxX1, maxX2);
		int minYInt = std::max(minY1, minY2);
		int maxYInt = std::min(maxY1, maxY2);
		float intersectArea = (maxXInt - minXInt) * (maxYInt - minYInt);
		float unionArea = d1.w * d1.h + d2.w * d2.h - intersectArea;
		return intersectArea / unionArea;
	}
	float iouAABBOld(yoloDetectionResultOld d1, yoloDetectionResultOld d2) {
		
	}
};