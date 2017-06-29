#include "graphics.h"

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

using namespace Gdiplus;
using namespace hotk;
using namespace hotk::winutils;

inline void graphics::Graphics::build_device_contexts()
{
	using hdc_ptr = std::unique_ptr<HDC__, deleters::HDCDeleter>;
	hdc = hdc_ptr(GetDC(nullptr));

	if (hdc.get() == nullptr)
		throw graphics::GraphicsInitError(GetLastError(), "graphics ctor: GetDC failed");

	using hdest_ptr = std::unique_ptr<HDC__, deleters::CompatibleDCDeleter>;
	hDest = hdest_ptr(CreateCompatibleDC(hdc.get()));

	if (hDest.get() == nullptr)
		throw graphics::GraphicsInitError(GetLastError(), "graphics ctor: CreateCompatibleDC failed");
}

graphics::Graphics::Graphics()
	: hdc(nullptr),
	  hDest(nullptr)
{
	this->build_device_contexts();
}

std::unique_ptr<HBITMAP__, deleters::HBitmapDeleter> graphics::Graphics::capture_screen() const
{
	// Get screen dimensions.
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (width == 0)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: failed to get screen width");

	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (height == 0)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: failed to get screen height");

	// Create the Bitmap.
	HBITMAP handler = CreateCompatibleBitmap(hdc.get(), width, height);
	auto bitmap_handler = std::unique_ptr<HBITMAP__, deleters::HBitmapDeleter>(handler);
	if (handler == nullptr)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: CreateCompatibleBitmap failed");

	// Use the previously created device context with the bitmap.
	auto result = SelectObject(hDest.get(), handler);
	if (result == NULL || result == HGDI_ERROR)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: SelectObject failed");

	// Copy from the desktop device context to the bitmap device context
	// call this once per 'frame'.
	auto bb_result = BitBlt(hDest.get(), 0, 0, width, height, hdc.get(), 0, 0, SRCCOPY);
	if (bb_result == NULL)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: BitBlt failed");

	return bitmap_handler;
}

std::unique_ptr<BITMAPINFO> graphics::Graphics::create_bitmap_info(HBITMAP hbitmap)
{
	BITMAP bmp;
	auto   result = GetObject(hbitmap, sizeof(BITMAP), &bmp);

	if (result == 0)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: could not GetObject");

	auto bitmap_info = std::make_unique<BITMAPINFO>();
	bitmap_info->bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
	bitmap_info->bmiHeader.biWidth        = bmp.bmWidth;
	bitmap_info->bmiHeader.biHeight       = bmp.bmHeight;
	bitmap_info->bmiHeader.biPlanes       = bmp.bmPlanes;
	bitmap_info->bmiHeader.biBitCount     = bmp.bmBitsPixel;
	bitmap_info->bmiHeader.biCompression  = BI_RGB;
	bitmap_info->bmiHeader.biSizeImage    = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4 * bmp.bmHeight;
	bitmap_info->bmiHeader.biClrImportant = 0;

	return bitmap_info;
}

std::vector<byte> graphics::Graphics::to_vector(HBITMAP hbitmap)
{
	auto bmi = create_bitmap_info(hbitmap);

	BITMAPFILEHEADER bitmap_header;
	bitmap_header.bfType      = 0x4D42;
	bitmap_header.bfSize      = sizeof(BITMAPFILEHEADER) + bmi->bmiHeader.biSize + bmi->bmiHeader.biSizeImage;
	bitmap_header.bfReserved1 = 0;
	bitmap_header.bfReserved2 = 0;
	bitmap_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + bmi->bmiHeader.biSize;

	std::vector<byte> bmp;
	bmp.reserve(bitmap_header.bfSize);

	std::copy(
		(byte*)&bitmap_header,
		(byte*)&bitmap_header + sizeof(BITMAPFILEHEADER),
		std::back_inserter(bmp)
	);
	std::copy(
		(byte*)&bmi->bmiHeader,
		(byte*)&bmi->bmiHeader + sizeof(BITMAPINFOHEADER),
		std::back_inserter(bmp)
	);

	bmp.resize(bitmap_header.bfSize);
	auto result = GetDIBits(
		hdc.get(),
		hbitmap,
		0,
		bmi->bmiHeader.biHeight,
		bmp.data() + bitmap_header.bfOffBits,
		bmi.get(),
		DIB_RGB_COLORS
	);

	if (result == 0)
		throw graphics::CaptureScreenError(GetLastError(), "capture screen error: could not GetDIBits");

	return bmp;
}