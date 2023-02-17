//for win32
typedef unsigned char ubyte;
typedef unsigned short uword;

typedef unsigned int uint;
typedef short word;

typedef int CpuClockCycles;
typedef int PpuClockCycles;

#define CpuToMaster( val ) (val * 3)

#define PpuToMaster( val ) (val / 3)
#define PpuToMasterMod( val ) (val % 3)
