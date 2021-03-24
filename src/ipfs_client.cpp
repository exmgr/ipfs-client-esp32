#include "ipfs_client.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char IPFSClient::API_PATH[] = "/api/v0";

/******************************************************************************
* Constructor, sets wifi client. Default constructor is private.
* @param    client  WiFi Client to use for requests
******************************************************************************/
IPFSClient::IPFSClient(WiFiClient client) : _client(client)
{
}

/******************************************************************************
* Set node connection data
*
* @param	addr    IPFS node address
* @param	port    IPFS node address
******************************************************************************/
void IPFSClient::set_node_address(String addr, uint16_t port)
{
	_node_addr = addr;
	_node_port = port;
}

/******************************************************************************
* Add a plain text file to IPFS
* @param	file_out	Parsed IPFS response (output)
* @param	filename	Filename to submit to IFPS
* @param	data		Data
* @return	Result struct
******************************************************************************/
IPFSClient::Result IPFSClient::add(IPFSFile *file_out, String filename, String data)
{
	return add_req(file_out, filename, data, NULL);
}

/******************************************************************************
* Add a file to IPFS from SPIFFS
* @param	file_out	Parsed IPFS response (output)
* @param	filename	Filename to submit to IPFS (not related to SPIFFS filename)
* @param	spiffs_file	Handle to an opened file in SPIFFS
* @return   Result struct
******************************************************************************/
IPFSClient::Result IPFSClient::add(IPFSFile *file_out, String filename, File *spiffs_file)
{
	return add_req(file_out, filename, "", spiffs_file);
}

/******************************************************************************
* Add text or file data to IPFS. Used by other add functions
* @param	file_out	Parsed IPFS response (output)
* @param	filename	Filename to submit to IPFS (not related to SPIFFS filename)
* @param	data		Data. Ignored if spiffs_file is set
* @param	spiffs_file	Handle to an opened file in SPIFFS
* @return	Result struct
******************************************************************************/
IPFSClient::Result IPFSClient::add_req(IPFSFile *file_out, String filename, String data, File *spiffs_file = NULL)
{
	Result ret = IPFS_CLIENT_OK;

	bool is_file = false;
	if (spiffs_file != NULL)
	{
		is_file = true;

		// TODO: check if file valid, eg. size > 0
	}

	// Attempt to connect if not connected already
	if (!_client.connected())
	{
		if (!_client.connect(_node_addr.c_str(), _node_port))
		{
			return IPFS_CLIENT_CANNOT_CONNECT;
		}
	}

	//
	// Main request headers
	//
	String boundary = "EXM-IPFSClient"; // + String(millis());
	String boundary_start = "--" + boundary + "\r\n";
	String boundary_end = "\r\n--" + boundary + "--\r\n\r\n";

	String headers_main =
		"POST " + String(API_PATH) + "/add HTTP/1.1\r\n"
		"Host: " +
		String(_node_addr) + ":" + _node_port + "\r\n"
		"User-Agent: EXM-IPFSClient/1.0\r\n"
		"Content-Type: multipart/form-data; boundary=" +
		boundary + "\r\n";
	//
	// Data part headers
	//
	String headers_file =
		"Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";

	if (is_file)
	{
		headers_file += "Content-Type: application/octet-stream\r\n\r\n";
	}
	else
	{
		headers_file += "Content-Type: text/plain\r\n\r\n";
	}

	//
	// Boundaries
	//
	int content_length = headers_file.length() + boundary_start.length() + boundary_end.length();

	if (is_file)
	{
		content_length += spiffs_file->size();
	}
	else
	{
		content_length += data.length();
	}

	headers_main += "Content-Length: " + String(content_length) + "\n\n";

	// Output request
	// Serial.println(F("Request: "));
	// Serial.print(headers_main);
	// Serial.print(boundary_start);
	// Serial.print(headers_file);
	// Serial.print(data);
	// Serial.print(boundary_end);

	// Serial.println(F("Submitting request."));

	_client.write(headers_main.c_str());
	_client.write(boundary_start.c_str());
	_client.write(headers_file.c_str());

	if (is_file)
	{
		_client.write(*spiffs_file);
	}
	else
	{
		_client.write(data.c_str());
	}

	_client.write(boundary_end.c_str());
	_client.write("\r\n\r\n");

	_client.setTimeout(5);

	//
	// Read headers
	//
	String line = "";

	bool headers = true;
	StaticJsonDocument<255> json_doc;

	int response_code = -1;

	bool failed = true;
	while (line = _client.readStringUntil('\n'))
	{
		if (headers)
		{
			// Done with headers
			if (line.length() <= 1)
			{
				headers = false;
			}
			else
			{
				if (line.startsWith(F("HTTP")))
				{
					response_code = line.substring(line.indexOf(" ")).toInt();
				}
			}
		}
		else
		{
			if (deserializeJson(json_doc, line) == DeserializationError::Ok)
			{
				JsonObject obj = json_doc.as<JsonObject>();

				if (obj["Name"].isNull() || obj["Hash"].isNull() || obj["Size"].isNull())
				{
					Serial.print(F("Invalid JSON object received."));
					ret = IPFS_CLIENT_INVALID_RESPONSE;
					break;
				}
				else
				{
					failed = false;

					// Serial.println(F("Parsed:"));
					// serializeJsonPretty(json_doc, Serial);

					if (file_out != NULL)
					{
						strncpy(file_out->name, obj["Name"].as<char *>(), sizeof(file_out->name));
						strncpy(file_out->hash, obj["Hash"].as<char *>(), sizeof(file_out->hash));
						file_out->size = obj["Size"].as<uint32_t>();
					}

					ret = IPFS_CLIENT_OK;
					break;
				}
			}
			else
			{
				ret = IPFS_CLIENT_INVALID_RESPONSE;
			}
		}
	}

	_client.stop();

	return ret;
}