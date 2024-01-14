#if !defined(WIN32_SKRIV_H)

struct win32_screen_buffer
{
    BITMAPINFO Info;
    bitmap Bitmap;
};


struct win32_program_code
{
    HMODULE DLLHandle;
    FILETIME DLLLastWriteTime;

    update_and_render *UpdateAndRender;

    b32 IsValid;
};

#define WIN32_SKRIV_H
#endif


