#include "winutils.h"

using namespace hotk::winutils;

void deleters::HDCDeleter::operator()(HDC hdc)
{
	if (hdc != nullptr)
		ReleaseDC(NULL, hdc);
}

void deleters::CompatibleDCDeleter::operator()(HDC hdc)
{
	if (hdc != nullptr)
		DeleteDC(hdc);
}

void deleters::HBitmapDeleter::operator()(HBITMAP handle)
{
	if (handle != nullptr)
		DeleteObject(handle);
}