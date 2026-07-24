#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

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
	DWORD parentPid = FindProcessId(L"notepad.exe");
	if (!parentPid) {
		std::cerr << "[-] Could not find explorer.exe\n";
		return -1;
	}
	// Step 2: Open the process handle
	HANDLE hParent = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, parentPid);
	if (!hParent) {
		std::cerr << "[-] Failed to open parent process\n";
		return -1;
	}
	// Step 3: Set up attribute list for spoofing PPID
// Prepare extended startup info and process information structures
	STARTUPINFOEXA si = { sizeof(si)};            // STARTUPINFOEXA contains a STARTUPINFOA + lpAttributeList for extra attributes
	PROCESS_INFORMATION pi = { 0 };       // Will receive hProcess, hThread, dwProcessId, dwThreadId

	SIZE_T attrSize = 0;                  // Will receive required size for the attribute list
	

	// First call: query how much memory is needed for the attribute list.
	// This call intentionally passes NULL for the list pointer; it only returns required size in attrSize.
	InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);

	// Allocate the exact amount of memory required for the attribute list
	si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrSize);

	// Second call: actually initialize the attribute list in the allocated memory.
	// The '1' indicates we plan to add up to 1 attribute (here: parent process).
	if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize)) {
		std::cout << "InitializeProcThreadAttributeList failed , Error: " << GetLastError() << std::endl;
		return 1;
		}

	// Add the parent-process attribute to the attribute list.
	// This tells CreateProcess to use hParent as the parent process of the new process.
	if (!UpdateProcThreadAttribute(
		si.lpAttributeList,                    // attribute list to update
		0,                                     // flags (must be 0)
		PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,  // which attribute we're setting
		&hParent,                              // pointer to the data (HANDLE of parent)
		sizeof(HANDLE),                        // size of data
		NULL,                                  // optional returned buffer (not used)
		NULL                                   // optional returned size (not used)
	)) {
		std::cout << "UpdateProcThreadAttribute failed , Error: " << GetLastError() << std::endl;
		return 1;
	}
	

		// Step 4: Create the new process with spoofed parent
	char cmdLine[] = "C:\\Windows\\System32\\mspaint.exe";
	if (!CreateProcessA(
		NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW,
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
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hParent);
	//getchar();

	return 0;

}
