#pragma once

#include <windows.h>

namespace hotk {
	namespace winutils {
		namespace deleters {
			struct HDCDeleter {
				void operator()(HDC);
			};

			struct CompatibleDCDeleter {
				void operator()(HDC);
			};

			struct HBitmapDeleter {
				void operator()(HBITMAP);
			};
		}
	}
}

