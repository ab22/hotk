#include "graphics.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <fstream>

#include <zlib.h>
#include <png.h>

using namespace hotk::graphics;
using hotk::winutils::errors::Win32Error;

inline void Graphics::build_device_contexts()
{
	hdc = std::unique_ptr<HDC__, HDCDeleter>(GetDC(nullptr));
	if (hdc.get() == nullptr)
		throw Win32Error(GetLastError(), "graphics ctor: GetDC failed");

	hDest = std::unique_ptr<HDC__, CompatibleDCDeleter>(CreateCompatibleDC(hdc.get()));
	if (hDest.get() == nullptr)
		throw Win32Error(GetLastError(), "graphics ctor: CreateCompatibleDC failed");
}

Graphics::Graphics()
	: hdc(nullptr)
	, hDest(nullptr)
{
	this->build_device_contexts();
}

std::unique_ptr<HBITMAP__, HBitmapDeleter> Graphics::capture_screen() const
{
	// Get screen dimensions.
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (width == 0)
		throw Win32Error(GetLastError(), "capture screen: failed to get screen width");

	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (height == 0)
		throw Win32Error(GetLastError(), "capture screen: failed to get screen height");

	// Create the Bitmap.
	HBITMAP handler = CreateCompatibleBitmap(hdc.get(), width, height);
	auto bitmap_handler = std::unique_ptr<HBITMAP__, HBitmapDeleter>(handler);
	if (handler == nullptr)
		throw Win32Error(GetLastError(), "capture screen: CreateCompatibleBitmap failed");

	// Use the previously created device context with the bitmap.
	auto result = SelectObject(hDest.get(), handler);
	if (result == NULL || result == HGDI_ERROR)
		throw Win32Error(GetLastError(), "capture screen: SelectObject failed");

	// Copy from the desktop device context to the bitmap device context
	// call this once per 'frame'.
	auto bb_result = BitBlt(hDest.get(), 0, 0, width, height, hdc.get(), 0, 0, SRCCOPY);
	if (bb_result == NULL)
		throw Win32Error(GetLastError(), "capture screen: BitBlt failed");

	return bitmap_handler;
}

std::unique_ptr<BITMAPINFO> Graphics::create_bitmap_info(const HBITMAP hbitmap) const
{
	BITMAP bmp;
	auto result = GetObject(hbitmap, sizeof(BITMAP), &bmp);

	if (result == 0)
		throw Win32Error(GetLastError(), "create bitmap info: could not GetObject");

	auto bitmap_info                      = std::make_unique<BITMAPINFO>();
	bitmap_info->bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
	bitmap_info->bmiHeader.biWidth        = bmp.bmWidth;
	bitmap_info->bmiHeader.biHeight       = bmp.bmHeight;
	bitmap_info->bmiHeader.biPlanes       = bmp.bmPlanes;
	bitmap_info->bmiHeader.biBitCount     = bmp.bmBitsPixel;
	bitmap_info->bmiHeader.biCompression  = BI_RGB;
	// bitmap_info->bmiHeader.biSizeImage    = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4 * bmp.bmHeight;
	bitmap_info->bmiHeader.biSizeImage = bmp.bmWidth * bmp.bmHeight * 4;
	bitmap_info->bmiHeader.biClrImportant = 0;

	return bitmap_info;
}

std::vector<std::byte> Graphics::to_vector(const HBITMAP hbitmap) const
{
	auto bmi = create_bitmap_info(hbitmap);

	BITMAPFILEHEADER bitmap_header;
	bitmap_header.bfType      = 0x4D42;
	bitmap_header.bfSize      = sizeof(BITMAPFILEHEADER) + bmi->bmiHeader.biSize + bmi->bmiHeader.biSizeImage;
	bitmap_header.bfReserved1 = 0;
	bitmap_header.bfReserved2 = 0;
	bitmap_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + bmi->bmiHeader.biSize;

	std::vector<std::byte> bmp;

	// Allocate enough memory to hold the bitmap header + it's data.
	// When performing a std::copy with std::back_inserter, all elements
	// copied will get push_back() to the end and the vector's size will
	// increment by itself.
	bmp.reserve(bitmap_header.bfSize);
	std::copy(
		reinterpret_cast<std::byte*>(&bitmap_header),
		reinterpret_cast<std::byte*>(&bitmap_header) + sizeof(BITMAPFILEHEADER),
		std::back_inserter(bmp));
	std::copy(
		reinterpret_cast<std::byte*>(&bmi->bmiHeader),
		reinterpret_cast<std::byte*>(&bmi->bmiHeader) + sizeof(BITMAPINFOHEADER),
		std::back_inserter(bmp));

	// Unlike std::copy, GetDIBits expects a LPVOID to where the data should
	// be stored. In this case, GetDIBits will not push_back() so the
	// vector will never know that it's size changed, so we perform a
	// resize() on the vector to tell it the new total size of it.
	bmp.resize(bitmap_header.bfSize);
	auto result = GetDIBits(
		hdc.get(),
		hbitmap,
		0,
		bmi->bmiHeader.biHeight,
		bmp.data() + bitmap_header.bfOffBits,
		bmi.get(),
		DIB_RGB_COLORS);

	if (result == 0)
		throw Win32Error(GetLastError(), "capture screen error: could not GetDIBits");

	return bmp;
}

void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
	if (png_ptr == NULL) {
		std::cout << "WRITE_CALLBACK: png_ptr is null\n";
		return;
	}

	std::cout << "--------------------------------\n";
	std::cout << "row: " << row << "\n";
	std::cout << "pass: " << pass << "\n";
}

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	std::cout << "user error fn: " << error_msg << "\n";
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	std::cout << "user warning fn: " << warning_msg << "\n";
}

void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	if (png_ptr == NULL)
		return;

	std::ofstream* output = reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr));

	if (!output)
		throw std::exception("No ofstream found when writing!");

	size_t before = output->tellp();
	output->write(reinterpret_cast<char*>(data), length);
	size_t bytes_written = static_cast<size_t>(output->tellp()) - before;

	if (bytes_written != length)
		throw std::exception("Failed to write all bytes to file");
}

void user_flush_data(png_structp png_ptr)
{
	if (png_ptr == NULL)
		return;

	std::ofstream* output = reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr));

	if (!output)
		throw std::exception("No ofstream found when flushing!");

	output->flush();
}

void Graphics::to_png(const HBITMAP hbitmap) const 
{
	auto bmi = create_bitmap_info(hbitmap);
	auto data = to_vector(hbitmap);

	// FILE *fp = NULL;
	// fopen_s(&fp, "test.png", "wb"); 

	// if (fp == NULL) {
	//		throw std::exception("Could not open file test.png!");
	// }

	std::ofstream output("out.png", std::ios::binary);

	if (!output) 
		throw std::exception("Could not open file out.png!");
	
	png_voidp user_error_ptr = NULL;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, user_error_ptr, user_error_fn, user_warning_fn);

	if (!png_ptr)
		throw std::exception("Could not create png write struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		throw std::exception("Could not create png info struct");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		throw std::exception("Error creating jump buf");
	}

	// png_init_io(png_ptr, fp);
	png_set_write_fn(png_ptr, reinterpret_cast<void*>(&output), user_write_data, user_flush_data);
	png_set_write_status_fn(png_ptr, write_row_callback);

	png_set_IHDR(
		png_ptr,
		info_ptr,
		bmi->bmiHeader.biWidth,
		bmi->bmiHeader.biHeight,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	constexpr size_t bitmap_header_offset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	auto rows = std::vector<png_byte *>();
	png_byte* row_ptr = reinterpret_cast<png_byte*>(data.data() + bitmap_header_offset);
	rows.reserve(bmi->bmiHeader.biHeight);

	std::cout << "Bitmap starts at: " << static_cast<void*>(data.data()) << "\n";
	std::cout << "Bitmap rows should start at: " << static_cast<void*>(row_ptr) << "\n";

	for (LONG i = 0; i < bmi->bmiHeader.biHeight; i++) {
		rows.insert(rows.begin(), row_ptr);

		row_ptr += bmi->bmiHeader.biWidth * 4;
	}

	std::cout << "Rows calculated: " << rows.size() << "\n";

	png_set_rows(png_ptr, info_ptr, rows.data());
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);
	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	//fclose(fp);
}
