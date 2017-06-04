#pragma once

#include <memory>
#include <windows.h>
#include <gdiplus.h>
#include "../errors/errors.h"
#include "errors.h"

#pragma comment(lib, "gdiplus.lib")

namespace hotk {
	namespace graphics {
		class Graphics {
		private:
			Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
			ULONG_PTR                    gdiPlusToken;
			CLSID                        clsID;

			void get_encoder_clsid(const WCHAR* format, CLSID* pClsID, hotk::errors::ErrorCode &err) const;

		public:
			Graphics();
			void save_bitmap_to_file(const WCHAR *filename, HBITMAP hBitmap) const;
			HBITMAP capture_screen() const;
		};
	}
}