#pragma once

#include <windows.h>
#include <memory>
#include <gdiplus.h>
#include "../errors/errors.h"
#include "../utils/winutils.h"
#include "errors.h"


#pragma comment(lib, "gdiplus.lib")

namespace hotk {
	namespace graphics {
		class Graphics {
		private:
			using hdc_deleter = hotk::winutils::deleters::HDCDeleter;
			using compatible_dc_deleter = hotk::winutils::deleters::CompatibleDCDeleter;
			using hbitmap_deleter = hotk::winutils::deleters::HBitmapDeleter;

			Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
			ULONG_PTR                    gdiPlusToken;
			CLSID                        bmpCLSID;

			std::unique_ptr<HDC__, hdc_deleter>                 hdc;
			std::unique_ptr<HDC__, compatible_dc_deleter>       hDest;

			void get_encoder_clsid(const WCHAR* format, CLSID& pClsID, hotk::errors::ErrorCode &err) const noexcept;
			inline void startup_gdi_plus();
			inline void load_encoders_clsids();
			inline void build_device_contexts();

		public:
			Graphics();
			void save_bitmap_to_file(const WCHAR *filename, HBITMAP hBitmap) const;
			std::unique_ptr<HBITMAP__, hbitmap_deleter> capture_screen() const;
		};
	}
}