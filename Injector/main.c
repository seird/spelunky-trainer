#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "trainer/trainer.h"


static void
print_usage(char * path)
{
	printf("Usage:\n"
			"%s <process_name> <full_path_to_dll>\n",
			path);
}


static bool
inject(const char * name_process, const char * path_dll)
{
    // Get the process id
    DWORD process_id = trainer_process_id(name_process);
    if (process_id == 0) {
        return false;
    }

    // Open a handle to target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (hProcess == NULL) {
        return false;
    }

    // Allocate memory for path_dll in the target process
    LPVOID p_dll_path = VirtualAllocEx(hProcess, 0, strlen(path_dll) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (p_dll_path == NULL) {
        return false;
    }

    // Write path_dll to the allocated memory in the target process
    WriteProcessMemory(hProcess, p_dll_path, (LPVOID)path_dll, strlen(path_dll) + 1, 0);

    // Create a remote thread in the target process that runs LoadLibraryA with
    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, p_dll_path, 0, 0);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
    }

    VirtualFreeEx(hProcess, p_dll_path, 0, MEM_RELEASE);

    return true;
}


int
main(int argc, char * argv[])
{
	if (argc != 3) {
		print_usage(argv[0]);
		return 0;
	}

    if (inject(argv[1], argv[2])) {
        puts("Injection succeeded.\n");
    }
    else {
        puts("Injection failed.\n");
    }

	return 0;
}