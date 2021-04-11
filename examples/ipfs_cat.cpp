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
	Serial.println(F("IPFS 'cat' example"));

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

	// Return value for client functions
	IPFSClient::Result result;

	String output;
	result = ipfs_client.cat("[put IPFS CID here]", output);

	if(result != IPFSClient::IPFS_CLIENT_OK)
	{
		print_ipfs_error(result);
	}
	else
	{
		Serial.println(F("Returned: "));
		Serial.println(output);
	}
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