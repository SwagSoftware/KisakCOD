/*
	base64.c - by Joe DF (joedf@ahkscript.org)
	Released under the MIT License
	
	Revision: 2015-06-12 01:26:51
	
	Thank you for inspiration:
	http://www.codeproject.com/Tips/813146/Fast-base-functions-for-encode-decode
*/

#include <cstdint>
#include <cstdio>

//Base64 char table function - used internally for decoding
uint32_t b64_int(uint32_t ch);

// in_size : the number bytes to be encoded.
// Returns the recommended memory size to be allocated for the output buffer excluding the null byte
uint32_t b64e_size(uint32_t in_size);

// in_size : the number bytes to be decoded.
// Returns the recommended memory size to be allocated for the output buffer
uint32_t b64d_size(uint32_t in_size);

// in : buffer of "raw" binary to be encoded.
// in_len : number of bytes to be encoded.
// out : pointer to buffer with enough memory, user is responsible for memory allocation, receives null-terminated string
// returns size of output including null byte
uint32_t b64_encode(const unsigned char* in, uint32_t in_len, unsigned char* out);

// in : buffer of base64 string to be decoded.
// in_len : number of bytes to be decoded.
// out : pointer to buffer with enough memory, user is responsible for memory allocation, receives "raw" binary
// returns size of output excluding null byte
uint32_t b64_decode(const unsigned char* in, uint32_t in_len, unsigned char* out);

// file-version b64_encode
// Input : filenames
// returns size of output
uint32_t b64_encodef(char *InFile, char *OutFile);

// file-version b64_decode
// Input : filenames
// returns size of output
uint32_t b64_decodef(char *InFile, char *OutFile);