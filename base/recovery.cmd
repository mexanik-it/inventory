@echo off
chcp 65001 >nul
setlocal
cls

set "SRC=\\10.67.1.36\oit_techbase\list-points"
set "BAK=C:\Inventory-base\Backup"

:: Восстановление HTML
robocopy "%BAK%" "%SRC%" "list-points.html" /COPY:DAT /R:5 /W:10 /V /NP
if %errorlevel% geq 8 (
    echo ERROR: HTML restore failed.
) else (
    echo OK: HTML restored from %BAK%
)

:: Восстановление XLS
robocopy "%BAK%" "%SRC%" "list-points.xls" /COPY:DAT /R:5 /W:10 /V /NP
if %errorlevel% geq 8 (
    echo ERROR: XLS restore failed.
) else (
    echo OK: XLS restored from %BAK%
)

echo.
echo Done.
pause
