echo off
if x%1 == x goto die
if %1 == all goto libr
if %1 == lib goto libr
if %1 == srcdev goto dev
if %1 == srclib goto lib
if %1 == srccmd goto cmd
if %1 == ibm goto ibm
if %1 == pronet goto pronet
goto die

:libr
rem	del lib\*.lib 
	del lib\*.bak 
	if not %1 == all goto exit

:dev
	del srcdev\netdev\*.obj
	del srcdev\netdev\*.exe
	del srcdev\netdev\*.sys
	if not %1 == all goto exit

:lib
	del srclib\3com\*.obj
	del srclib\domain\*.obj
	del srclib\em\*.obj
	del srclib\interlan\*.obj
	del srclib\internet\*.obj
	del srclib\net\*.obj
	del srclib\pc\*.obj
	del srclib\pronet\*.obj
	del srclib\serial\*.obj
	del srclib\slip\*.obj
	del srclib\task\*.obj
	del srclib\tcp\*.obj
	del srclib\tftp\*.obj
	del srclib\udp\*.obj
	del srclib\ni5210\*.obj
	del srclib\wd8003\*.obj
	del srclib\trw\*.obj
	del srclib\pcnet\*.obj
	del srclib\at\*.obj
	del srclib\packet\*.obj
	if not %1 == all goto exit

:cmd
	del srccmd\bootp\*.obj
	del srccmd\bootp\*.exe
	del srccmd\cookie\*.obj
	del srccmd\cookie\*.exe
	del srccmd\custom\*.obj
	del srccmd\custom\*.exe
	del srccmd\haddr\*.obj
	del srccmd\haddr\*.exe
	del srccmd\hostname\*.obj
	del srccmd\hostname\*.exe
	del srccmd\iprint\*.obj
	del srccmd\iprint\*.exe
	del srccmd\lpr\*.obj
	del srccmd\lpr\?lpr.exe
	del srccmd\monitor\*.obj
	del srccmd\monitor\*.exe
	del srccmd\netname\*.obj
	del srccmd\netname\*.exe
	del srccmd\netwatch\*.obj
	del srccmd\netwatch\*.lib
	del srccmd\netwatch\*.bak
	del srccmd\netwatch\*.exe
	del srccmd\nicname\*.obj
	del srccmd\nicname\*.exe
	del srccmd\onhook\*.obj
	del srccmd\onhook\*.exe
	del srccmd\onhook\*.com
	del srccmd\ping\*.obj
	del srccmd\ping\*.exe
	del srccmd\setclock\*.obj
	del srccmd\setclock\*.exe
	del srccmd\term\*.obj
	del srccmd\term\*.exe
	del srccmd\tftp\*.obj
	del srccmd\tftp\*.exe
	del srccmd\tn\*.obj
	del srccmd\tn\*.exe
	del srccmd\whois\*.obj
	del srccmd\whois\*.exe
	del srccmd\rexec\*.obj
	del srccmd\rexec\*.exe
	del srccmd\red\*.obj
	del srccmd\red\*.exe
	del srccmd\smtp\*.obj
	del srccmd\smtp\*.exe
	del srccmd\pop2\*.obj
	del srccmd\pop2\*.exe
	goto exit

:die
	echo No such option "%1"
	echo Usage: "clean [all | lib | srcdev | srclib | srccmd]"
	goto end

:exit

:end

