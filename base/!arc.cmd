@echo off
chcp 1251
cls
echo  [92m===== start =====
rem for /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set datetime=%%a
rem set timestamp=%datetime:~0,4%-%datetime:~4,2%-%datetime:~6,2%_%datetime:~8,2%-%datetime:~10,2%-%datetime:~12,2%
rem echo Timestamp: %timestamp%

for /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set datetime=%%a
set dt=%datetime:~0,4%-%datetime:~4,2%-%datetime:~6,2%_%datetime:~8,2%-%datetime:~10,2%

::lha.exe a d:\flash-"%dt%".lha  *
rem 7z.exe a "f:\ archive\sd" *

set time_start=%time:~0,2%:%time:~3,2%
rem с русскими не работает ==> lha.exe a "f:\ archive\flash-'%dt%'".lha / personal
:: > 7z.exe a  f:\flash-"%dt%" /*
:: 7z.exe a  -bt  ..\INST-OIT-"%dt%" * 
::7z a -pvbnz1002 ..\INST-OIT-"%dt%" /*
::rar a -r ..\INST-OIT-"%dt%" *
lha.exe a ..\inventory-base-"%dt%".lha  * 
set time_end=%time:~0,2%:%time:~3,2%

echo.
echo [0m=====
echo .
echo %time_start%
echo %time_end%

echo [33m================================================================================
echo [92m========       Transferring INVENTORY files to an FTP server            ========
echo.[0m
::choice /T 3 /D Y /M "Автопродолжение через 3 сек. Продолжить? "
echo  [94m Starting process .. 
echo [96m
curl -i -T ..\inventory-base-"%dt%".lha -u a0831755 ftp://denkin.ru/domains/denkin.ru/public_html/tmp/
curl -i -T ..\inventory-base-"%dt%".lha -u dimon ftp://86.110.167.86/ftp/_inventory/
echo [0m
echo.

::copy -V -Y "\\10.67.1.36\oit_techbase\list-points\list-points.html" "\\10.67.1.36\oit_techbase\list-points\list-points.html.bak"
::copy -V -Y "\\10.67.1.36\oit_techbase\list-points\list-points.xls" "\\10.67.1.36\oit_techbase\list-points\list-points.xls.bak"
                                                  
pause