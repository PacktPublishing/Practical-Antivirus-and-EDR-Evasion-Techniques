#include<Windows.h>
#include<iostream>

int main()
{

	//prepare startupinfo & process_info structure
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };   

	CreateProcessA("C:\\Windows\\System32\\mspaint.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);

	std::cout << "Process is created with process id : " << pi.dwProcessId << std::endl;

	getchar();

	return 0;


}
