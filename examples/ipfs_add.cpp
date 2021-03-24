#include <Arduino.h>
#include <WiFi.h>
#include "ipfs_client.h"
#include <SPIFFS.h>

const char WIFI_SSID[] = "";
const char WIFI_PASS[] = "";

const char NODE_ADDR[] = "";
const int NODE_PORT = 5001;

void print_ipfs_error(IPFSClient::Result result);

void setup()
{
	Serial.begin(115200);
	Serial.println(F("IPFS add example"));

	//
	// Start WiFi
	//

	WiFiClient wifi_client;

	WiFi.begin(WIFI_SSID, WIFI_PASS);
	Serial.println("Connecting...");
	while (WiFi.status() != WL_CONNECTED)
	{}
	Serial.println("WiFi connected.");

	// Init IPFS client object
	IPFSClient ipfs_client(wifi_client);
	ipfs_client.set_node_address(NODE_ADDR, NODE_PORT);

	// Will hold response info from IPFS
	IPFSClient::IPFSFile ipfs_file = {0};

	// Return value for client functions
	IPFSClient::Result result;

	//
	// Add plain text to IPFS
	//
	result = ipfs_client.add(&ipfs_file, "file.txt", "Lorem IPFSum");

	if(result == IPFSClient::IPFS_CLIENT_OK)
	{
		Serial.print(F("Text added to IPFS. CID: "));
		Serial.println(ipfs_file.hash);
	}
	else
	{
		Serial.println(F("Text could not be added to IPFS."));
		print_ipfs_error(result);
	}

	//
	// Add file from SPIFFS from IPFS
	//

	// Lets create a dummy file in SPIFFS first
	SPIFFS.begin(true); // true to format SPIFFS partition in case it hasn't been created yet

	File f = SPIFFS.open("/file_in_spiffs.txt", "w");
	if(!f)
	{
		Serial.println(F("Could create file in SPIFFS."));
		return;
	}

	String contents =
		"Lorem ipfsum dolor sit amet, consectetur adipiscing elit, "
		"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

	f.write((uint8_t*)contents.c_str(), contents.length());
	f.close();

	//
	// Now that we have a dummy file lets add it to IPFS
	//
	f = SPIFFS.open("/file_in_spiffs.txt", "r");
	if(!f)
	{
		Serial.println(F("Could not open file."));
		return;
	}
	
	result = ipfs_client.add(&ipfs_file, "file_from_spiffs.txt", &f);

	if(result == IPFSClient::IPFS_CLIENT_OK)
	{
		Serial.print(F("File added to IPFS. CID: "));
		Serial.println(ipfs_file.hash);
	}
	else
	{
		Serial.println(F("File could not be added to IPFS."));
		print_ipfs_error(result);
	}

	f.close();
}

/**
* Print error
*/
void print_ipfs_error(IPFSClient::Result result)
{
	switch(result)
	{
	case IPFSClient::Result::IPFS_CLIENT_CANNOT_CONNECT:
		Serial.println(F("Could not connect to IPFS."));
		break;
	case IPFSClient::Result::IPFS_CLIENT_INVALID_RESPONSE:
		Serial.println(F("Invalid response received."));
		break;
	}
}

void loop()
{
}