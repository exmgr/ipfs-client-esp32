# IPFS Client ESP32 Arduino Library

ESP32 library for interacting with IPFS.
Requires ArduinoJSON.

## Usage

Initialize IPFS client object
```c++
WiFiClient wifi_client();
IPFSClient ipfs_client(wifi_client);
ipfs_client.set_node_address([ipfs node address], 5001);
```

### IPFS 'add'

Add text file

```c++
IPFSClient::IPFSFile ipfs_file; // Parsed return parameters

ipfs_client.add(&ipfs_file, "file.txt", "Lorem IPFSum");

// Resulting CID is in ipfs_file.hash
```

Add binary file
```c++
IPFSClient::IPFSFile ipfs_file; // Parsed return parameters

// Open file in SPIFFS
f = SPIFFS.open("/file_in_spiffs.png", "r");
ipfs_client.add(&ipfs_file, "file.png", &f);

// Resulting CID is in ipfs_file.hash
```

### IPFS 'cat'

Read data from file.

```c++
IPFSClient::Result = ipfs_client.cat("[IPFS CID here]", output);
```

Check /examples for more.

### IPFS 'cp' / 'mv/

**cp** copy file from IPFS to the Mutable File System, or between MFS directories.
**mv** move file between MFS directories.

```c++
IPFSClient::Result result = ipfs_client.files_mv("/path/to/source/file", "/path/to/destination");
```

### IPFS 'stat'
Get file status

```c++
StaticJsonDocument<200> json;
IPFSClient::Result res = ipfs_client.files_stat("/path/to/file", json);

Serial.print(F("Hash: "));
Serial.println(doc["Hash"].as<char*>());
Serial.print(F("Size: "));
Serial.println(doc["Size"].as<int>(), DEC);
Serial.print(F("Type: "));
Serial.println(doc["Type"].as<char*>());