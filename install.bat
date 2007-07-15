@echo off

set MM3DDIR="c:\Program Files\Misfit Model 3D"

regedit /i /s mm3d.reg

mkdir %MM3DDIR%
mkdir %MM3DDIR%\doc

xcopy /q /y /r mm3d.exe %MM3DDIR%
xcopy /q /y /r /e doc %MM3DDIR%\doc
