#include "skriv.h"
#include "skriv_platform.h"
#include "skriv_intrinsics.h"
#include "skriv_math.h"


#pragma comment(linker, "-EXPORT:UpdateAndRender")

#include "skriv_render.cpp"


internal void
DrawCharacter(offscreen_buffer *Buffer, glyph *Glyph)
{
    u8 *SourceRow = (u8 *)Glyph->Memory;
    u8 *DestRow = (u8 *)Buffer->Memory;

    for(u32 Y = 0;
           Y < Glyph->Height;
           ++Y)
    {
        u32 *SourcePixel = (u32 *)SourceRow;
        u32 *DestPixel   = (u32 *)DestRow;
        for(u32 X = 0;
                X < Glyph->Width;
                ++X)
        {
            v4 SourceColor =
            {
                (r32)((*SourcePixel >> 16) & 0xFF),
                (r32)((*SourcePixel >> 8) & 0xFF),
                (r32)((*SourcePixel >> 0) & 0xFF),
                (r32)((*SourcePixel >> 24) & 0xFF)
            };

            SourceColor = SRGB255ToLinear1(SourceColor);

            v4 DestColor =
            {
                (r32)((*DestPixel >> 16) & 0xFF),
                (r32)((*DestPixel >> 8) & 0xFF),
                (r32)((*DestPixel >> 0) & 0xFF),
                (r32)((*DestPixel >> 24) & 0xFF)
            };

            DestColor = SRGB255ToLinear1(DestColor);

            v4 Result = (1.0f - SourceColor.a)*DestColor + SourceColor;

            Result = Linear1ToSRGB255(Result);

            *DestPixel = (((u32)(Result.a + 0.5f) << 24) |
                          ((u32)(Result.r + 0.5f) << 16) | 
                          ((u32)(Result.g + 0.5f) << 8) | 
                          ((u32)(Result.b + 0.5f) << 0));

            ++SourcePixel;
            ++DestPixel;
        }

        SourceRow += BYTES_PER_PIXEL*Glyph->Width;
        DestRow += Buffer->Pitch;
    }
}


#define MAX_FONT_WIDTH 1024
#define MAX_FONT_HEIGHT 1024
extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    ClearBackground(Buffer, Memory->BackgroundColor);
    
    DrawGrid(Buffer, 100.0f, V4(0, 1, 0, 1));

    glyph *GlyphA = Memory->Glyph;

#if 0
    for(u32 Y = 0;
            Y < Buffer->Height;
            ++Y)
    {
        u8 *SourceRow = (u8 *)GlyphA->Memory + Y*BYTES_PER_PIXEL*GlyphA->Width;
        u8 *DestRow = (u8 *)Buffer->Memory + Y*Buffer->Pitch;
        for(u32 X = 0;
                X < Buffer->Width;
                ++X)
        {
            u32 *SourcePixel = (u32 *)SourceRow + X;
            u32 *DestPixel = (u32 *)DestRow + X;
            *DestPixel = *SourcePixel;
        }
    }
#endif

    DrawCharacter(Buffer, GlyphA);
}

