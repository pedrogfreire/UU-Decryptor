#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "Base64Encode.c"
#include "Base64Decode.c"
//#include "Base64Custom.c"
#include "rsa_lib.c"
#include <math.h>
//#include <curl/curl.h>
#include "aes.c"
//#include "bluetooth_IHAP.c"
#include "uart.c"


#include <json-c/json.h>

#define PRINT_AGG_INFO
#define PRINT_RECEIVED_DATA
#define PRINT_LOOKUP_TABLE 	
#define PRINT_MESSAGE_TO_BE_SENT			
#define PRINT_ENCRYPT_RSA 	
#define PRINT_ENCRYPT_AES 	
#define PRINT_DECRYPT_AES 		
#define PRINT_ENCODED_BASE64
#define PRINT_DECODED_BASE64
#define PRINT_DECRYPTED_MESSAGE_RSA
#define PRINT_DATA_RECEIVED

#define MSG_BLUETOOTH_MAX_SIZE 1000

void decrypt_message(){
	char fileDataReceivedBuffer[2048];
	FILE *fileData = fopen("data.bin", "r");
	fread(fileDataReceivedBuffer, 2048, 1, fileData);
	fclose(fileData);	
	#ifdef PRINT_DATA_RECEIVED
		printf("Data Received: %s\n", fileDataReceivedBuffer);
	#endif
	
	struct json_object *parsed_data;	
	parsed_data = json_tokener_parse(fileDataReceivedBuffer);	
	
	//extract sensors list from lookup table
	struct json_object *Jinfo;
	struct json_object *JEncryption;
	struct json_object *JUserKey;
	struct json_object *JPulse;
	struct json_object *JIVEC;
	struct json_object *JDate;
	
	json_object_object_get_ex(parsed_data, "Info", &Jinfo);
	json_object_object_get_ex(parsed_data, "Encryption", &JEncryption);
	json_object_object_get_ex(parsed_data, "UserKey", &JUserKey);
	json_object_object_get_ex(parsed_data, "Pulse", &JPulse);
	json_object_object_get_ex(parsed_data, "Ivec", &JIVEC);
	json_object_object_get_ex(parsed_data, "date", &JDate);
	
	char string_Info[1000];
	char string_UserKey[1000];
	char string_Ivec[1000];
	
	FILE *fileInfo = fopen("info.bin", "r");
	fread(string_Info, 2048, 1, fileInfo);
	fclose(fileInfo);	
	
	FILE *fileUserKey = fopen("UserKey.bin", "r");
	fread(string_UserKey, 2048, 1, fileUserKey);
	fclose(fileUserKey);	
	
	FILE *fileIVEC = fopen("ivec.bin", "r");
	fread(string_Ivec, 2048, 1, fileIVEC);
	fclose(fileIVEC);	
	
	//sprintf(string_Info, "%s", json_object_to_json_string_ext(Jinfo, JSON_C_TO_STRING_SPACED));
	//sprintf(string_UserKey, "%s", json_object_to_json_string_ext(JUserKey, JSON_C_TO_STRING_SPACED));
	//sprintf(string_Ivec, "%s", json_object_to_json_string_ext(JIVEC, JSON_C_TO_STRING_SPACED));
	//sprintf(string_Info, "%s", &string_Info[1]);
	//string_Info[strlen(string_Info)-1]=0;
	//sprintf(string_Ivec, "%s", &string_Ivec[1]);
	//string_Ivec[strlen(string_Ivec)-1]=0;
	//sprintf(string_UserKey, "%s", &string_UserKey[1]);
	//string_UserKey[strlen(string_UserKey)-1]=0;
	for(int i=0; i<strlen(string_Info); i++){
		if(string_Info[i]=='=' && string_Info[i+1]=='=')string_Info[i+2]=0;
	}
	for(int i=0; i<strlen(string_Ivec); i++){
		if(string_Ivec[i]=='=' && string_Ivec[i+1]=='=')string_Ivec[i+2]=0;
	}
	for(int i=0; i<strlen(string_UserKey); i++){
		if(string_UserKey[i]=='=' && string_UserKey[i+1]=='=')string_UserKey[i+2]=0;
	}
	printf("Info: %s\n", string_Info);
	printf("UserKey: %s\n", string_UserKey);
	printf("IVEC: %s\n", string_Ivec);
	printf("Encryption: %s\n", json_object_to_json_string_ext(JEncryption, JSON_C_TO_STRING_SPACED));
	printf("Pulso: %s\n", json_object_to_json_string_ext(JPulse, JSON_C_TO_STRING_SPACED));
	printf("Date: %s\n", json_object_to_json_string_ext(JDate, JSON_C_TO_STRING_SPACED));
	
	int infoLen = strlen(string_Info);
	int UsrKeyLen = strlen(string_UserKey);
	int IVECLen = strlen(string_Ivec);
	printf("Len info: %d - Len UsrKey: %d - Len IVEC: %d \n", infoLen, UsrKeyLen, IVECLen);
	

	
	
	
	char* base64DecodedInfoOutput = NULL; //Info
	int InfoStrLen;
	base64DecodedInfoOutput = base64_decode_cust(string_Info, strlen(string_Info), &InfoStrLen);
	
	char* base64DecodedUserKeyOutput = NULL; //UserKey
	int UserKeyStrLen;
	base64DecodedUserKeyOutput = base64_decode_cust(string_UserKey, strlen(string_UserKey), &UserKeyStrLen);
	
	char* base64DecodedIVECOutput = NULL; //Ivec
	int IVECStrLen;
	base64DecodedIVECOutput = base64_decode_cust(string_Ivec, strlen(string_Ivec), &IVECStrLen);
	
	/*
	char* base64DecodedInfoOutput = NULL; //Info
	int InfoStrLen;
	base64DecodedInfoOutput = base64_decode_cust((unsigned char*)string_Info, infoLen, &InfoStrLen);
	
	char* base64DecodedUserKeyOutput = NULL; //UserKey
	int UserKeyStrLen;
	base64DecodedUserKeyOutput = base64_decode_cust((unsigned char*)string_UserKey, UsrKeyLen, &UserKeyStrLen);
	
	char* base64DecodedIVECOutput = NULL; //Ivec
	int IVECStrLen;
	base64DecodedIVECOutput = base64_decode_cust((unsigned char*)string_Ivec, IVECLen, &IVECStrLen);	
	*/
	if(!(open_private_key((const char *) ("privateKey.bin")))){
		printf("Error opening private key\n");
	}
	
	char userKeyStr[1024];
	if(!rsa_decrypt_long_message_priv(base64DecodedUserKeyOutput, userKeyStr, UserKeyStrLen)){
		printf("Error decrypting userKey RSA priv\n");
	}
	char IVECStr[1024];
	if(!rsa_decrypt_long_message_priv(base64DecodedIVECOutput, IVECStr, IVECStrLen)){
		printf("Error decrypting IVEC RSA priv\n");
	}
	
	for(int aux=0; aux<AES_USER_KEY_SIZE; aux++){
		userKey[aux] = userKeyStr[aux];
	}
	for(int aux=0; aux<AES_IVEC_SIZE; aux++){
		IV[aux] = IVECStr[aux];
	}
	
	//Decrypt aggregator info in AES
	char   AES_decrypted_message[2048];    // Encrypted message
	unsigned int aes_decrypted_message_len;		
	aes_decrypt_message(base64DecodedInfoOutput, AES_decrypted_message, InfoStrLen);
	#ifdef PRINT_DECRYPT_AES
		printf("AES decrypted result: %s\n", AES_decrypted_message);
	#endif
	
	
}
/*
void program_loop(){
		
	//open uart and check for messages
	Message_Struct received_data = check_for_new_message();
	
	#ifdef PRINT_RECEIVED_DATA
		printf("Received data: ID[%d] and Value[%f]\n", received_data.Sensor_ID, received_data.Sensor_Value);
	#endif
	
	if(received_data.Sensor_ID!=0){
		//Open JSON file with Aggregator Info
		char fileAggregatorInfoBuffer[1024];
		FILE *fileAggregatorInfo = fopen("aggregatorInfo.json", "r");
		fread(fileAggregatorInfoBuffer, 1024, 1, fileAggregatorInfo);
		fclose(fileAggregatorInfo);	
		#ifdef PRINT_AGG_INFO
			printf("Aggregator Info: %s\n", fileAggregatorInfoBuffer);
		#endif
		
		//Open JSON file with Sensors Lookup Table
		char fileSensorsLookupTableBuffer[2024];
		FILE *fileSensLookupTable = fopen("sensorsLookupTable.json", "r");
		fread(fileSensorsLookupTableBuffer, 2024, 1, fileSensLookupTable);
		fclose(fileSensLookupTable);	
		#ifdef PRINT_LOOKUP_TABLE
			printf("Aggregator Info: %s\n", fileSensorsLookupTableBuffer);
		#endif
		
		//extract json objects from the opened files
		struct json_object *parsed_json_info;
		struct json_object *parsed_json_lookup;
		parsed_json_info = json_tokener_parse(fileAggregatorInfoBuffer);	
		parsed_json_lookup = json_tokener_parse(fileSensorsLookupTableBuffer);
		
		//extract sensors list from lookup table
		struct json_object *sensors_orig;
		struct json_object *ID;
		struct json_object *sensor1;
		struct json_object *sensors_dest;
		json_object_object_get_ex(parsed_json_lookup, "sensors", &sensors_orig);
		
		//locate ID in the sensors list
		
		sensors_dest = json_object_new_array();
		int is_sensorID_found=0;
		for(int a=0; a<json_object_array_length(sensors_orig); a++){
			sensor1 = json_object_array_get_idx(sensors_orig, a);
			json_object_object_get_ex(sensor1, "ID", &ID);
			if(json_object_get_int(ID)==received_data.Sensor_ID){
				json_object_object_add(sensor1,"Value",json_object_new_int(received_data.Sensor_Value));
				json_object_object_add(sensor1,"measurementTime",json_object_new_string(get_current_time()));
				json_object_array_put_idx(sensors_dest, 0, sensor1);
				is_sensorID_found=1;
				break;
			}
		}
		if(!is_sensorID_found){	
			sensor1 = json_object_new_object();
			json_object_object_add(sensor1,"ID",json_object_new_int(received_data.Sensor_ID));
			json_object_object_add(sensor1,"UniqID",json_object_new_string("Unknown"));
			json_object_object_add(sensor1,"unit",json_object_new_string("Unknown"));
			json_object_object_add(sensor1,"Value",json_object_new_int(received_data.Sensor_Value));						
			json_object_object_add(sensor1,"measurementTime",json_object_new_string(get_current_time()));
			json_object_array_put_idx(sensors_dest, 0, sensor1);
		}
		json_object_object_add(parsed_json_info, "sensors", sensors_dest);
		
		fileAggregatorInfoBuffer[0]=0;
		sprintf(fileAggregatorInfoBuffer, "%s", json_object_to_json_string_ext(parsed_json_info, JSON_C_TO_STRING_SPACED));
		
		#ifdef PRINT_MESSAGE_TO_BE_SENT
			printf("Message to be sent: %s\n", fileAggregatorInfoBuffer);
		#endif
		//Open public key file
		if(!(open_public_key((const char *) ("publicKey.bin")))){
			printf("Error opening public key\n");
		}
		
		//Open JSON file with Configuration info
		char fileConfigBuffer[1024];
		FILE *fileConfig = fopen("config.json", "r");
		fread(fileConfigBuffer, 1024, 1, fileConfig);
		fclose(fileConfig);
		
		struct json_object *configJsonObj;
		struct json_object *destinationConfigJson;
		char *transmissionDestination = NULL;	
		configJsonObj = json_tokener_parse(fileConfigBuffer);
		json_object_object_get_ex(configJsonObj, "destination", &destinationConfigJson); 
		transmissionDestination = malloc(json_object_get_string_len(destinationConfigJson) * sizeof(char));
		transmissionDestination = (char *) json_object_get_string(destinationConfigJson);

		
		
		
		//Open JSON file with Header
		char fileHeaderBuffer[1024];
		FILE *fileHeader = fopen("header.json", "r");
		fread(fileHeaderBuffer, 1024, 1, fileHeader);
		fclose(fileHeader);	
		
		struct json_object *parsed_json_header;	
		parsed_json_header = json_tokener_parse(fileHeaderBuffer);
		
		
		
		
		//Encrypt AES UserKey and IVEC with RSA
		
		char   *rsa_encrypted_UserKey = NULL;    // UserKey
		unsigned int rsa_encrypted_UserKey_len;		
		rsa_encrypted_UserKey = malloc(((strlen(userkey)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
		if(!(rsa_encrypted_UserKey_len = rsa_encrypt_long_message_pub(userkey, rsa_encrypted_UserKey))){
			printf("RSA UserKey Encryption Error");
		}

		char   *rsa_encrypted_IVEC = NULL;    // IVEC
		unsigned int rsa_encrypted_IVEC_len;		
		rsa_encrypted_IVEC = malloc(((strlen(IV)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
		if(!(rsa_encrypted_IVEC_len = rsa_encrypt_long_message_pub(IV, rsa_encrypted_IVEC))){
			printf("RSA IVEC Encryption Error");
		}
		
		
		//Encode encrypted UserKey and IVEC in base64
		
		char* base64EncodeUserKeyOutput = NULL; //UserKey
		int base64_UserKey_len;
		if(!(base64_UserKey_len = Base64Encode((unsigned char*)rsa_encrypted_UserKey, &base64EncodeUserKeyOutput, rsa_encrypted_UserKey_len))){
			printf("Base64 UserKeyEncoding Error");
		}		
		
		char* base64EncodeIVECOutput = NULL; //IVEC
		int base64_IVEC_len;
		if(!(base64_IVEC_len = Base64Encode((unsigned char*)rsa_encrypted_IVEC, &base64EncodeIVECOutput, rsa_encrypted_IVEC_len))){
			printf("Base64 IVEC Encoding Error");
		}		
		
		json_object_object_add(parsed_json_header, "UserKey", json_object_new_string(base64EncodeUserKeyOutput));
		json_object_object_add(parsed_json_header, "Ivec", json_object_new_string(base64EncodeIVECOutput));
		
		
		//Encrypt aggregator info in AES
		char   *AES_encrypted_message = NULL;    // Encrypted message
		unsigned int aes_encrypted_message_len;		
		AES_encrypted_message = malloc(((strlen(fileAggregatorInfoBuffer)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
		aes_encrypted_message_len = aes_encrypt_message(fileAggregatorInfoBuffer, AES_encrypted_message);
		#ifdef PRINT_ENCRYPT_AES
			print_encrypt_message(AES_encrypted_message, aes_encrypted_message_len);
			printf("AES Encrypted result: %s\n", AES_encrypted_message);
		#endif
		
		
		//Decrypt aggregator info in AES
		char   *AES_decrypted_message = NULL;    // Encrypted message
		unsigned int aes_decrypted_message_len;		
		AES_decrypted_message = malloc(((strlen(fileAggregatorInfoBuffer)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
		aes_decrypt_message(AES_encrypted_message, AES_decrypted_message, aes_encrypted_message_len);
		#ifdef PRINT_DECRYPT_AES
			printf("AES decrypted result: %s\n", AES_decrypted_message);
		#endif
		
		//Encode message in base64
		char* base64EncodeOutput = NULL;
		int base64_encrypt_len;
		if(!(base64_encrypt_len = Base64Encode((unsigned char*)AES_encrypted_message, &base64EncodeOutput, aes_encrypted_message_len))){
			printf("Base64 Encoding Error");
		}		
		#ifdef PRINT_ENCODED_BASE64
			printf("Base64 encoded result %s\n\n\n", base64EncodeOutput);
		#endif
		
		json_object_object_add(parsed_json_header, "Info", json_object_new_string(base64EncodeOutput));
				
		
		char comand_sys[2000];
		sprintf(comand_sys, "%s", json_object_to_json_string_ext(parsed_json_header, JSON_C_TO_STRING_SPACED));
		//comand_sys = (char *) json_object_to_json_string_ext(parsed_json_header, JSON_C_TO_STRING_SPACED);
		printf("Result: %s\n", comand_sys);
		//sprintf(comand_sys, "'{%cencrypted%c:'%s'}'",  '"', '"', base64EncodeOutput);
		
		if(transmissionDestination[0]=='I'){
			printf("Sending to IHAP\n");
			search_bt_uuid_service();
			int msg_len = strlen(comand_sys);
			printf("Text Lenght %d\n", msg_len);
			int num_parts = ceil((double) msg_len/MSG_BLUETOOTH_MAX_SIZE);
			char msg_with_header[MSG_BLUETOOTH_MAX_SIZE];
			char temp_buffer[MSG_BLUETOOTH_MAX_SIZE+1];
			for(int i=0; i<num_parts; i++){
				msg_with_header[0] = 'P';
				msg_with_header[1] = '1' + i;
				msg_with_header[2] = 'O';
				msg_with_header[3] = '0' + num_parts;
				msg_with_header[4] = 0;
				strncpy(temp_buffer, &comand_sys[MSG_BLUETOOTH_MAX_SIZE*i], MSG_BLUETOOTH_MAX_SIZE);
				temp_buffer[MSG_BLUETOOTH_MAX_SIZE] = 0;
				printf("Part %d, message: %s\n\n", i, temp_buffer);
				strcat(msg_with_header, temp_buffer);			
				send_message_bluetooth(msg_with_header, strlen(msg_with_header));
				delay(100);
			}				
		}
		else{
			printf("Sending to Alleato\n");
			CURL *curl;
			CURLcode res;
			
			curl_global_init(CURL_GLOBAL_ALL);
			
			curl = curl_easy_init();
			
			if(curl){
				curl_easy_setopt(curl, CURLOPT_URL, "http://tk.omsorgsportal.se/Receiver/");
				
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, comand_sys);
				
				res = curl_easy_perform(curl);
				
				if(res != CURLE_OK){
					fprintf(stderr, "curl_easy_perform() returned %s\n", curl_easy_strerror(res));			
				}
				curl_easy_cleanup(curl);		
			}
			curl_global_cleanup();
		}
	}
}
*/
int main(){
	//program_loop();
	decrypt_message();
	return 0;
}