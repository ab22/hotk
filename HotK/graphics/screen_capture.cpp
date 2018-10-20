#include "screen_capture.h"

#include <cassert>
#include <zlib.h>
#include <png.h>
#include <fstream>
#include <iostream>

using namespace hotk::graphics::screen_capture;

using hotk::winutils::errors::Win32Error;

ScreenCapture::ScreenCapture(HDCPtr hdc, HBITMAPPtr hbitmap_ptr)
	: hdc(std::move(hdc))
	, hbitmap_ptr(std::move(hbitmap_ptr))
{
	assert(!hdc || !hbitmap_ptr);

	fill_bitmap_headers();
}

void ScreenCapture::fill_bitmap_headers()
{
	fill_bitmap_info(hbitmap_ptr.get());
	fill_bitmap_file_header(bitmap_info);
}

void ScreenCapture::fill_bitmap_info(const HBITMAP hbitmap)
{
	BITMAP bmp;
	auto   result = GetObject(hbitmap, sizeof(BITMAP), &bmp);

	if (result == 0)
		throw Win32Error(GetLastError(), "fill bitmap info: GetObject failed for BITMAP");

	bitmap_info.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
	bitmap_info.bmiHeader.biWidth        = bmp.bmWidth;
	bitmap_info.bmiHeader.biHeight       = bmp.bmHeight;
	bitmap_info.bmiHeader.biPlanes       = bmp.bmPlanes;
	bitmap_info.bmiHeader.biBitCount     = bmp.bmBitsPixel;
	bitmap_info.bmiHeader.biCompression  = BI_RGB;
	// bitmap_info.bmiHeader.biSizeImage = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4 * bmp.bmHeight;
	bitmap_info.bmiHeader.biSizeImage    = bmp.bmWidth * bmp.bmHeight * 4;
	bitmap_info.bmiHeader.biClrImportant = 0;
}

void ScreenCapture::fill_bitmap_file_header(const BITMAPINFO& info)
{
	file_header.bfType      = 0x4D42;
	file_header.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + info.bmiHeader.biSizeImage;
	file_header.bfReserved1 = 0;
	file_header.bfReserved2 = 0;
	file_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + info.bmiHeader.biSize;
}

std::vector<std::byte> ScreenCapture::to_bmp()
{
	BITMAPINFOHEADER&      info_header = bitmap_info.bmiHeader;
	std::vector<std::byte> bmp;

	// Allocate enough memory to hold the bitmap header + it's data.
	// When performing a std::copy with std::back_inserter, all elements
	// copied will get push_back() to the end and the vector's size will
	// increment by itself.
	bmp.reserve(file_header.bfSize);
	std::copy(
		reinterpret_cast<std::byte*>(&file_header),
		reinterpret_cast<std::byte*>(&file_header) + sizeof(BITMAPFILEHEADER),
		std::back_inserter(bmp));
	std::copy(
		reinterpret_cast<std::byte*>(&info_header),
		reinterpret_cast<std::byte*>(&info_header) + sizeof(BITMAPINFOHEADER),
		std::back_inserter(bmp));

	// Unlike std::copy, GetDIBits expects a LPVOID to where the data should
	// be stored. In this case, GetDIBits will not push_back() so the
	// vector will never know that it's size changed, so we perform a
	// resize() on the vector to tell it the new total size of it.
	bmp.resize(file_header.bfSize);
	auto result = GetDIBits(
		hdc.get(),
		hbitmap_ptr.get(),
		0,
		info_header.biHeight,
		bmp.data() + file_header.bfOffBits,
		&bitmap_info,
		DIB_RGB_COLORS);

	if (result == 0)
		throw Win32Error(GetLastError(), "to bmp: GetDIBits failed");

	return bmp;
}

void ss_png_write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
	if (png_ptr == NULL) {
		std::cout << "WRITE_CALLBACK: png_ptr is null\n";
		return;
	}

	std::cout << "--------------------------------\n";
	std::cout << "row: " << row << "\n";
	std::cout << "pass: " << pass << "\n";
}

void ss_png_on_err(png_structp, png_const_charp error_msg)
{
	std::cout << "user error fn: " << error_msg << "\n";
}

void ss_png_on_warn(png_structp, png_const_charp warning_msg)
{
	std::cout << "user warning fn: " << warning_msg << "\n";
}

void ss_png_on_write_to_vec(png_structp png_ptr, png_bytep data, png_size_t length)
{
	// TODO: 
	//    Catch possible bad_alloc exceptions that might cause the png C code
	//    to return unexpectedly and cause memory leaks on perform_png_conversion.
	if (png_ptr == NULL)
		return;

	auto* output = reinterpret_cast<std::vector<std::byte>*>(png_get_io_ptr(png_ptr));

	assert(output != nullptr);
	std::copy(
		reinterpret_cast<std::byte*>(data), 
		reinterpret_cast<std::byte*>(data) + length,
		std::back_inserter(*output));
}

void ss_png_on_flush_to_vec(png_structp)
{
	// No need to flush to a vector.
}

std::vector<std::byte*> ScreenCapture::get_bitmap_rows(std::vector<std::byte>& bmp, LONG height, LONG width) const
{
	assert(height > 0 || width > 0);
	auto* row_ptr = bmp.data();
	auto  rows    = std::vector<std::byte*>();

	rows.reserve(height);

	for (LONG i = 0; i < height; i++) {
		// Rows on a bitmap start from bottom to top, so every new row coming should be
		// placed at the beginning.
		rows.insert(rows.begin(), row_ptr);

		row_ptr += width * 4;
	}

	return rows;
}

std::vector<std::byte> ScreenCapture::perform_png_conversion(BITMAPINFOHEADER& info_header, std::vector<std::byte*>& rows) const
{
	std::vector<std::byte> output;
	png_voidp              error_ptr   = NULL;
	png_structp            png_ptr     = NULL;
	png_infop              info_ptr    = NULL;

	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		error_ptr,
		ss_png_on_err,
		ss_png_on_warn);

	if (!png_ptr)
		throw std::exception("failed to create a png write struct!");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		throw std::exception("to png: failed to create info struct");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		throw std::exception("to png: failed to assign jmpbuf");
	}

	png_set_write_fn(
		png_ptr,
		reinterpret_cast<void*>(&output),
		ss_png_on_write_to_vec,
		ss_png_on_flush_to_vec);
	png_set_write_status_fn(png_ptr, ss_png_write_row_callback);
	png_set_IHDR(
		png_ptr,
		info_ptr,
		info_header.biWidth,
		info_header.biHeight,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	png_set_rows(png_ptr, info_ptr, reinterpret_cast<png_bytepp>(rows.data()));

	// Transform bitmap's little endian to big endian bytes using a transform.
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return output;
}


std::vector<std::byte> ScreenCapture::to_png()
{
	BITMAPINFOHEADER&       info_header = bitmap_info.bmiHeader;
	std::vector<std::byte>  bitmap;
	std::vector<std::byte*> rows;

	// Get bitmap.
	bitmap.resize(info_header.biSizeImage);
	auto result = GetDIBits(
		hdc.get(),
		hbitmap_ptr.get(),
		0,
		info_header.biHeight,
		bitmap.data(),
		&bitmap_info,
		DIB_RGB_COLORS);

	if (result == 0)
		throw Win32Error(GetLastError(), "to png: GetDIBits failed");

	rows = get_bitmap_rows(bitmap, info_header.biHeight, info_header.biWidth);
	return perform_png_conversion(info_header, rows);
}