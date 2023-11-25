#include <windows.h>

#define internal static
#define global static

#define u32 unsigned int
#define s32 signed int
#define u64 unsigned long
#define s64 signed long
#define r32 float
#define r64 double
#define b32 unsigned int

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

struct screen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    u32 Width;
    u32 Height;
};

global b32 GlobalRunning; 

internal LRESULT CALLBACK Win32MainCallback(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_CLOSE:
            {
                GlobalRunning = false;
                //DestroyWindow(WindowHandle);
            } break;

        case WM_DESTROY:
            {
                GlobalRunning = false;
                //PostQuitMessage(0);
            } break;

        default:
            {
                return(DefWindowProc(WindowHandle, Message, wParam, lParam));
            }
    }

    return(0);
}

int CALLBACK 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
        LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX WindowClass;
    MSG Message;

    WindowClass.cbSize        = sizeof(WNDCLASSEX);
    WindowClass.style         = 0;
    WindowClass.lpfnWndProc   = Win32MainCallback;
    WindowClass.cbClsExtra    = 0;
    WindowClass.cbWndExtra    = 0;
    WindowClass.hInstance     = hInstance;
    WindowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = "SkrivWindowClass";
    WindowClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(RegisterClassEx(&WindowClass))
    {
        HWND Window= CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WindowClass.lpszClassName,
                WindowClass.lpszMenuName,
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 512, 512,
                NULL, NULL, hInstance, NULL);

        if(Window)
        {
            GlobalRunning = true;
            //ShowWindow(WindowHandle, nCmdShow);
            //UpdateWindow(WindowHandle);

            screen_buffer Buffer_;
            screen_buffer *Buffer = &Buffer_;
            Buffer->Width = 512;
            Buffer->Height = 512;

            Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
            Buffer->Info.bmiHeader.biWidth = Buffer->Width;
            Buffer->Info.bmiHeader.biHeight = Buffer->Height;
            Buffer->Info.bmiHeader.biPlanes = 1;
            Buffer->Info.bmiHeader.biBitCount = 32;
            Buffer->Info.bmiHeader.biCompression = BI_RGB;

            u32 BytesPerPixel = 4;
            u32 BitmapMemorySize = Buffer->Width*BytesPerPixel*Buffer->Height;

            Buffer->Memory = VirtualAlloc(0, BitmapMemorySize,
                    MEM_RESERVE|MEM_COMMIT,
                    PAGE_READWRITE);

            while(GlobalRunning)
            {
                while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                HDC DeviceContext = GetDC(Window);
                StretchDIBits(DeviceContext, 
                        0, 0, Buffer->Width, Buffer->Height,
                        0, 0, Buffer->Width, Buffer->Height,
                        Buffer->Memory,
                        &Buffer->Info,
                        DIB_RGB_COLORS, SRCCOPY);

                ReleaseDC(Window, DeviceContext);

                //NOTE: Unused params:
                nCmdShow;
                lpCmdLine;
                hPrevInstance;
            }

        }
    }

    return(0);
}
