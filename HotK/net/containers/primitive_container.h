#pragma once

#include "base_container.h"
#include <type_traits>

namespace hotk::net::containers
{
	template<typename T, typename Enable = void>
	class PrimitiveContainer
	{
		static_assert(std::is_integral<T>::value, "Primitive Container Type Error: class only accepts integral types!");
	};

	template<typename T>
	class PrimitiveContainer<T, typename std::enable_if< std::is_integral<T>::value >::type >
		: public BaseContainer
	{
	private:
		T _value;

	public:
		PrimitiveContainer(T value)
			: _value(value)
		{
		}

		const char* data() const noexcept override final {
			return reinterpret_cast<const char*>(&_value);
		}

		std::size_t size() const noexcept override final {
			return sizeof(T);
		}
	};
}