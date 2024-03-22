netdev.sys: netdev.asm
	masm netdev;
	link netdev;
	exe2bin netdev.exe netdev.sys
	command /c copy netdev.sys ..\..\exe
