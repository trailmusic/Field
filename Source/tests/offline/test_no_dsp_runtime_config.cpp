#include <cassert>
int main(){ static_assert(true, "build-only: ensures no hard dep on DspRuntimeConfig"); return 0; }
