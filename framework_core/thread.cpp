#include <thread>
#include "thread.h"

namespace tengu {
void yield() {
    std::this_thread::yield();
}
} // namespace tengu
