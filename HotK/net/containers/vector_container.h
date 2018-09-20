#pragma once

#include "base_container.h"

namespace hotk::net::containers {

	template<typename T>
	class VectorContainer : public BaseContainer {
	private:
		std::vector<T> _vec;
	public:
		VectorContainer(const std::vector<T>&& vec) noexcept {
			_vec = std::move(vec);
		}

		const char* data() const noexcept override final {
			return reinterpret_cast<const char*>(_vec.data());
		}

		std::size_t size() const noexcept override final {
			return _vec.size();
		}
	};

}