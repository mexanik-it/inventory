@echo off
chcp 1251
cls
echo [33m================================================================================
echo [92m========       Transferring INVENTORY files to an FTP server            ========
echo.[94m--------           надо сделать при помощи write-to-ftp                 --------
echo.[0m
choice /T 5 /D Y /M "Автопродолжение через 5 сек. Продолжить? "
if %errorlevel%==1 (
echo  [96m Starting process .. [93m
curl -T linux\3.5-1\inventory ftp://a0831755:vbnz1002@ftp.denkin.ru/domains/denkin.ru/public_html/tmp/
) else (
    echo Выполнение прервано
    exit 1
)
IF %ERRORLEVEL% EQU 0 (
    ECHO [92m [Linux] - File transfer completed successfully... [0m
) ELSE (
    ECHO [91m [Linux] - An error occurred while transferring the file... [0m
)

echo.[0m
choice /T 5 /D Y /M "Автопродолжение через 5 сек. Продолжить? "
if %errorlevel%==1 (
echo  [96m Starting process .. [93m
curl -T windows\x86\3.4-1\inventory-x86.exe ftp://a0831755:vbnz1002@ftp.denkin.ru/domains/denkin.ru/public_html/tmp/
) else (
    echo Выполнение прервано
    exit 1
)
IF %ERRORLEVEL% EQU 0 (
    ECHO [92m [Windows] - File transfer completed successfully... [0m
) ELSE (
    ECHO [91m [Windows] An error occurred while transferring the file... [0m
)

echo.[0m
choice /T 5 /D Y /M "Автопродолжение через 5 сек. Продолжить? "
if %errorlevel%==1 (
echo  [96m Starting process .. [93m
curl -T windows\x64\4.1-1\inventory.exe ftp://a0831755:vbnz1002@ftp.denkin.ru/domains/denkin.ru/public_html/tmp/
) else (
    echo Выполнение прервано
    exit 1
)
IF %ERRORLEVEL% EQU 0 (
    ECHO [92m [Windows] - File transfer completed successfully... [0m
) ELSE (
    ECHO [91m [Windows] An error occurred while transferring the file... [0m
)

echo  [0m
pause