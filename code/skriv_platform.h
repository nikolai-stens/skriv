#if !defined(SKRIV_PLATFORM_H)

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

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define BITMAP_BYTES_PER_PIXEL 4

struct offscreen_buffer
{
    void *Memory;
    u32 Width;
    u32 Height;
    u32 Pitch;
};

#define UPDATE_AND_RENDER(name) void name(offscreen_buffer *Buffer)
typedef UPDATE_AND_RENDER(update_and_render);

#define SKRIV_PLATFORM_H
#endif
