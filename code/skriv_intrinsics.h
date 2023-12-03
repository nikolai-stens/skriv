#if !defined(SKRIV_INTRINSICS_H)

#include <math.h>

inline u32 RoundReal32ToUInt32(r32 Value)
{
    u32 Result = (u32)roundf(Value);

    return(Result);
}


#define SKRIV_INTRINSICS_H
#endif
