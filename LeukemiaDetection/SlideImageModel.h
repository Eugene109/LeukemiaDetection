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

	int level = 0;
	int numLevels;
	void setLevel(int newLvl) {
		level = newLvl;
	}
	int getLevel() {
		return level;
	}
	int getNumLevels() {
		return numLevels;
	}
	int seg_w;
	int seg_h;
	long long total_w; long long total_h;
	SlideImageModel(LPWSTR filename, int segmentWidth = 640, int segmentHeight = 640, int startX = 0, int startY = 0) 
		: seg_w(segmentWidth), seg_h(segmentHeight),
		xPos((startX)*segmentWidth / 2), yPos(startY* segmentHeight / 2), xOff(0), yOff(0) {
		int requiredSize = WideCharToMultiByte(CP_ACP, 0, filename, -1, NULL, 0, NULL, NULL);
		char* c_str = new char[requiredSize];
		memset(c_str, 0, requiredSize);
		WideCharToMultiByte(CP_ACP, 0, filename, -1, c_str, requiredSize, NULL, NULL);

		slide = openslide_open(c_str);

		numLevels = openslide_get_level_count(slide);
		level = numLevels-1;
		openslide_get_level0_dimensions(slide, &total_w, &total_h);
		wchar_t buffer[256];
		wsprintf(buffer, L"openslide image dimensions: levels=%d, width=%d, height=%d\n", numLevels,(int)total_w, (int)total_h);
		OutputDebugStringW(buffer);


		int bestLevel = openslide_get_best_level_for_downsample(slide, 2);
		openslide_get_level0_dimensions(slide, &total_w, &total_h);
		wsprintf(buffer, L"best level for 0.5x: %d\n", bestLevel);
		OutputDebugStringW(buffer);

		imgBuff = new uint32_t[640 * 640 * 9];
		openslide_read_region(slide, imgBuff, xPos-seg_w * 3/2, yPos-seg_h * 3/2, level, seg_w*3, seg_h*3);
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
		int scale = openslide_get_level_downsample(slide, level);

		xPos += xOff* scale;
		yPos += yOff* scale;
		xOff = 0;
		yOff = 0;

		delete segmentBitmap; segmentBitmap = nullptr;
		// xPos and yPos represent the logical center of the current viewport at the given level.
		// A region that is 3× the viewport size (seg_w*3 by seg_h*3), centered on (xPos, yPos), is stored in a bitmap to enable smooth pan animation.
		// openslide API uses coordinates in level-0 coordinate space, to convert:
		//   - multiply the half-extent of the 3×3 region (seg_w * 3 / 2) by the level downsample (scale)
		//   - subtract this from the center (xPos, yPos) to obtain the top-left corner of the requested region.

		openslide_read_region(slide, imgBuff, xPos - (seg_w * 3 / 2) * scale, yPos - (seg_h * 3 / 2) * scale, level, seg_w * 3, seg_h * 3);

		segmentBitmap = new Bitmap((int)seg_w*3, (int)seg_h*3, (int)seg_w * 3 * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
};