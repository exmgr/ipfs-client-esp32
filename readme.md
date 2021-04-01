# IPFS Client ESP32 Arduino Library
ESP32 Arduino library for submitting data to IPFS.
Requires ArduinJSON.

## Usage

Initialize IPFS client object
```c++
WiFiClient wifi_client();
IPFSClient ipfs_client(wifi_client);
ipfs_client.set_node_address([ipfs node address], 5001);
```

Submit text
```c++
IPFSClient::IPFSFile ipfs_file; // Parsed return parameters

ipfs_client.add(&ipfs_file, "file.txt", "Lorem IPFSum");

// Resulting CID is in ipfs_file.hash
```

Submit binary data
```c++
IPFSClient::IPFSFile ipfs_file; // Parsed return parameters

// Open file in SPIFFS
f = SPIFFS.open("/file_in_spiffs.png", "r");
ipfs_client.add(&ipfs_file, "file.png", &f);

// Resulting CID is in ipfs_file.hash
```

Check /examples for more.
