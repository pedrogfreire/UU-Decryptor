//Decodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include "Base64Custom.c"

int calcDecodeLength(const char* b64input) { //Calculates the length of a decoded base64 string
  int len = strlen(b64input);
  int padding = 0;

  if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
    padding = 2;
  else if (b64input[len-1] == '=') //last char is =
    padding = 1;

  return (int)len*0.75 - padding;
}


int Base64Decode(char* b64message, char** buffer) { //Decodes a base64 encoded string
  BIO *bio, *b64;
  int decodeLen = calcDecodeLength(b64message),
      len = 0;
  *buffer = (char*)malloc(decodeLen+1);
  FILE* stream = fmemopen(b64message, strlen(b64message), "r");
  //FILE* stream = fmemopen(b64message,message_len, "r");

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
  len = BIO_read(bio, *buffer, strlen(b64message));//strlen(b64message));
    //Can test here if len == decodeLen - if not, then return an error
  (*buffer)[len] = '\0';

  //printf("Base64 len: %d\n", len);
  //printf("Base64 decodeLen: %d\n", decodeLen);
  BIO_free_all(bio);
  fclose(stream);

  return (len); //success
}

int Base64DecodeLong(char* b64message, char** buffer) { //Decodes a base64 encoded string
	int len = strlen(b64message);
	int returnLen=0;
	int outputLenTot=0;
	char string_aux[64];	
	int num_t = len/64;
	char* buff = NULL;
	int decodeLen = calcDecodeLength(b64message);
	if(len%64)num_t++;
	*buffer = (char*)malloc(len+1);
	for (int i =0; i<num_t; i++){
		sprintf(string_aux, "%.64s", &b64message[i*64]);
		buff = NULL;
		buff = base64_decode_cust(string_aux, strlen(string_aux), &returnLen);		
		for(int a=0; a<returnLen; a++)(*buffer)[outputLenTot++]=buff[a];
		(*buffer)[outputLenTot]=0;	
	}
	return outputLenTot;
}