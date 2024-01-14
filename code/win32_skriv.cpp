#include <windows.h>

#include "skriv_platform.h"
#include "skriv_intrinsics.h"
#include "skriv_math.h"
#include "win32_skriv.h"

#include <stdio.h>

global b32 GlobalRunning; 
global win32_screen_buffer Win32ScreenBuffer;

DWORD RenderThreadID = 0;

internal FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

internal win32_program_code
Win32LoadProgramCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_program_code Result = {};

    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

        CopyFile(SourceDLLName, TempDLLName, FALSE);

        Result.DLLHandle = LoadLibraryA(TempDLLName);
        if(Result.DLLHandle)
        {
            Result.UpdateAndRender = (update_and_render *)
                GetProcAddress(Result.DLLHandle, "UpdateAndRender");

            Result.IsValid = (Result.UpdateAndRender && 1);
        }
    }
    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
    }

    return(Result);
}

internal void
Win32UnloadProgramCode(win32_program_code *ProgramCode)
{
    if(ProgramCode->DLLHandle)
    {
        FreeLibrary(ProgramCode->DLLHandle);
        ProgramCode->DLLHandle = 0;
    }

    ProgramCode->IsValid = false;
    ProgramCode->UpdateAndRender = 0;
}

internal void
Win32ResizeClientScreen(HWND Window, win32_screen_buffer *Buffer)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    u32 Width = ClientRect.right - ClientRect.left;
    u32 Height = ClientRect.bottom - ClientRect.top;

    Buffer->Info.bmiHeader.biSize = sizeof(Win32ScreenBuffer.Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Width;
    Buffer->Info.bmiHeader.biHeight = Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    Buffer->Bitmap.Width = Width;
    Buffer->Bitmap.Height = Height;
    Buffer->Bitmap.Pitch = Width*BYTES_PER_PIXEL;

}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, win32_screen_buffer *ScreenBuffer)
{
    bitmap *Buffer = &ScreenBuffer->Bitmap;
    StretchDIBits(DeviceContext, 
            0, 0, Buffer->Width, Buffer->Height,
            0, 0, Buffer->Width, Buffer->Height,
            Buffer->Memory,
            &ScreenBuffer->Info,
            DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK 
Win32MainCallback(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_CLOSE:
            {
                GlobalRunning = false;
            } break;

        case WM_DESTROY:
            {
                GlobalRunning = false;
            } break;

        case WM_SIZE:
            {
                PostThreadMessageW(RenderThreadID, Message, wParam, lParam);
            };

        default:
            {
                return(DefWindowProc(WindowHandle, Message, wParam, lParam));
            }
    }

    return(0);
}

internal HWND 
CreateOutputWindow(void)
{
    WNDCLASSEX WindowClass;
    WindowClass.cbSize        = sizeof(WNDCLASSEX);
    WindowClass.style         = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc   = Win32MainCallback;
    WindowClass.cbClsExtra    = 0;
    WindowClass.cbWndExtra    = 0;
    WindowClass.hInstance     = GetModuleHandleW(NULL);
    WindowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = "SkrivWindowClass";
    WindowClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    HWND Result = {0};

    if(RegisterClassEx(&WindowClass))
    {
        Result = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WindowClass.lpszClassName,
                WindowClass.lpszMenuName,
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 512, 512,
                NULL, NULL, WindowClass.hInstance, NULL);
    }

    return(Result);
}

internal void
ProcessMessages(HWND Window)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_SIZE:
                {
                    Win32ResizeClientScreen(Window, &Win32ScreenBuffer);
                } break;
        case WM_PAINT:
            {
                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);
                Win32DisplayBufferInWindow(DeviceContext, &Win32ScreenBuffer);
                EndPaint(Window, &Paint);
            } break;
        }
    }
}

FREE_FILE_MEMORY(FreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

READ_ENTIRE_FILE(ReadEntireFile)
{
    entire_file Result = {};

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateToU64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                        (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    FreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else 
            {
                //logging
            }
        }
        else
        {
            //logging
        }
        CloseHandle(FileHandle);
    }
    else
    {
        //logging
    }
    return(Result);
}


#define MAX_FONT_WIDTH  1024
#define MAX_FONT_HEIGHT 1024

struct temp_font
{
    void *DummyDrawing;
    u32 DummyDrawingSize;
    HFONT FontHandle;
    s32 FontHeight;
    TEXTMETRIC TextMetric;
    HDC DeviceContext;
};

internal temp_font
Win32InitializeFont(char *FontName, u32 PointsSize)
{
    temp_font Result = {};

    Result.DeviceContext = CreateCompatibleDC(GetDC(NULL));

    BITMAPINFO Info = {};
    Info.bmiHeader.biSize = sizeof(Info.bmiHeader);
    Info.bmiHeader.biWidth = MAX_FONT_WIDTH;
    Info.bmiHeader.biHeight = MAX_FONT_HEIGHT;
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = 32;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biSizeImage = 0;
    Info.bmiHeader.biXPelsPerMeter = 0;
    Info.bmiHeader.biYPelsPerMeter = 0;
    Info.bmiHeader.biClrUsed = 0;
    Info.bmiHeader.biClrImportant = 0;

    Result.DummyDrawingSize = MAX_FONT_WIDTH*MAX_FONT_HEIGHT*sizeof(u32);
    Result.DummyDrawing = VirtualAlloc(0, Result.DummyDrawingSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    HBITMAP Bitmap = CreateDIBSection(Result.DeviceContext, &Info, DIB_RGB_COLORS, &Result.DummyDrawing, 0, 0);
    SelectObject(Result.DeviceContext, Bitmap);
    SetBkColor(Result.DeviceContext, RGB(0, 0, 0));

    Result.FontHeight = -MulDiv(PointsSize, GetDeviceCaps(Result.DeviceContext, LOGPIXELSY), 72);

    Result.FontHandle = CreateFontA(Result.FontHeight, 0, 0, 0,
                                    FW_NORMAL, //Weight
                                    FALSE, // Italic
                                    FALSE, // Underline
                                    FALSE, // Strikeout
                                    DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY,
                                    //ANTIALIASED_QUALITY,
                                    DEFAULT_PITCH|FF_DONTCARE,
                                    FontName);

    SelectObject(Result.DeviceContext, Result.FontHandle);
    GetTextMetrics(Result.DeviceContext, &Result.TextMetric);
    SetTextColor(Result.DeviceContext, RGB(255, 255, 255));

    return(Result);
}
internal void
Win32LoadGlyph(temp_font *TempFont, bitmap *GlyphOutput, void *GlyphLocation, char Codepoint)
{
    ZeroMemory(TempFont->DummyDrawing, TempFont->DummyDrawingSize);

    SIZE Size;
    GetTextExtentPoint32W(TempFont->DeviceContext, &(wchar_t)Codepoint, 1, &Size);

    s32 PreStepX = 128;

    s32 BoundWidth = (s32)(Size.cx + 2*PreStepX);
    s32 BoundHeight = (s32)(Size.cy);

    if(BoundWidth > MAX_FONT_WIDTH)
    {
        BoundWidth = MAX_FONT_WIDTH;
    }
    if(BoundHeight > MAX_FONT_HEIGHT)
    {
        BoundHeight = MAX_FONT_HEIGHT;
    }

    TextOutW(TempFont->DeviceContext, PreStepX, 0, &(wchar_t)Codepoint, 1);

    s32 MinX = 10000;
    s32 MinY = 10000;
    s32 MaxX = -10000;
    s32 MaxY = -10000;

    u32 *Row = (u32 *)TempFont->DummyDrawing + (MAX_FONT_HEIGHT - BoundHeight)*MAX_FONT_WIDTH;
    for(s32 Y = 0;
            Y < BoundHeight;
            ++Y)
    {
        u32 *Pixel = Row;
        for(s32 X = 0;
                X < BoundWidth;
                ++X)
        {
            if(*Pixel != 0)
            {
                if(MinX > X)
                {
                    MinX = X;
                }

                if(MinY > Y)
                {
                    MinY = Y;
                }

                if(MaxX < X)
                {
                    MaxX = X;
                }

                if(MaxY < Y)
                {
                    MaxY = Y;
                }
            }
            ++Pixel;
        }
        Row += MAX_FONT_WIDTH;
    }

    //r32 KerningChange = 0;
    //if(MinX <= MaxX)
    //{
    //    s32 
    //}

    bitmap Glyph;
    Glyph.Width = (u32)(MaxX - MinX + 1);
    Glyph.Height = (u32)(MaxY - MinY + 1);
    Glyph.Pitch = BYTES_PER_PIXEL*Glyph.Width;

    Glyph.Memory = GlyphLocation;

    u8 *SourceRow = (u8 *)TempFont->DummyDrawing + BYTES_PER_PIXEL*MAX_FONT_WIDTH*(MAX_FONT_HEIGHT - BoundHeight + MinY);
    u8 *DestRow = (u8 *)Glyph.Memory;
    for(s32 Y = MinY;
            Y <= MaxY;
            ++Y)
    {
        u32 *SourcePixel = (u32 *)SourceRow + MinX;
        u32 *DestPixel = (u32 *)DestRow;
        for(s32 X = MinX;
                X <= MaxX;
                ++X)
        {
            r32 Gray = (r32)(*SourcePixel & 0xFF);
            v4 PixelColor = {255.0f, 255.0f, 255.0f, Gray};

            PixelColor = SRGB255ToLinear1(PixelColor);
            PixelColor.rgb *= PixelColor.a;
            PixelColor = Linear1ToSRGB255(PixelColor);

            *DestPixel++ = (((u32)(PixelColor.a + 0.5f) << 24) |
                            ((u32)(PixelColor.r + 0.5f) << 16) | 
                            ((u32)(PixelColor.g + 0.5f) << 8) | 
                            ((u32)(PixelColor.b + 0.5f) << 0));
            ++SourcePixel;
        }

        SourceRow += BYTES_PER_PIXEL*MAX_FONT_WIDTH;
        DestRow += Glyph.Pitch;
    }


    //ReleaseDC(NULL, DeviceContext);

    //Glyph.Width = MAX_FONT_WIDTH;
    //Glyph.Height = MAX_FONT_HEIGHT;
    //Glyph.Pitch = MAX_FONT_WIDTH*BYTES_PER_PIXEL;
    //Glyph.Memory = DummyDrawing;
    //return(Glyph);
    *GlyphOutput = Glyph;
}

internal void
Win32FinalizeFont(temp_font *TempFont)
{
    VirtualFree(TempFont->DummyDrawing, 0, MEM_RELEASE);
    DeleteDC(TempFont->DeviceContext);
}

internal DWORD WINAPI
EditorThread(LPVOID Param)
{
    program_memory *ProgramMemory = (program_memory *)VirtualAlloc(0, sizeof(program_memory), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    ProgramMemory->BackgroundColor = 0xFF323232;
    ProgramMemory->PlatformAPI.FreeFileMemory = FreeFileMemory;
    ProgramMemory->PlatformAPI.ReadEntireFile = ReadEntireFile;

    HWND Window = (HWND)Param;

    GlobalRunning = true;


    //TODO: hva hvis man kobler til en ny skjerm? eller bytter til en annen skjerm med annen oppløsning?
    u32 TotalScreenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
    u32 TotalScreenHeight = GetSystemMetrics(SM_CYMAXIMIZED);

    u32 TotalScreenMemorySize = TotalScreenWidth*TotalScreenHeight*BYTES_PER_PIXEL;
    void *TotalScreenMemory = VirtualAlloc(0, TotalScreenMemorySize,
            MEM_RESERVE|MEM_COMMIT,
            PAGE_READWRITE);
    Win32ScreenBuffer.Bitmap.Memory = TotalScreenMemory;

    Win32ResizeClientScreen(Window, &Win32ScreenBuffer);

    win32_program_code ProgramCode = Win32LoadProgramCode(
            "w:/build/skriv.dll", 
            "w:/build/skriv_temp.dll", 
            "w:/build/lock.tmp");

    bitmap Buffer = {};

    b32 FontLoaded = false;


    loaded_font Font;
    while(GlobalRunning)
    {
        FILETIME NewDLLWriteTime = Win32GetLastWriteTime("w:/build/skriv.dll");

        if(CompareFileTime(&NewDLLWriteTime, &ProgramCode.DLLLastWriteTime) != 0)
        {
            Win32UnloadProgramCode(&ProgramCode);
            ProgramCode = Win32LoadProgramCode(
                    "w:/build/skriv.dll", 
                    "w:/build/skriv_temp.dll", 
                    "w:/build/lock.tmp");
        }

        ProcessMessages(Window);

        Buffer.Memory = Win32ScreenBuffer.Bitmap.Memory;
        Buffer.Width =  Win32ScreenBuffer.Bitmap.Width;
        Buffer.Height = Win32ScreenBuffer.Bitmap.Height;
        Buffer.Pitch =  Win32ScreenBuffer.Bitmap.Pitch;

        if(!FontLoaded)
        {
            temp_font TempFont = Win32InitializeFont("Consolas", 150);
            
            u32 GlyphMemorySize = Megabytes(10);
            u32 GlyphsSize = (u32)('~' - ' ')*sizeof(loaded_font);
            Font.GlyphMemory = VirtualAlloc(0, GlyphMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            Font.Glyphs = (bitmap *)VirtualAlloc(0, GlyphsSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            void *PositionInGlyphMemory = Font.GlyphMemory;
            for(char Codepoint = ' ';
                    Codepoint <= '~';
                    ++Codepoint)
            {
                bitmap *Glyph = (bitmap *)((u8 *)Font.Glyphs + (Codepoint - ' ')*sizeof(bitmap));
                Win32LoadGlyph(&TempFont, Glyph, PositionInGlyphMemory, Codepoint);
                PositionInGlyphMemory = (u8 *)PositionInGlyphMemory + Glyph->Height*Glyph->Pitch; 

            }
            Win32FinalizeFont(&TempFont);
            FontLoaded = true;
            ProgramMemory->Font = &Font;
        }

        if(ProgramCode.UpdateAndRender)
        {
            ProgramCode.UpdateAndRender(&Buffer, ProgramMemory);
        }


        HDC DeviceContext = GetDC(Window);

        Win32DisplayBufferInWindow(DeviceContext, &Win32ScreenBuffer);

        ReleaseDC(Window, DeviceContext);

    }

    ExitProcess(0);
}


int CALLBACK 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
        LPSTR lpCmdLine, int nCmdShow)
{
    HWND Window = CreateOutputWindow();

    if(Window)
    {
        CreateThread(0, 0, EditorThread, Window,  0, &RenderThreadID);

        for(;;)
        {
            MSG Message;
            GetMessageW(&Message, 0, 0, 0);
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    return(0);
}
