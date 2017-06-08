#include "graphics.h"

#include <memory>
#include <vector>

using namespace Gdiplus;
using namespace hotk;
using namespace hotk::winutils;

inline void graphics::Graphics::startup_gdi_plus()
{
	auto err_code = GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, NULL);

	if (err_code != Status::Ok)
		throw graphics::GraphicsInitError(err_code, "graphics ctor: could not startup GDI+");
}

inline void graphics::Graphics::load_encoders_clsids()
{
	errors::ErrorCode err;
	get_encoder_clsid(L"image/bmp", bmpCLSID, err);

	if (err)
		throw err;
}

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
	this->startup_gdi_plus();
	this->load_encoders_clsids();
	this->build_device_contexts();
}

void graphics::Graphics::get_encoder_clsid(const WCHAR* format, CLSID& cls_id, errors::ErrorCode &err) const noexcept
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in byte

	auto err_code = GetImageEncodersSize(&num, &size);
	if (err_code != Status::Ok) {
		err = std::move(graphics::EncoderClsIDError(err_code, "could not get image encoders size"));
		return;
	} else if (size == 0) {
		err = std::move(graphics::EncoderClsIDError(-1, "encoders size is 0"));
		return;
	}

	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	err_code = GetImageEncoders(num, size, pImageCodecInfo);

	if (err_code != Status::Ok) {
		err = std::move(graphics::EncoderClsIDError(err_code,"failed to get image encoders"));
		return;
	}

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			cls_id = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return;  // Success
		}
	}

	free(pImageCodecInfo);
	err = std::move(graphics::GraphicsInitError(-2, "could not find encoder info"));
	return;
}

void graphics::Graphics::save_bitmap_to_file(const WCHAR *filename, HBITMAP hBitmap) const
{
	auto image = std::make_unique<Bitmap>(hBitmap, (HPALETTE)NULL);
	image->Save(filename, &bmpCLSID, NULL);
}

std::unique_ptr<HBITMAP__, deleters::HBitmapDeleter> graphics::Graphics::capture_screen() const
{
	// Get screen dimensions.
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (width == 0) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: failed to get screen width");
	}

	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (height == 0) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: failed to get screen height");
	}

	// Create the Bitmap.
	HBITMAP handler = CreateCompatibleBitmap(hdc.get(), width, height);
	auto bitmap_handler = std::unique_ptr<HBITMAP__, deleters::HBitmapDeleter>(handler);
	if (handler == nullptr) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: CreateCompatibleBitmap failed");
	}

	// Use the previously created device context with the bitmap.
	auto result = SelectObject(hDest.get(), handler);
	if (result == NULL || result == HGDI_ERROR) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: SelectObject failed");
	}

	// Copy from the desktop device context to the bitmap device context
	// call this once per 'frame'.
	auto bb_result = BitBlt(hDest.get(), 0, 0, width, height, hdc.get(), 0, 0, SRCCOPY);
	if (bb_result == NULL) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: BitBlt failed");
	}

	return bitmap_handler;
}