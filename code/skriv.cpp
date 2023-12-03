#include "skriv.h"
#include "skriv_platform.h"
#include "skriv_math.h"

#include "skriv_intrinsics.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#pragma comment(linker, "-EXPORT:UpdateAndRender")


internal void
DrawRectangle(offscreen_buffer *Buffer, v2 Min, v2 Max, v4 Color)
{
    u32 WidthMax = (Buffer->Width);
    u32 HeightMax = (Buffer->Height);

    u32 MinX = RoundReal32ToUInt32(Min.x);
    u32 MinY = RoundReal32ToUInt32(Min.y);
    u32 MaxX = RoundReal32ToUInt32(Max.x);
    u32 MaxY = RoundReal32ToUInt32(Max.y);

    r32 MinXPercent = (Min.x - (r32)MinX);
    r32 MinYPercent = (Min.y - (r32)MinY);
    r32 MaxXPercent = (Max.x - (r32)MaxX);
    r32 MaxYPercent = (Max.y - (r32)MaxY);

    if(MinX < 0) {MinX = 0;}
    if(MinX > WidthMax) {MinX = WidthMax;}

    if(MaxX < 0) {MaxX = 0;}
    if(MaxX > WidthMax) {MaxX = WidthMax;}

    if(MinY < 0) {MinY = 0;}
    if(MinY > HeightMax) {MinY = HeightMax;}

    if(MaxY < 0) {MaxY = 0;}
    if(MaxY > HeightMax) {MaxY = HeightMax;}


    u8 *Row = (u8 *)Buffer->Memory + MinX*BYTES_PER_PIXEL + MinY*Buffer->Pitch;
    for(u32 Y = MinY;
            Y < MaxY;
            ++Y)
    {
        r32 Alpha = Color.a;
        if(Y == MinY)
        {
            Alpha *= MinYPercent;
        }
        else if(Y == MaxY)
        {
            Alpha *= MaxYPercent;
        }
        u32 *Pixel = (u32 *)Row;
        for(u32 X = MinX;
                X < MaxX;
                ++X)
        {
            if(X == MinX)
            {
                Alpha = MinXPercent;
            }
            else if(X == MaxX)
            {
                Alpha = MaxXPercent;
            }
            *Pixel++ = ((RoundReal32ToUInt32(Alpha   * 255.0f) << 24) |
                        (RoundReal32ToUInt32(Color.r * 255.0f) << 16) |
                        (RoundReal32ToUInt32(Color.g * 255.0f) << 8) |
                        (RoundReal32ToUInt32(Color.b * 255.0f) << 0));

        }
        Row += Buffer->Pitch;
    }
}

internal void
DrawOrthogonalLine(offscreen_buffer *Buffer, v2 Min, v2 Max, v4 Color, r32 LineWidth = 1)
{
    if(Min.x == Max.x)
    {
        DrawRectangle(Buffer, Min - V2(LineWidth, 0), Max + V2(LineWidth, 0), Color);
    }
    else if(Min.y == Max.y)
    {
        DrawRectangle(Buffer, Min - V2(0, LineWidth), Max + V2(0, LineWidth), Color);
    }
}

internal void
DrawVerticalLine(offscreen_buffer *Buffer, r32 X, v4 Color, r32 LineWidth = 1.0f)
{
    DrawOrthogonalLine(Buffer, V2(X + LineWidth, 0.0f),V2(X + LineWidth, (r32)Buffer->Height), Color, LineWidth);
}

internal void
DrawHorizontalLine(offscreen_buffer *Buffer, r32 Y, v4 Color, r32 LineWidth = 1.0f)
{
    DrawOrthogonalLine(Buffer, V2(0.0f, Y + LineWidth), V2((r32)Buffer->Width, Y + LineWidth), Color, LineWidth);
}

internal void
DrawGrid(offscreen_buffer *Buffer, r32 GridSize, v4 Color)
{
    Assert(GridSize != 0.0f);
    u32 NumberOfHorLines = (u32)((Buffer->Height -2)/GridSize);
    u32 NumberOfVerLines = (u32)((Buffer->Width -2)/GridSize);

    r32 AlignedHorizontalGridSize = ((r32)(Buffer->Height -2))/((r32)NumberOfHorLines);
    r32 AlignedVerticalGridSize =   ((r32)(Buffer->Width -2))/((r32)NumberOfVerLines);

    for(u32 LineIndex = 0;
            LineIndex <= NumberOfHorLines;
            ++LineIndex)
    {
        DrawHorizontalLine(Buffer, (LineIndex*AlignedHorizontalGridSize), Color);
    }
    for(u32 LineIndex = 0;
            LineIndex <= NumberOfVerLines;
            ++LineIndex)
    {
        DrawVerticalLine(Buffer, (LineIndex*AlignedVerticalGridSize), Color);
    }
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    DrawRectangle(Buffer, V2i(0,0), V2i(Buffer->Width, Buffer->Height), HexToV4(Editor->BackgroundColor));


    v2 Min = {0, 0};

    v4 Color = V4(1, 0, 0, 1);

    v2 Center = 0.5f*V2i(Buffer->Width, Buffer->Height);
    v2 Max = 2*Center;

    v2 Offset = V2i(100, 100);

    DrawRectangle(Buffer, Center - Offset, Center + Offset, HexToV4(0xFF0000FF));

    r32 GridSize = 20.0f; //10000.0f/Buffer->Height;
    DrawGrid(Buffer, GridSize, V4(0, 1, 0, 1));
}

