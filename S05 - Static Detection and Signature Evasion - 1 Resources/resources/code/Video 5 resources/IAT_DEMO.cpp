#include<Windows.h>



//step 1: create a typedef for target function
typedef INT(WINAPI* mymsgbox)(HWND, LPCSTR, LPCSTR, UINT);


int main()
{

	//step 2: load the dll file
	HMODULE usr32dll = LoadLibraryA("user32.dll");

	//step 3: load the function address from dll 
	mymsgbox newmsgbox = (mymsgbox)GetProcAddress(usr32dll, "MessageBoxA");
	
	//step 4: call the obfuscated function
	newmsgbox(NULL, "Hello Hacker", "Title", MB_OK);

	return 0;
}
