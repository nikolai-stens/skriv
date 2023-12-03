@echo off

set CompilerFlags= -O2 -MTd -nologo -W4 -Z7 
set LinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp

cl %CompilerFlags% w:\skriv\code\skriv.cpp -Fmskriv.map -LD /link -incremental:no -opt:ref -PDB:skriv_%random%.pdb
cl %CompilerFlags% w:\skriv\code\win32_skriv.cpp /link %LinkerFlags%

del lock.tmp

popd

