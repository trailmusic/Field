#include "core/runtime/TailGuard.h"
#include <cassert>

int main()
{
    field::core::runtime::TailGuard tail;

    tail.setDesiredSeconds(2.5);
    bool applied = tail.applyIfChanged(nullptr, /*transportStopped*/true, /*currentLatencySamples*/0);
    assert(applied && tail.getAppliedSeconds() == 2.5);

    tail.setDesiredSeconds(3.0);
    applied = tail.applyIfChanged(nullptr, /*transportStopped*/false, 0);
    assert(!applied && tail.getAppliedSeconds() == 2.5);

    applied = tail.applyIfChanged(nullptr, /*transportStopped*/true, 0);
    assert(applied && tail.getAppliedSeconds() == 3.0);

    tail.setDesiredSeconds(0.0);
    applied = tail.applyIfChanged(nullptr, /*transportStopped*/true, /*latencySamples*/128);
    assert(applied && tail.getAppliedSeconds() == 0.0);

    return 0;
}


