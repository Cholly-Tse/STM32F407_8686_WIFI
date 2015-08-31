#ifndef __LWIP_HEAD__H__
#define __LWIP_HEAD__H__

#include "lwip/memp.h"
#include "lwIP/tcp.h"
#include "lwIP/udp.h"
#include "lwIP/tcpip.h"
#include "netif/etharp.h"
#include "lwIP/dhcp.h"
#include "lwIP/dns.h"
#include "lwip/netif.h"
#include "lwip/netbios.h"
#include "netif/ethernetif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "system.h"	 




// void timer3_int(void);
void timer_thread(void);
void Init_lwIP(void);
void rest_sys_mem(void);

#endif







