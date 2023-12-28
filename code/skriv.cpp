#include "skriv.h"
#include "skriv_platform.h"
#include "skriv_math.h"

#include "skriv_intrinsics.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#pragma comment(linker, "-EXPORT:UpdateAndRender")

#include "skriv_render.cpp"


extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    ClearBackground(Buffer, Memory->BackgroundColor);
    
    if(Memory->LoadedFont.ContentsSize == 0)
    {
        Memory->LoadedFont = Memory->PlatformAPI.ReadEntireFile("C:/windows/fonts/arial.ttf");
    };
    
    
    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8 *)Memory->LoadedFont.Contents, 
            stbtt_GetFontOffsetForIndex((u8 *)Memory->LoadedFont.Contents, 0));

    int Codepoint = 'A';
    r32 ScaleX = 0.1f;
    r32 ScaleY = 0.1f;
    int ix0, ix1, iy0, iy1;

    stbtt_GetCodepointBitmapBox(&Font, Codepoint, ScaleX, ScaleY,
            &ix0, &iy0, &ix1, &iy1);
    int Width = ix1 - ix0;
    int Height = iy1 - iy0;

    stbtt_MakeCodepointBitmap(&Font, (unsigned char *)Buffer->Memory, Width, Height, Buffer->Pitch,
            ScaleX, ScaleY, Codepoint);

    //int Width, Height, XOffset, YOffset;
    //u8 *MonoBitmap = stbtt_GetCodepointBitmap(&Font, 0, stbtt_ScaleForPixelHeight(&Font, 128.0f),
    //        CodePoint, &Width, &Height, &XOffset, &YOffset);


    //DrawRectangle(Buffer, , , );
    //DrawHorizontalLine(Buffer, 0.5f*Buffer->Height, V4(0, 0, 1, 1), 2.0f); 

    //v4 Color = V4(1, 1, 0, 1);
    //v2 Min = V2(0.25f*Buffer->Width, 0.25f*Buffer->Height);
    //v2 Max = V2(0.25f*Buffer->Width, 0.75f*Buffer->Height);
    //r32 LineWidth = 1.0f;

    //v2 HalfLineWidth = V2(0.5f*LineWidth, 0);
    //DrawRectangle(Buffer, Min - HalfLineWidth, Max + HalfLineWidth, Color);

    //DrawHorizontalLine(Buffer, 0.5f*Buffer->Height, V4(1, 1, 1, 1));
    DrawGrid(Buffer, 100.0f, V4(0, 1, 0, 1));
#if 0
    v2 Min = {0, 0};

    v4 Color = V4(1, 0, 0, 1);

    v2 Center = 0.5f*V2i(Buffer->Width, Buffer->Height);
    v2 Max = 2*Center;

    v2 Offset = V2i(100, 100);

    DrawRectangle(Buffer, Center - Offset, Center + Offset, HexToV4(0xFF0000FF));

    r32 GridSize = 10000.0f/Buffer->Height;
    DrawGrid(Buffer, GridSize, V4(0, 1, 0, 1));
#endif
}

