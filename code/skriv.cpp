#include "skriv.h"
#include "skriv_platform.h"

#pragma comment(linker, "-EXPORT:UpdateAndRender")

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    u32 Color1 = 0xFFFF0000;
    u32 Color2 = 0x0000FFFF;
    u8 *Row = (u8 *)Buffer->Memory;
    for(u32 Y = 0;
            Y < Buffer->Height;
            ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(u32 X = 0;
                X < Buffer->Width;
                ++X)
        {
            if((X < 0.2f*Buffer->Width) || (X > 0.8f*Buffer->Width))
            {
                //0xAA RR GG BB
                *Pixel++ = Color1; 
            }
            else
            {
                *Pixel++ = Color2; 
            }

        }
        Row += Buffer->Pitch;
    }
}

