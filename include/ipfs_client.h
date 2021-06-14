/******************************************************************************
* IPFS Client ESP32
* EX-MACHINA - exm.gr
* Nikos Ts.
******************************************************************************/

#ifndef IPFS_CLIENT_H
#define IPFS_CLIENT_H

#include <WiFiClient.h>
#include "SPIFFS.h"

class IPFSClient
{
public:
    /**
     * Return codes
     */
    enum Result
    {
        IPFS_CLIENT_OK,
        IPFS_CLIENT_ERROR,
        IPFS_CLIENT_CANNOT_CONNECT,
        IPFS_CLIENT_INVALID_RESPONSE
    };

    /**
     * File descriptor
     */
    struct IPFSFile
    {
        char name[255];
        char hash[50];
        uint32_t size;
    };

    /**
     * Command response as returned by IPFS node
     */
    struct IPFSResponse
    {
        int code;
        char type[20];
        char message[200];
    };

    IPFSClient(WiFiClient client);

    void set_node_address(String addr, uint16_t port);

    Result add(IPFSFile *file_out, String filename, String data);
    Result add(IPFSFile *file_out, String filename, File *spiffs_file);
    Result cat(String cid, String& output, int max_length = 0);
    Result files_cp(String from, String to);

    const IPFSResponse *get_last_response();
private:
    //
    // Functions
    //

    // Default constructor private
    IPFSClient(){};

    Result add_req(IPFSFile *file_out, String filename, String data, File *spiffs_file);

    Result post(String path, String* output = nullptr);
    void parse_last_response(String response);
    String build_api_path(String path);

    //
    // Vars
    //
    String _node_addr = "";
    uint16_t _node_port = 0;

    WiFiClient _client;
    IPFSResponse _last_response = {0};
    static const char API_PATH[];
};

#endif