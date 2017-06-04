#include "graphics.h"

using namespace Gdiplus;
using namespace hotk;

graphics::Graphics::Graphics()
{
	auto err_code = GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, NULL);

	if (err_code != Status::Ok)
		throw graphics::GDIStartupError(err_code, "graphics init: could not initialize GDI");

	// Load image encoder IDs
	errors::ErrorCode err;
	get_encoder_clsid(L"image/bmp", &clsID, err);

	if (err)
		throw err;
}

void graphics::Graphics::get_encoder_clsid(const WCHAR* format, CLSID* cls_id, errors::ErrorCode &err) const
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in byte

	auto err_code = GetImageEncodersSize(&num, &size);
	if (err_code != Status::Ok) {
		err = std::move(graphics::EncoderClsIDError(err_code, "get encoder clsid: could not get image encoders size"));
		return;
	} else if (size == 0) {
		err = std::move(graphics::EncoderClsIDError(-1, "get encoder clsid: encoders size is 0"));
		return;
	}

	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	err_code = GetImageEncoders(num, size, pImageCodecInfo);

	if (err_code != Status::Ok) {
		err = std::move(graphics::EncoderClsIDError(err_code,"get encoder clsid: failed to get image encoders"));
		return;
	}

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*cls_id = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return;  // Success
		}
	}

	free(pImageCodecInfo);
	err = std::move(graphics::GDIStartupError(-2, "get encoder clsid: could not find encoder info"));
	return;
}

void graphics::Graphics::save_bitmap_to_file(const WCHAR *filename, HBITMAP hBitmap) const
{
	auto image = std::make_unique<Bitmap>(hBitmap, (HPALETTE)NULL);
	image->Save(filename, &clsID, NULL);
}

HBITMAP graphics::Graphics::capture_screen() const
{
	// Get the desktop device context.
	HDC hdc = GetDC(NULL);

	if (hdc == NULL) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: GetDC failed");
	}

	// Create a device context to use ourselves.
	HDC hDest = CreateCompatibleDC(hdc);

	if (hDest == NULL) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: CreateCompatibleDC failed");
	}

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
	HBITMAP hbDesktop = CreateCompatibleBitmap(hdc, width, height);

	if (hbDesktop == NULL) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: CreateCompatibleBitmap failed");
	}

	// Use the previously created device context with the bitmap.
	auto result = SelectObject(hDest, hbDesktop);

	if (result == NULL || result == HGDI_ERROR) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: SelectObject failed");
	}

	//Copy from the desktop device context to the bitmap device context
	// call this once per 'frame'.
	auto bb_result = BitBlt(hDest, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

	if (bb_result == NULL) {
		int error_code = GetLastError();
		throw graphics::CaptureScreenError(error_code, "capture screen error: BitBlt failed");
	}

	// Release DCs.
	ReleaseDC(NULL, hdc);
	DeleteDC(hDest);

	return hbDesktop;
}