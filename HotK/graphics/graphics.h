#pragma once

#include <windows.h>
#include <memory>
#include <gdiplus.h>
#include <vector>
#include "../errors/errors.h"
#include "../utils/winutils.h"
#include "errors.h"

namespace hotk::graphics {
	namespace deleters = hotk::winutils::deleters;

	class Graphics {
	private:
		using hdc_deleter = deleters::HDCDeleter;
		using compatible_dc_deleter = deleters::CompatibleDCDeleter;
		using hbitmap_deleter = hotk::winutils::deleters::HBitmapDeleter;

		std::unique_ptr<HDC__, hdc_deleter>                 hdc;
		std::unique_ptr<HDC__, compatible_dc_deleter>       hDest;

		inline void build_device_contexts();
		std::unique_ptr<BITMAPINFO> create_bitmap_info(HBITMAP hbitmap);

	public:
		Graphics();
		std::unique_ptr<HBITMAP__, hbitmap_deleter> capture_screen() const;
		std::vector<byte> to_vector(HBITMAP hbitmap);
	};
}