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

    IPFSClient(WiFiClient client);

    void set_node_address(String addr, uint16_t port);

    Result add(IPFSFile *file_out, String filename, String data);
    Result add(IPFSFile *file_out, String filename, File *spiffs_file);

    Result cat(String cid, String& output, int max_length = 0);
private:
    // Default constructor private
    IPFSClient(){};

    Result add_req(IPFSFile *file_out, String filename, String data, File *spiffs_file);

    String build_api_path(String path);

    String _node_addr = "";
    uint16_t _node_port = 0;

    WiFiClient _client;

    static const char API_PATH[];
};

#endif