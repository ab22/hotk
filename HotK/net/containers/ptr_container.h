#pragma once

#include "base_container.h"

namespace hotk::net::containers {
    class PtrContainer : public BaseContainer {
    private:
        const char* _data;
        std::size_t _size;

    public:
        PtrContainer(const char* data, std::size_t size) noexcept {
            _data = data;
            _size = size;
        }

        const char* data() const noexcept override final {
            return _data;
        }

        std::size_t size() const noexcept override final {
            return _size;
        }
    };
}