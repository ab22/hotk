#pragma once

#include "../winutils/deleters.h"
#include "../winutils/errors.h"

#include <Windows.h>
#include <vector>
#include <cstddef>

namespace hotk::graphics::screen_capture {
	using hotk::winutils::deleters::CompatibleDCDeleter;
	using hotk::winutils::deleters::HBitmapDeleter;
	using hotk::winutils::deleters::HDCDeleter;

	using HDCPtr = std::unique_ptr<HDC__, HDCDeleter>;
	using CompatibleDCPtr = std::unique_ptr<HDC__, CompatibleDCDeleter>;
	using HBITMAPPtr = std::unique_ptr<HBITMAP__, HBitmapDeleter>;

	class ScreenCapture {
	private:
		HDCPtr           hdc;
		HBITMAPPtr       hbitmap_ptr;
		BITMAPINFO       bitmap_info;
		BITMAPFILEHEADER file_header;

		void fill_bitmap_headers();
		void fill_bitmap_info(const HBITMAP);
		void fill_bitmap_file_header(const BITMAPINFO&);

		std::vector<std::byte*> get_bitmap_rows(std::vector<std::byte>& bmp, LONG height, LONG width) const;
		std::vector<std::byte> perform_png_conversion(BITMAPINFOHEADER& info_header, std::vector<std::byte*>& rows) const;

	public:
		ScreenCapture(HDCPtr, HBITMAPPtr);

		std::vector<std::byte> to_bmp();
		std::vector<std::byte> to_png();
	};
}
