#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        std::cout << "Usage:" << std::endl
                  << argv[0] << " " << "<path to exe>" << std::endl;
        return 1;
    }

    char buf[4096];
    if (!GetFullPathName("cheat.dll", 4096, buf, NULL)) {
        std::cout << "[-] Get full path failed: " << GetLastError() << std::endl;
    }
    std::string dll = buf;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::cout << "[*] Opening target proccess" << std::endl;
    if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "[-] Failed to create process: " << GetLastError() << std::endl;
        return 1;
    }


    std::cout << "[*] Allocating memory" << std::endl;
    LPVOID vmem = VirtualAllocEx(pi.hProcess, 0, dll.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (vmem == NULL) {
        std::cout << "[-] Failed to allocate memory: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "[*] Writing dll path" << std::endl;
    DWORD wrt;
    if (WriteProcessMemory(pi.hProcess, vmem, dll.c_str(), dll.length(), (SIZE_T*)&wrt) == 0) {
        std::cout << "[-] Failed to write dll path: " << GetLastError() << std::endl;
        return 1;
    }

    FARPROC LoadLib = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    
    std::cout << "[*] Executing CreateRemoteThread" << std::endl;
    HANDLE h = CreateRemoteThread(pi.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLib, vmem, 0, 0);
    if (h == NULL) {
        std::cout << "[-] Execution of CreateRemoteThread failed: " << GetLastError() << std::endl;
        return 1;
    }

    WaitForSingleObject(h, INFINITE);
    DWORD exit;
    GetExitCodeThread(h, &exit);
    printf("[*] Dll loaded to %#x\n", exit);

    CloseHandle(h);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::cout << "[+] Injection was successsfully accomplished" << std::endl;
    return 0;
}