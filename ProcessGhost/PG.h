#pragma once
#include<Windows.h>
#include "ntddk.h"
#include <userenv.h>
#include <iostream>
#include "syscall.h"
#define PS_INHERIT_HANDLES                      4
#define RTL_USER_PROC_PARAMS_NORMALIZED 0x00000001
using namespace std;
/*
* get calc.exe path by enviroment 
* outPath the path of calc.exe
* outSize the size of path
* 
*/
BOOL get_calc_path(LPWSTR outPath, DWORD outSize) {
	ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\calc.exe", outPath, outSize);
	return true;
}
/*
* get image in memory from target exe
* filename file path of target exe
* r_size size of file path
*/

BYTE* buffer_payload(wchar_t *filename, OUT size_t &r_size) {
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (file == INVALID_HANDLE_VALUE) {
		cout << "create file fail!\n" << endl;
	}
	HANDLE mapping = CreateFileMapping(file, 0, PAGE_READONLY, 0, 0, 0);
	if (!mapping) {
		CloseHandle(file);
		return nullptr;
		cout << "create map fail!\n" << endl;
	}
	BYTE* dllRawData = (BYTE*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
	if (dllRawData == nullptr) {
		cout << "map move fail!\n" << endl;
		CloseHandle(mapping);
		CloseHandle(file);
		return nullptr;
	}
	r_size = GetFileSize(file, 0);
	BYTE* localCopyAddress = (BYTE*)VirtualAlloc(NULL, r_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memcpy(localCopyAddress, dllRawData, r_size);
	UnmapViewOfFile(dllRawData);
	CloseHandle(mapping);
	CloseHandle(file);
	return localCopyAddress;

}






















