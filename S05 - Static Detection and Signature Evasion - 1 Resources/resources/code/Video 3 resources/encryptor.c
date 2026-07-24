#include <stdio.h>
#include <string.h>
#include <stdint.h>

// XOR encrypt/decrypt function
void xor_encrypt_decrypt(unsigned char *data, size_t data_len, const char *key) {

   size_t key_len = strlen(key); //getting the size of key
  //start encrypting/decrypting the shellcode bytes 
   for (size_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len]; // XOR with repeating key
    }
}

// Print shellcode
void print_shellcode(unsigned char *data, size_t len) {
    printf("\"");  // start first line

    for (size_t i = 0; i < len; i++) {
        printf("\\x%02X", data[i]);
	
	//checking if current line is filled with 16 bytes
        if ((i + 1) % 16 == 0 && (i + 1) < len) {
            printf("\"\n\"");  // if its 16 then start new line
        }
    }
    printf("\"\n"); // close final string
}

int main() {
    // shellcode
    unsigned char shellcode[] = 
   //paste the shellcode here
;



    size_t shellcode_len = sizeof(shellcode); //getting size of shellcode

    const char *key = "secret";  //key

    // Encrypt
    xor_encrypt_decrypt(shellcode, shellcode_len, key);
   // print the shellcode
    printf("\nEncrypted shellcode:\n");
    print_shellcode(shellcode, shellcode_len);

    return 0;
}

