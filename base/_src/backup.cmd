@echo off
chcp 65001 >nul
::setlocal

:: Удаленная папка
set "SRC=\\10.67.1.36\oit_techbase\list-points"

:: Проверка существования папки
if not exist "%SRC%" (
    powershell -Command Write-Host "ERROR: Папка не найдена: %SRC%" -ForegroundColor Red
    exit /b 1
)

:: Получаем дату в формате YYYYMMDD через wmic
for /f "tokens=2 delims==" %%i in ('wmic os get localdatetime /value') do set "dt=%%i"
set "YYYY=%dt:~0,4%"
set "MM=%dt:~4,2%"
set "DD=%dt:~6,2%"

:: Формируем дату в виде ГГГГ-ММ-ДД
set "datestamp=%YYYY%-%MM%-%DD%"

powershell -Command Write-Host "Дата для бэкапа: %datestamp%" -ForegroundColor Cyan
powershell -Command Write-Host "Папка источника: %SRC%" -ForegroundColor Cyan

:: Файлы для бэкапа
set "file1=list-points.html"
set "file2=list-points.xls"

set "allOk=1"

:: Копируем и переименовываем в ту же папку
copy "%SRC%\%file1%" "%SRC%\list-points_%datestamp%.html.bak" >nul
if %errorlevel% equ 0 (
    powershell -Command Write-Host "OK: %file1% -> list-points_%datestamp%.html.bak" -ForegroundColor Green
) else (
    powershell -Command Write-Host "ERROR: Не удалось скопировать %file1%" -ForegroundColor Red
    set "allOk=0"
)

copy "%SRC%\%file2%" "%SRC%\list-points_%datestamp%.xls.bak" >nul
if %errorlevel% equ 0 (
    powershell -Command Write-Host "OK: %file2% -> list-points_%datestamp%.xls.bak" -ForegroundColor Green
) else (
    powershell -Command Write-Host "ERROR: Не удалось скопировать %file2%" -ForegroundColor Red
    set "allOk=0"
)

if "%allOk%"=="1" (
    powershell -Command Write-Host "Готово. Все файлы успешно скопированы." -ForegroundColor Green
    endlocal
    pause
    exit /b 0
) else (
    powershell -Command Write-Host "Готово. Были ошибки при копировании." -ForegroundColor Red
    endlocal
    pause
    exit /b 1
)