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

#define BYTES_PER_PIXEL 4

struct editor_settings
{
    u32 BackgroundColor;
};
struct offscreen_buffer
{
    void *Memory;
    u32 Width;
    u32 Height;
    u32 Pitch;
};

#define UPDATE_AND_RENDER(name) void name(offscreen_buffer *Buffer, editor_settings *Editor)
typedef UPDATE_AND_RENDER(update_and_render);

#define SKRIV_PLATFORM_H
#endif
