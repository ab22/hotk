#include "deleters.h"

using namespace hotk::winutils::deleters;

void HDCDeleter::operator()(HDC hdc)
{
    if (hdc != nullptr)
        ReleaseDC(NULL, hdc);
}

void CompatibleDCDeleter::operator()(HDC hdc)
{
    if (hdc != nullptr)
        DeleteDC(hdc);
}

void HBitmapDeleter::operator()(HBITMAP handle)
{
    if (handle != nullptr)
        DeleteObject(handle);
}
