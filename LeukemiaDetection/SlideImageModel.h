#pragma once

#include "openslide-features.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "openslide.h"
#include <WinNls.h>
#include <gdiplus.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#ifdef GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#endif
#include <glm/gtc/matrix_inverse.hpp>

using namespace Gdiplus;

class SlideImageModel {
public:
	openslide_t* slide = nullptr;
	uint32_t* imgBuff = nullptr;
	Bitmap* segmentBitmap = nullptr;

	int currentSeg_x;
	int currentSeg_y;
	const static int LEVEL = 0;
	int seg_w;
	int seg_h;
	long long total_w; long long total_h;
	SlideImageModel(LPWSTR filename, int segmentWidth = 640, int segmentHeight = 640, int startX = 0, int startY = 0) 
		: seg_w(segmentWidth), seg_h(segmentHeight), currentSeg_x(startX), currentSeg_y(startY),
		xPos(startX*segmentWidth/2 - 640), yPos(startY*segmentHeight/2 - 640), xOff(640), yOff(640) {
		int requiredSize = WideCharToMultiByte(CP_ACP, 0, filename, -1, NULL, 0, NULL, NULL);
		char* c_str = new char[requiredSize];
		memset(c_str, 0, requiredSize);
		WideCharToMultiByte(CP_ACP, 0, filename, -1, c_str, requiredSize, NULL, NULL);

		slide = openslide_open(c_str);

		openslide_get_level0_dimensions(slide, &total_w, &total_h);
		wchar_t buffer[256];
		wsprintf(buffer, L"openslide image dimensions: width=%d, height=%d\n", (int)total_w, (int)total_h);
		OutputDebugStringW(buffer);

		imgBuff = new uint32_t[640 * 640 * 9];
		openslide_read_region(slide, imgBuff, xPos, yPos, LEVEL, seg_w*3, seg_h*3);
		segmentBitmap = new Bitmap((int)seg_w*3, (int)seg_h*3, (int)seg_w * 3 * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
	~SlideImageModel() {
		openslide_close(slide);
		delete[] imgBuff;
		delete segmentBitmap;
	}

	int xPos;
	int yPos;
	int xOff;
	int yOff;
	char debug[100] = { 0 };
	void move(int dx, int dy) {
		//sprintf_s(debug, "move x:%d, y:%d\n", x, y);
		//OutputDebugStringA(debug);
		xOff += dx;
		yOff += dy;

		//glm::vec2 start = glm::vec2(currentSeg_x*seg_w / 2, currentSeg_y*seg_h / 2);
		//glm::vec2 pos = (/*glm::inverse*/(transform) * glm::vec3(start, 1));
		//delete segmentBitmap; segmentBitmap = nullptr;
		//openslide_read_region(slide, imgBuff, xPos, yPos, LEVEL, seg_w, seg_h);

		//segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
	void reframe() {
		xPos += xOff;
		yPos += yOff;
		xOff = 0;
		yOff = 0;

		delete segmentBitmap; segmentBitmap = nullptr;
		openslide_read_region(slide, imgBuff, xPos, yPos, LEVEL, seg_w*3, seg_h*3);

		segmentBitmap = new Bitmap((int)seg_w*3, (int)seg_h*3, (int)seg_w * 3 * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}

	void setSegment(int x, int y) {
		currentSeg_x = x;
		currentSeg_y = y;

		delete segmentBitmap; segmentBitmap = nullptr;
		openslide_read_region(slide, imgBuff, (currentSeg_x)*seg_w / 2, (currentSeg_y)*seg_h / 2, LEVEL, seg_w, seg_h);

		segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}

	void nextSegment() {
		currentSeg_x++;
		if (currentSeg_x * seg_w > total_w) {
			currentSeg_y++;
			currentSeg_x = 0;
		}

		delete segmentBitmap; segmentBitmap = nullptr;
		openslide_read_region(slide, imgBuff, (currentSeg_x)*seg_w / 2, (currentSeg_y)*seg_h / 2, LEVEL, seg_w, seg_h);

		segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
};