#include "lib_spu.hpp"

extern "C" short SsVabTransCompleted(short immediateFlag)
{
    long ret = SpuIsTransferCompleted(immediateFlag);
    return ret;
}
