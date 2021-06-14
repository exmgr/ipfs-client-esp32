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

/******************************************************************************
* Helper function to build paths to HTTP api functions
* @param	path	Path to API function
* @return	Result API path URL
******************************************************************************/
String IPFSClient::build_api_path(String path)
{
	return _node_addr + ":" + _node_port + API_PATH + path;
}

/******************************************************************************
* Get contents of IPFS file
* @param	file_out	Parsed IPFS response (output)
* @param	filename	Filename to submit to IPFS (not related to SPIFFS filename)
* @param	data		Data. Ignored if spiffs_file is set
* @param	spiffs_file	Handle to an opened file in SPIFFS
* @return	Result struct
******************************************************************************/
IPFSClient::Result IPFSClient::cat(String cid, String& output, int max_length)
{
	HTTPClient http_client;

	// Build path and append required arguments
	String path = "/cat?arg=" + cid;
	if(max_length > 0)
	{
		path += "&length=" + String(max_length);
	}

	String full_path = build_api_path(path);

	return post(full_path, &output);
}

/******************************************************************************
 * Copy file from IPFS/MFS to MFS
 * Equivalent to HTTP /files/cp
 * @param	from	Source path
 * @param	to		Destination path
 * @return	Result struct
 ******************************************************************************/
IPFSClient::Result IPFSClient::files_cp(String from, String to)
{
	HTTPClient http_client;

	// Prepare path/params
	String path = "/files/cp?arg=" + from + "&arg=" + to;
	String full_path = build_api_path(path);

	return post(full_path);
}

/******************************************************************************
 * Parse response into the last response object 
 * @param	response	Response received from request
 ******************************************************************************/
void IPFSClient::parse_last_response(String response)
{
	StaticJsonDocument<255> json_doc;

	if(deserializeJson(json_doc, response) == DeserializationError::Ok)
	{
		_last_response.code = json_doc["Code"].as<int>();
		strncpy(_last_response.message, json_doc["Message"], sizeof(_last_response.message));
		strncpy(_last_response.type, json_doc["Type"], sizeof(_last_response.type));
	}
	else
	{
		memset(&_last_response, 0, sizeof(_last_response));
	}
}

/**
 * Get response object returned from the last executed command
 * @return	Ptr to last response struct
 */
const IPFSClient::IPFSResponse* IPFSClient::get_last_response()
{
	return &_last_response;
}

/******************************************************************************
 * Helper function for submitting command requests.
 * @param	Path to use along with querystring of params
 * @return 	Result struct
 ******************************************************************************/
IPFSClient::Result IPFSClient::post(String path, String* output)
{
	HTTPClient http_client;

	Serial.print(F("POST to: "));
	Serial.println(path);

	// Do req
	if(http_client.begin(path) == false)
	{
		return IPFS_CLIENT_CANNOT_CONNECT;
	}

	int response_code = http_client.POST("");

	String response = http_client.getString();

	if(output != nullptr)
		*output = response;

	if(response_code != 200)
		parse_last_response(response);

	// Serial.print(F("Response code: "));
	// Serial.println(response_code, DEC);
	// Serial.print(F("Response"));
	// Serial.println(response);

	if (response_code == 200)
	{
		return IPFS_CLIENT_OK;
	}
	else if (response_code == 500)
	{
		return IPFS_CLIENT_ERROR;
	}
	else
	{
		return IPFS_CLIENT_INVALID_RESPONSE;
	}
}