#include "APP.hpp"

auto APP::run() -> void
{
    while (m_Running) {
        frame(this);
    }
}