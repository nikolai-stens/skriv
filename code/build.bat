@echo off

set CompilerFlags= -Od -MTd -nologo /Feskriv.exe -W4 -wd4702 -wd4189 -Z7 
set LinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

cl %CompilerFlags% w:\skriv\code\win32_skriv.cpp /link %LinkerFlags%

popd

