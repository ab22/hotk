#include "screen.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <fstream>

#include <zlib.h>
#include <png.h>

namespace screen = hotk::graphics::screen;

using hotk::graphics::screen_capture::HDCPtr;
using hotk::graphics::screen_capture::CompatibleDCPtr;
using hotk::graphics::screen_capture::HBITMAPPtr;
using hotk::graphics::screen_capture::ScreenCapture;

using hotk::winutils::errors::Win32Error;

std::unique_ptr<ScreenCapture> screen::capture_full_screen()
{
	// Create device contexts.
	auto hdc = HDCPtr(GetDC(nullptr));
	if (hdc.get() == nullptr)
		throw Win32Error(GetLastError(), "capture full screen: GetDC failed");

	auto hDest = CompatibleDCPtr(CreateCompatibleDC(hdc.get()));
	if (hDest.get() == nullptr)
		throw Win32Error(GetLastError(), "capture full screen: CreateCompatibleDC failed");

	// Get screen dimensions.
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (width == 0)
		throw Win32Error(GetLastError(), "capture full screen: failed to get screen width");

	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (height == 0)
		throw Win32Error(GetLastError(), "capture full screen: failed to get screen height");

	// Create the Bitmap.
	auto hbitmap = HBITMAPPtr(CreateCompatibleBitmap(hdc.get(), width, height));
	if (hbitmap.get() == nullptr)
		throw Win32Error(GetLastError(), "capture full screen: CreateCompatibleBitmap failed");

	// Use the previously created device context with the bitmap.
	auto result = SelectObject(hDest.get(), hbitmap.get());
	if (result == NULL || result == HGDI_ERROR)
		throw Win32Error(GetLastError(), "capture full screen: SelectObject failed");

	// Copy from the desktop device context to the bitmap device context
	// call this once per 'frame'.
	auto bitblt_result = BitBlt(hDest.get(), 0, 0, width, height, hdc.get(), 0, 0, SRCCOPY);
	if (bitblt_result == NULL)
		throw Win32Error(GetLastError(), "capture full screen: BitBlt failed");

	return std::make_unique<ScreenCapture>(std::move(hdc), std::move(hbitmap));
}