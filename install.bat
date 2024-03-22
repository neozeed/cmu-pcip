@echo off
	echo Installing files from tar files
	mkdir lib
	tarread xv include.tar
	tarread xv srclib.tar
	tarread xv srccmd.tar
	tarread xv srcdev.tar
	tarread xv root.tar
@echo on

