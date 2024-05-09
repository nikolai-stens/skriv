#include "skriv.h"
#include "skriv_platform.h"
#include "skriv_intrinsics.h"
#include "skriv_math.h"


#pragma comment(linker, "-EXPORT:UpdateAndRender")

#include "skriv_render.cpp"

internal void
DrawCharacter(bitmap *Buffer, bitmap *Glyph, v2 Pos, v4 Color = {1, 1, 1, 1})
{

    u32 PosX = (u32)Pos.x;
    u32 PosY = (u32)Pos.y;
    
    if(PosX < 0)
    {
        PosX = 0;
    }
    if(PosX > Buffer->Width)
    {
        PosX = Buffer->Width;
    }
    if(PosY < 0)
    {
        PosY = 0;
    }
    if(PosY > Buffer->Height)
    {
        PosY = Buffer->Height;
    }

    if(Glyph)
    {
        u8 *SourceRow = (u8 *)Glyph->Memory;
        u8 *DestRow = (u8 *)Buffer->Memory + PosY*Buffer->Pitch + PosX*BYTES_PER_PIXEL;

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

                SourceColor = Hadamard(SourceColor, Color);

                //SourceColor *= Color.a;

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

            SourceRow += Glyph->Pitch;
            DestRow += Buffer->Pitch;
        }
    }
}

internal u32
GlyphIsLoaded(loaded_font *Font, char Codepoint)
{
    u32 Index = 0;
    for(u32 TestIndex = 0;
            TestIndex < NUMBER_OF_GLYPHS;
            ++TestIndex)
    {
        if(Font->LoadedCodepoints[TestIndex] == Codepoint)
        {
            Index = TestIndex;
            break;
        }
    }

    return(Index);
}

internal bitmap *
GetBitmapFromCodepointIndex(loaded_font *Font, u32 Index)
{
    bitmap *Result = 0;

    Result = (bitmap *)((u8 *)Font->Glyphs + Index*sizeof(bitmap)); // 0 index reserved

    return(Result);
}

internal bitmap *
GetBitmapFromCodepoint(program_memory *Memory, char Codepoint)
{
    loaded_font *Font = Memory->Font;
    bitmap *Result = 0;
    u32 Index = GlyphIsLoaded(Font, Codepoint);

    if(Index)
    {
        Result = GetBitmapFromCodepointIndex(Font, Index);
    }
    else
    {
        Memory->PlatformAPI.LoadGlyphToMemory(Font, Codepoint, Font->Name, Font->PointSize);
    }

    return(Result);
}

// TODO: finn på noe bedre her
#define MAX_LINE_NUMBER 4096
struct document
{
    char *Contents;

    u32 NumberOfLines;
    u32 *LineSizeArray;
};


internal void
RenderTextLine(bitmap *Buffer, program_memory *Memory, 
        document *Doc, u32 LineNumber,
        v4 Color = {1, 1, 1, 1})
{

    char *TextOut = Doc->Contents + LineNumber;
    for(u32 LineIndex = 0;
            LineIndex < LineNumber;
            ++LineIndex)
    {
        u32 LineSize = *(Doc->LineSizeArray + LineIndex);
        TextOut += LineSize;
    }
    u32 TextSize = *(Doc->LineSizeArray + LineNumber);

    r32 AtX = 0;
    r32 AtY = (r32)(Buffer->Height - (LineNumber + 1)*Memory->Font->GlyphHeight);
    if(TextOut)
    {
        for(u32 Char = 0;
                Char < TextSize;
                ++Char)
        {
            bitmap *Glyph = GetBitmapFromCodepoint(Memory, *TextOut);
            if(Glyph)
            {
                DrawCharacter(Buffer, Glyph, V2(AtX, AtY), Color);
                TextOut++;
                AtX += Glyph->Width;
            }
            else
            {
                break;
            }
        }
    }
}

internal document
ParseFileAsDocument(program_memory *Memory, entire_file *File)
{
    document Result = {};
    Result.LineSizeArray = (u32 *)Memory->PlatformAPI.Allocate(MAX_LINE_NUMBER);
    Result.Contents = (char *)File->Contents;

    if(File->ContentsSize)
    {
        u32 *CurrentLineSize = Result.LineSizeArray + Result.NumberOfLines;
        for(u32 ByteIndex = 0;
                ByteIndex < File->ContentsSize;
                ++ByteIndex)
        {
            ++(*CurrentLineSize);
            char Codepoint = *((char *)File->Contents + ByteIndex);
            if(Codepoint == '\n')
            {
                //TODO: include zero char in size? 
                if(Codepoint != 0)
                {
                    ++Result.NumberOfLines;
                    CurrentLineSize = Result.LineSizeArray + Result.NumberOfLines;
                }
            }
        }
    }
    return(Result);
}

static b32 DocumentParsed;
static entire_file TestFile;
static document TestDoc;

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    ClearBackground(Buffer, Memory->BackgroundColor);
    

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

    //v4 Blue = V4(0, 0, 1, 1);
    //DrawRectangle(Buffer, V2(0,0), V2((r32)Glyph->Width, (r32)Glyph->Height), Blue);

//    if(Glyph)
//    {
//        v2 GlyphPos = V2((r32)Glyph->Width, (r32)Glyph->Height);
//        DrawRectangle(Buffer, Pos, GlyphPos + Pos, V4(1, 0, 1, 1)); 
//    }
//

    if(!DocumentParsed)
    {
        TestFile = Memory->PlatformAPI.ReadEntireFile("w:/skriv/test/testfile.txt");
        TestDoc = ParseFileAsDocument(Memory, &TestFile);
    }


    for(u32 Line = 0;
            Line < TestDoc.NumberOfLines;
            ++Line)
    {
        RenderTextLine(Buffer, Memory, &TestDoc, Line);
    }
}

