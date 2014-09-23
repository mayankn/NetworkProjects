#ifndef ARPREQ_H
#define ARPREQ_H 1
#include <net/ethernet.h>

class ARPReq
{	public:static int getGatewayMacAddr(char* buf, char* local_mac, int* idx);
}
;
#endif
