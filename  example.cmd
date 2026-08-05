@echo off
chcp 1251
cls
echo [92m======================================================================
echo [92m===                Select mainer and pools                         ===
echo.
echo [93m   ###  Miner - Pool  ###
echo [93m        ~~~~~~~~~~~~ [0m
echo.
echo == SBMainer ==
echo [91m 1. [96m--pool [94mzeph.kryptex.network:7030 
echo [91m 2. [96m--pool [94mpool.hashvault.pro:443 
echo [91m 3. [96m--pool [94mzeph.2miners.com:2222 
echo [91m 4. [96m--pool [94mfr.zephyr.herominers.com:1123 
echo [91m 5. [96m--pool [94mzeph.2miners.com:2222 
echo [91m 6. [96m--pool [94mde-zephyr.miningocean.org:5342 
echo [91m 7. [96m--pool [94mfr-zephyr.miningocean.org:5332 [0m
echo.
echo == XMRig ==
echo [91m a. [96m--pool [33mzeph.kryptex.network:7030
echo [91m b. [96m--pool [33mpool.hashvault.pro:443
echo [91m c. [96m--pool [33mfr.zephyr.herominers.com:1123
echo [91m d. [96m--pool [33mzeph.2miners.com:2222
echo [91m e. [96m--pool [33mzeph.2miners.com:2222
echo [91m f. [96m--pool [33msg-zephyr.miningocean.org:5462[0m
echo == 
echo [91m 0.  [92mexit [0m

@choice /C 12345670abcdef > nul
If %errorlevel%==5 ( 
	SBMainer\SRBMiner-MULTI.exe zeph.2miners.com:2222 
	exit )
If %errorlevel%==4 echo Вы выбрали 4
If %errorlevel%==3 echo Вы выбрали 3
If %errorlevel%==2 echo Вы выбрали 2
If %errorlevel%==1 echo Вы выбрали 1
If Errorlevel 0 goto :EOF


::YES
::	if %errorlevel%==1 echo Вы выбрали 1
::NO
::	if %errorlevel%==2 echo Вы выбрали 2


echo.
echo.
echo.
echo.
echo.
echo.
echo [31m	red
echo [32m	green
echo [33m	brown
echo [34m	blue
echo [35m	magenta
echo [36m	cyan
echo [90m	gray
echo [91m	litht red
echo [92m	light green
echo [93m	litht yellow
echo [94m	light blue
echo [95m	light magenta
echo [96m	light cyan
echo [0m	сброс настроек

pause
exit
