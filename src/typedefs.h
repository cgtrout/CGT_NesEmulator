#include <cstdint>

using ubyte = std::uint8_t;
using uword = std::uint16_t;
using uint = std::uint32_t;
using word = std::int16_t;

using CpuClockCycles = std::int32_t;
using PpuClockCycles = std::int32_t;

//#define CpuToMaster( val ) (val * 3)

#define CpuToPpu( val ) (val * 3)
#define PpuToCpu( val ) (val / 3)
#define PpuToCpuMod( val ) (val % 3)