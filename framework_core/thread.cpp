#include "thread.h"
#include <thread>

namespace tengu {
void yield()
{
    std::this_thread::yield();
}
} // namespace tengu
