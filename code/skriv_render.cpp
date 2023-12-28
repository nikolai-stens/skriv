internal void
ClearBackground(offscreen_buffer *Buffer, u32 Color)
{
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
            *Pixel++ = Color;

        }
        Row += Buffer->Pitch;
    }
}

internal void
DrawRectangle(offscreen_buffer *Buffer, v2 Min, v2 Max, v4 Color)
{
    u32 WidthMax = (Buffer->Width);
    u32 HeightMax = (Buffer->Height);

    u32 MinX = RoundReal32ToUInt32(Min.x);
    u32 MinY = RoundReal32ToUInt32(Min.y);
    u32 MaxX = RoundReal32ToUInt32(Max.x);
    u32 MaxY = RoundReal32ToUInt32(Max.y);

    if(MinX < 0) {MinX = 0;}
    if(MinX > WidthMax) {MinX = WidthMax;}

    if(MaxX < 0) {MaxX = 0;}
    if(MaxX > WidthMax) {MaxX = WidthMax;}

    if(MinY < 0) {MinY = 0;}
    if(MinY > HeightMax) {MinY = HeightMax;}

    if(MaxY < 0) {MaxY = 0;}
    if(MaxY > HeightMax) {MaxY = HeightMax;}
    u32 ColorU32 = ((RoundReal32ToUInt32(Color.a * 255.0f) << 24) |
                    (RoundReal32ToUInt32(Color.r * 255.0f) << 16) |
                    (RoundReal32ToUInt32(Color.g * 255.0f) << 8) |
                    (RoundReal32ToUInt32(Color.b * 255.0f) << 0));

    u8 *Row = (u8 *)Buffer->Memory + MinX*BYTES_PER_PIXEL + MinY*Buffer->Pitch;
    for(u32 Y = MinY;
            Y < MaxY;
            ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(u32 X = MinX;
                X < MaxX;
                ++X)
        {
            *Pixel++ = ColorU32;
        }
        Row += Buffer->Pitch;
    }
}

internal void
DrawOrthogonalLine(offscreen_buffer *Buffer, v2 Min, v2 Max, v4 Color, r32 LineWidth = 1.0f)
{
    if(Min.x == Max.x)
    {
        v2 HalfLineWidth = V2(0.5f*LineWidth, 0);
        DrawRectangle(Buffer, Min - HalfLineWidth, Max + HalfLineWidth, Color);
    }
    else if(Min.y == Max.y)
    {
        v2 HalfLineWidth = V2(0, 0.5f*LineWidth);
        DrawRectangle(Buffer, Min - HalfLineWidth, Max + HalfLineWidth, Color);
    }
}

internal void
DrawVerticalLine(offscreen_buffer *Buffer, r32 X, v4 Color, r32 LineWidth = 1.0f)
{
    DrawOrthogonalLine(Buffer, V2(X,0),V2(X, (r32)Buffer->Height), Color, LineWidth);
}

internal void
DrawHorizontalLine(offscreen_buffer *Buffer, r32 Y, v4 Color, r32 LineWidth = 1.0f)
{
    DrawOrthogonalLine(Buffer, V2(0.0f, Y), V2((r32)Buffer->Width, Y), Color, LineWidth);
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
