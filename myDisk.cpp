#include <stdio.h>
#include "myDisk.h"
FILE *file;
BOOL StartDisk(LPCTSTR lpszFile)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, lpszFile, -1, NULL, 0, NULL, FALSE); //计算长度
	char *fs = new char[dwMinSize];

	WideCharToMultiByte(CP_OEMCP, NULL, lpszFile, -1, fs, dwMinSize, NULL, FALSE);
	file = fopen(fs, "rb+");
	if (file != NULL)
	{
		return 1;
	}
	else{
		return 0;
	}
	fclose(file);
}
int ReadDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead){
	int a = 0;
	
	a = fread(lpBuffer,1, nNumberOfBytesToRead, file);
	if (lpNumberOfBytesRead!=NULL)
	{
		*lpNumberOfBytesRead = ((DWORD)a);
	}
	if (a!=0)
	{
		return 1;
	}
	else{
		return 0;
	}
}
int SetOffset(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod){
	//fseek函数成功返回0
	if (fseek(file, lDistanceToMove, dwMoveMethod) != 0){
		return 0;
	}
	else{
		return 1;
	}
}
int WriteDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten){
	int a = 0;

	a = fwrite(lpBuffer, 1, nNumberOfBytesToWrite, file);
	if (lpNumberOfBytesWritten != NULL)
	{
		*lpNumberOfBytesWritten = ((DWORD)a);
	}
	if (a != 0)
	{
		return 1;
	}
	else{
		return 0;
	}
}
void ShutDisk(){
	fclose(file);
}