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

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Width*BYTES_PER_PIXEL;

}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, win32_screen_buffer *Buffer)
{
    StretchDIBits(DeviceContext, 
            0, 0, Buffer->Width, Buffer->Height,
            0, 0, Buffer->Width, Buffer->Height,
            Buffer->Memory,
            &Buffer->Info,
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
internal glyph
Win32LoadFont(char *FontName, u32 PointsSize)
{
    win32_screen_buffer Result = {};
    HDC DeviceContext = CreateCompatibleDC(GetDC(NULL));

    u32 DummySize = MAX_FONT_WIDTH*MAX_FONT_HEIGHT*sizeof(u32);
    void *DummyDrawing = VirtualAlloc(0, DummySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    ZeroMemory(DummyDrawing, DummySize);

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

    HBITMAP Bitmap = CreateDIBSection(DeviceContext, &Info, DIB_RGB_COLORS, &DummyDrawing, 0, 0);
    SelectObject(DeviceContext, Bitmap);
    SetBkColor(DeviceContext, RGB(0, 0, 0));

    wchar_t A = 'A';
    int FontHeight = -MulDiv(PointsSize, GetDeviceCaps(DeviceContext, LOGPIXELSY), 72);
    HFONT FontHandle;

    FontHandle = CreateFontA(FontHeight, 0, 0, 0,
                             FW_NORMAL, //Weight
                             FALSE, // Italic
                             FALSE, // Underline
                             FALSE, // Strikeout
                             DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY,
                             DEFAULT_PITCH|FF_DONTCARE,
                             FontName);

    SelectObject(DeviceContext, FontHandle);
    SetTextColor(DeviceContext, RGB(255, 255, 255));
    SIZE Size;
    GetTextExtentPoint32W(DeviceContext, &A, 1, &Size);
    TextOutW(DeviceContext, 0, 0, &A, 1);

    s32 MinX = 10000;
    s32 MinY = 10000;
    s32 MaxX = -10000;
    s32 MaxY = -10000;

    for(s32 Y = 0;
            Y < MAX_FONT_HEIGHT;
            ++Y)
    {
        u32 *Row = (u32 *)DummyDrawing + Y*MAX_FONT_WIDTH;
        for(s32 X = 0;
                X < MAX_FONT_WIDTH;
                ++X)
        {
            u32 *Pixel = Row + X;
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
        }
    }

    glyph Glyph;
    Glyph.Width = (u32)(MaxX - MinX);
    Glyph.Height = (u32)(MaxY - MinY);

    Glyph.Memory = VirtualAlloc(0, Glyph.Width*Glyph.Height*sizeof(u32), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    u32 *SourceRow = (u32 *)DummyDrawing + MinY*MAX_FONT_WIDTH;
    u32 *DestRow = (u32 *)Glyph.Memory;
    for(s32 Y = MinY;
            Y <= MaxY;
            ++Y)
    {
        u32 *SourcePixel = SourceRow + MinX;
        u32 *DestPixel = DestRow;
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
        ++SourceRow;
        ++DestRow;
    }

#if 0

    int PreStepX = 128;

    int BoundWidth = Size.cx + 2*PreStepX;
    if(BoundWidth > MAX_FONT_WIDTH) 
    {
        BoundWidth = MAX_FONT_WIDTH;
    }
    int BoundHeight = Size.cy;
    if(BoundHeight > MAX_FONT_HEIGHT)
    {
        BoundHeight = MAX_FONT_HEIGHT;
    }

    s32 MinX = 10000;
    s32 MinY = 10000;
    s32 MaxX = -10000;
    s32 MaxY = -10000;

    u32 *Row = (u32 *)DummyDrawing;

    for(s32 Y = 0;
            Y < BoundHeight;
            ++Y)
    {
        u32 *Pixel = Row;
        for(s32 X = 0;
                X < BoundHeight;
                ++X)
        {
            if(*Pixel != 0)
            {
                if(MinX > X)
                {
                    MinX = X;
                }
            }
        }
    }

#endif

    //ReleaseDC(NULL, DeviceContext);
    DeleteDC(DeviceContext);
    return(Glyph);
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
    Win32ScreenBuffer.Memory = TotalScreenMemory;

    Win32ResizeClientScreen(Window, &Win32ScreenBuffer);

    win32_program_code ProgramCode = Win32LoadProgramCode(
            "w:/build/skriv.dll", 
            "w:/build/skriv_temp.dll", 
            "w:/build/lock.tmp");

    offscreen_buffer Buffer = {};

    glyph Glyph = {};

    b32 FontLoaded = false;

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

        Buffer.Memory = Win32ScreenBuffer.Memory;
        Buffer.Width = Win32ScreenBuffer.Width;
        Buffer.Height = Win32ScreenBuffer.Height;
        Buffer.Pitch = Win32ScreenBuffer.Pitch;

        if(!FontLoaded)
        {
             Glyph = Win32LoadFont("Verdana", 209);
             FontLoaded = true;
             ProgramMemory->Glyph = &Glyph;
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
