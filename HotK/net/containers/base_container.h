#pragma once

#include <cstddef>
#include <vector>

namespace hotk::net::containers {
	class BaseContainer {
	public:
		virtual const char* data() const noexcept = 0;
		virtual std::size_t size() const noexcept = 0;
	};
}