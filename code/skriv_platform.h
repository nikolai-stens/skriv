#if !defined(SKRIV_PLATFORM_H)

#pragma warning(disable:4201)
#pragma warning(disable:4702)
#pragma warning(disable:4189)
#pragma warning(disable:4100)
#pragma warning(disable:4505)

#define internal static
#define global static

#define u8 unsigned char
#define u32 unsigned int
#define s32 signed int
#define u64 unsigned long long
#define s64 signed long long
#define r32 float
#define r64 double
#define b32 unsigned int

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define Max(Value1, Value2) ((Value1) >= (Value2)) ? Value1 : Value2

#define BYTES_PER_PIXEL 4

struct bitmap
{
    void *Memory;
    u32 Width;
    u32 Height;
    u32 Pitch;
};

#define NUMBER_OF_GLYPHS 128
#define GLYPH_MEMORY_SIZE Megabytes(10)

struct loaded_font
{
    void *GlyphMemory;
    bitmap *Glyphs;
    u32 GlyphCount;
    char LoadedCodepoints[NUMBER_OF_GLYPHS];
    u32 CurrentCodepointIndex;
    void *NextFreeGlyphMemory;

    u32 Size;
    char *Name;
};


inline u32 
SafeTruncateToU64(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};

#define FREE_FILE_MEMORY(name) void name(void *Memory)
typedef FREE_FILE_MEMORY(free_file_memory);

#define READ_ENTIRE_FILE(name) entire_file name(char *Filename)
typedef READ_ENTIRE_FILE(read_entire_file);

#define LOAD_GLYPH_TO_MEMORY(name) void name(loaded_font *Font, char Codepoint, char *FontName,  u32 PointSize)
typedef LOAD_GLYPH_TO_MEMORY(load_glyph_to_memory);

struct platform_api
{
    free_file_memory *FreeFileMemory;
    read_entire_file *ReadEntireFile;
    load_glyph_to_memory *LoadGlyphToMemory;
};

struct program_memory
{
    u32 BackgroundColor;
    loaded_font *Font;
    platform_api PlatformAPI;
};

#define UPDATE_AND_RENDER(name) void name(bitmap *Buffer, program_memory *Memory)
typedef UPDATE_AND_RENDER(update_and_render);

#define SKRIV_PLATFORM_H
#endif
