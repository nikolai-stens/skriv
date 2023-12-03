#include <windows.h>

#include "skriv_platform.h"
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

internal DWORD WINAPI
EditorThread(LPVOID Param)
{
    editor_settings *Editor = (editor_settings *)VirtualAlloc(0, sizeof(editor_settings), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    Editor->BackgroundColor = 0xFF323232;
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

        if(ProgramCode.UpdateAndRender)
        {
            ProgramCode.UpdateAndRender(&Buffer, Editor);
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
