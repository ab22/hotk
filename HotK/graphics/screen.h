#pragma once

#include "../winutils/deleters.h"
#include "../winutils/errors.h"
#include "screen_capture.h"
#include "errors.h"

#include <memory>
#include <vector>
#include <cstddef>
#include <windows.h>

namespace hotk::graphics::screen {
	using hotk::graphics::screen_capture::ScreenCapture;

	std::unique_ptr<ScreenCapture> capture_full_screen();
}