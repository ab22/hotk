#pragma once

#include "../winutils/deleters.h"
#include "../winutils/errors.h"
#include "errors.h"
#include <memory>
#include <vector>
#include <windows.h>

namespace hotk::graphics {
	using hotk::winutils::deleters::CompatibleDCDeleter;
	using hotk::winutils::deleters::HBitmapDeleter;
	using hotk::winutils::deleters::HDCDeleter;

	class Graphics {
	private:
		std::unique_ptr<HDC__, HDCDeleter> hdc;
		std::unique_ptr<HDC__, CompatibleDCDeleter> hDest;

		inline void build_device_contexts();
		std::unique_ptr<BITMAPINFO> create_bitmap_info(HBITMAP hbitmap);

	public:
		Graphics();
		std::unique_ptr<HBITMAP__, HBitmapDeleter> capture_screen() const;
		std::vector<byte> to_vector(HBITMAP hbitmap);
	};
}
