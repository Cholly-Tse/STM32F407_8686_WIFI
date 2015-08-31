#ifndef __NETAPP__H__
#define __NETAPP__H__

#include "lwip.h"

void httpd_init(void);
void UDP_Receive(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port) ;
void UDP_Test(void);
void UDP_broadcast(void);
err_t tcp_sended(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
err_t TcpCli_Connected(void *arg,struct tcp_pcb *pcb,err_t err); 
void TCP_Client_Init(void);
void ReConTcpServer(void);
static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
void Init_tcp_server(void);

err_t data_send(struct tcp_pcb *pcb,const unsigned char* addr,int data_len,int no);

#endif


