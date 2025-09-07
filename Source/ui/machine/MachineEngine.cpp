#include "MachineEngine.h"

void MachineEngine::push (const float* L, const float* R, int n)
{
    if (freeze.load() || n <= 0) return;
    int s1, n1, s2, n2; fifo.prepareToWrite (n, s1, n1, s2, n2);
    if (n1 > 0) { if (L) memcpy (buf.getWritePointer(0) + s1, L, sizeof(float)*n1); if (R) memcpy (buf.getWritePointer(1) + s1, R, sizeof(float)*n1); else memcpy (buf.getWritePointer(1) + s1, buf.getWritePointer(0) + s1, sizeof(float)*n1); }
    if (n2 > 0) { if (L) memcpy (buf.getWritePointer(0) + s2, L + n1, sizeof(float)*n2); if (R) memcpy (buf.getWritePointer(1) + s2, R + n1, sizeof(float)*n2); else memcpy (buf.getWritePointer(1) + s2, buf.getWritePointer(0) + s2, sizeof(float)*n2); }
    fifo.finishedWrite (n1 + n2);
}

void MachineEngine::analyzeAsync (Target, Quality)
{
    // MVP stub: emit empty results immediately
    juce::ScopedLock sl (resLock);
    lastResults.clear();
}

void MachineEngine::cancel() { cancelFlag.store (true); }

bool MachineEngine::hasResults() const { return ! lastResults.empty(); }

std::vector<ParamPatch> MachineEngine::takeResults()
{
    juce::ScopedLock sl (resLock);
    auto out = std::move (lastResults);
    lastResults.clear();
    return out;
}


