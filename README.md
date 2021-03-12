# UU-Decryptor
Decryption code for Aggregator's data

This is the code for decryption of the data sent by the aggregator to the server via IHAP.

Compilation:
The main file for compilation is AggMain.c

Use GCC Compiler:
```linux
gcc AggMain.c -lcrypto -lssl -ljson-c -lm
```

------------------------------------------------------------------------------------
Data Input:

Create the following bin files and save them in the execution file directory.  

**data.bin** - The original content received at the server. (Only one message at the time)  
**info.bin** - Content of the "info" tag in the JSON string of data.  
**UserKey.bin** - Content of the "UserKey" tag in the JSON string of data.  
**ivec.bin** - Content of the "Ivec" tag in the JSON string of data.  

This code is yet not complete, there must be implemented the reading straight from the data.bin file without the need to breakdown the message in different files. However, until the last version of this code a problem was found witht the reading of encoded data from JSON file using the JSON-C library. The reading function includes a backslash '\' next to all forward slash '/', which causes a error in the decoding algorithm.

------------------------------------------------------------------------------------
