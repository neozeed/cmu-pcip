rem echo off
if x%1 == x goto die
if %1 == all goto dev
if %1 == srcdev goto dev
if %1 == srclib goto lib
if %1 == srccmd goto cmd
if %1 == local goto local
if %1 == tar goto tar
goto die

:dev
	cd srcdev\netdev
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..
	if not %1 == all goto exit

:lib
 cd srclib\domain
	cd srclib\3com
	msmake makefile
	if errorlevel 1 goto error

	cd ..\domain
	msmake makefile
	if errorlevel 1 goto error

	cd ..\em
	msmake makefile
	if errorlevel 1 goto error

	cd ..\interlan
	msmake makefile
	if errorlevel 1 goto error

	cd ..\internet
	msmake makefile
	if errorlevel 1 goto error

	cd ..\net
	msmake makefile
	if errorlevel 1 goto error

	cd ..\pc
	msmake makefile
	if errorlevel 1 goto error

	cd ..\pronet
	msmake makefile
	if errorlevel 1 goto error

	cd ..\serial
	msmake makefile
	if errorlevel 1 goto error

	cd ..\slip
	msmake makefile
	if errorlevel 1 goto error

	cd ..\task
	msmake makefile
	if errorlevel 1 goto error

	cd ..\tcp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\tftp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\udp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\ni5210
	msmake makefile
	if errorlevel 1 goto error

	cd ..\wd8003
	msmake makefile
	if errorlevel 1 goto error

	cd ..\trw
	msmake makefile
	if errorlevel 1 goto error

	cd ..\pcnet
	msmake makefile
	if errorlevel 1 goto error

	cd ..\at
	msmake makefile
	if errorlevel 1 goto error

	cd ..\packet
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..
	if not %1 == all goto exit

:cmd
	cd srccmd\bootp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\custom
	msmake makefile
	if errorlevel 1 goto error

	cd ..\cookie
	msmake makefile
	if errorlevel 1 goto error

	cd ..\haddr
	msmake makefile
	if errorlevel 1 goto error

	cd ..\hostname
	msmake makefile
	if errorlevel 1 goto error

	cd ..\lpr
	msmake makefile
	if errorlevel 1 goto error

	cd ..\monitor
	msmake makefile
	if errorlevel 1 goto error

	cd ..\netname
	msmake makefile
	if errorlevel 1 goto error

	cd ..\nicname
	msmake makefile
	if errorlevel 1 goto error

	cd ..\netwatch
	msmake makefile
	if errorlevel 1 goto error

	cd ..\iprint
	msmake makefile
	if errorlevel 1 goto error

	cd ..\onhook
	msmake makefile
	if errorlevel 1 goto error

	cd ..\ping
	msmake makefile
	if errorlevel 1 goto error

	cd ..\setclock
	msmake makefile
	if errorlevel 1 goto error

	cd ..\term
	msmake makefile
	if errorlevel 1 goto error

	cd ..\tftp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\tn
	msmake makefile
	if errorlevel 1 goto error

	cd ..\whois
	msmake makefile
	if errorlevel 1 goto error

	cd ..\rexec
	msmake makefile
	if errorlevel 1 goto error

	cd ..\red
	msmake makefile
	if errorlevel 1 goto error

	cd ..\smtp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\pop2
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..
	goto exit

:local
	cd srccmd\remp
	msmake makefile
	if errorlevel 1 goto error

	cd ..\gw
	msmake makefile
	if errorlevel 1 goto error

	cd ..\gw2
	msmake makefile
	if errorlevel 1 goto error

	cd ..\backup
	msmake makefile
	if errorlevel 1 goto error

	cd ..\rvd
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..

	cd srcdev\rvd
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..
	goto exit

:tar
	cd srccmd\tar
	msmake makefile
	if errorlevel 1 goto error

	cd ..\..
	goto exit

:error
	echo Make failed
	goto end

:die
	echo No such option "%1"
	echo Usage: "make [all | srcdev | srclib | srccmd | tar]"
	goto end

:exit

:end
