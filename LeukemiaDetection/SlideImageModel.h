#pragma once


#include "openslide-features.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "openslide.h"
#include <WinNls.h>
#include <gdiplus.h>

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
		: seg_w(segmentWidth), seg_h(segmentHeight), currentSeg_x(startX), currentSeg_y(startY) {
		int requiredSize = WideCharToMultiByte(CP_ACP, 0, filename, -1, NULL, 0, NULL, NULL);
		char* c_str = new char[requiredSize];
		memset(c_str, 0, requiredSize);
		WideCharToMultiByte(CP_ACP, 0, filename, -1, c_str, requiredSize, NULL, NULL);

		slide = openslide_open(c_str);

		openslide_get_level0_dimensions(slide, &total_w, &total_h);
		wchar_t buffer[256];
		wsprintf(buffer, L"openslide image dimensions: width=%d, height=%d\n", (int)total_w, (int)total_h);
		OutputDebugStringW(buffer);

		imgBuff = new uint32_t[640 * 640 * 4];
		openslide_read_region(slide, imgBuff, (currentSeg_x)*seg_w / 2, (currentSeg_y) * seg_h / 2, LEVEL, seg_w, seg_h);
		segmentBitmap = new Bitmap((int)seg_w, (int)seg_h, (int)seg_w * 4, PixelFormat32bppARGB, (BYTE*)imgBuff);
	}
	~SlideImageModel() {
		openslide_close(slide);
		delete[] imgBuff;
		delete segmentBitmap;
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