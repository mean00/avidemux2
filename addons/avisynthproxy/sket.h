/*



*/
#include "winsock2.h"
#include "proxytype.h"
#ifndef SKET_H
#define SKET_H
//#define DEBUG
#define MAGGIC 0xDEADBEEF


typedef struct SktHeader
{
	uint32_t cmd;
	uint32_t frame;
	uint32_t payloadLen;
	uint32_t magic;
}SktHeader;

class Sket
{
private:
	SOCKET mySocket;
	SOCKET workSocket;
	int port;
public:
	Sket(uint32_t port_number);
	~Sket(void);

	int getPort(void);
	uint8_t socketBind(void);
	uint8_t waitConnexion(void);
	uint8_t receive(uint32_t *cmd,uint32_t *frame, uint32_t *payload_size,uint8_t *payload);
	uint8_t sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload);
};


#endif
