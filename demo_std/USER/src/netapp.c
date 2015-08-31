#include "netapp.h"
#include "delay.h"
#include "lwip/tcp.h"

// typedef unsigned   char    uint8_t;    /* Unsigned 8 bit quantity         */
// /* Signed    8 bit quantity        */
// typedef unsigned   short   uint16_t;   /* Unsigned 16 bit quantity        */
// /* Signed   16 bit quantity        */
// typedef unsigned   long    uint32_t;   /* Unsigned 32 bit quantity        */


struct TcpClientCon{
    struct tcp_pcb *pcb;
    struct ip_addr ipaddr;//server ip
    long port;
    char connected;
};


static struct TcpClientCon TcpClient;

static void app_delay()
{
    int i;
    for(i=0;i<1000;i++);
}

void UDP_Receive(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port) 
{ 

    struct ip_addr ipaddr=*addr;
    printf("----udp have receive-------\n");
    if(p != NULL){
	app_delay();
        udp_sendto(upcb,p,&ipaddr,port);  /* ���յ��������ٷ��ͳ�ȥ                   */
        pbuf_free(p);                                           /*  �ͷŻ���������                      */
    } 
} 


const static char UDPData[]={"UDP_TEST\r\n"}; 

void UDP_broadcast(void) 
{ 
    struct udp_pcb *UdpPcb; 
    struct ip_addr ipaddr;
    struct pbuf *p;
    p = pbuf_alloc(PBUF_RAW,sizeof(UDPData),PBUF_RAM);
    printf("\nUDP broadcast\n");
    if(p==NULL)
        return;
    p->payload=(void *)UDPData;   ///��pbuf��ʼ��


    UdpPcb=udp_new();                                     // ����UDP ͨ�ŵĿ��ƿ�(pcb)             
    udp_bind(UdpPcb,IP_ADDR_ANY,8080);                     //  �󶨱���IP ��ַ                       
    IP4_ADDR(&ipaddr,255, 255, 255, 255);
    udp_sendto(UdpPcb,p,&ipaddr,8080);                    // ���յ��������ٷ��ͳ�ȥ                 //
    udp_sendto(UdpPcb,p,&ipaddr,8080); 
    udp_sendto(UdpPcb,p,&ipaddr,8080); 
    udp_recv(UdpPcb,UDP_Receive,NULL);                  // �������ݽ���ʱ�Ļص�����               //
} 	 


const static char TCP_TestData[]="LwIP TCP Client \r\n"; 

/*******  ����һ���ص���������TCP �ͻ�����������ӽ���ʱ������********/ 
extern const char demo[];

err_t tcp_sended(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    return 0;
}



static err_t
        tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if(err == ERR_OK && p != NULL) {
        /* Inform TCP that we have taken the data. */
        tcp_recved(pcb, p->tot_len);
	tcp_write(pcb,p->payload,p->tot_len,0);   
    }
    else if(err == ERR_OK && p == NULL) {
        tcp_close(pcb);
	TcpClient.connected=0;
    }
    pbuf_free(p);
    return ERR_OK;
}




err_t TcpCli_Connected(void *arg,struct tcp_pcb *pcb,err_t err) 
{ 
    tcp_arg(pcb,(void *)TCP_TestData);
    tcp_recv(pcb,tcp_client_recv);
    TcpClient.connected=1;
    return ERR_OK;

} 
void TCP_Client_Init(void);

static void ClientConError(void *arg, err_t err)
{	
    printf("tcp client connect error\n");
    TcpClient.connected=0;

}

void TCP_Client_Init(void) 
{ 
    printf("TCP_client_init\n");
    TcpClient.connected=1;
    IP4_ADDR(&TcpClient.ipaddr,192,168,43,100);
    TcpClient.port=8080;
    TcpClient.pcb=tcp_new();   /*  ����ͨ�ŵ�TCP ���ƿ�(Clipcb)                */
    tcp_bind(TcpClient.pcb,IP_ADDR_ANY,0);/*  �󶨱���IP ��ַ�Ͷ˿ں�*/
    tcp_err(TcpClient.pcb, ClientConError);
    tcp_connect(TcpClient.pcb,&TcpClient.ipaddr,
                TcpClient.port,TcpCli_Connected);
} 

void ReConTcpServer(void)
{
		printf("ReConTcpServer\n");
    if(!TcpClient.connected){
        TCP_Client_Init();
    }
}


extern int data_flag;
extern struct tcp_pcb *my_pcb;
static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if(err == ERR_OK && p != NULL)
    {
				/* Inform TCP that we have taken the data. */
				tcp_recved(pcb, p->tot_len); /// pcbָ���������� ��ȡ���յ����ݵĳ���
				if(strstr(p->payload,"0") > 0)                //��ʼ����
					data_flag=~data_flag;
		}
		my_pcb=pcb;
		pbuf_free(p);
    return ERR_OK;
}

err_t data_send(struct tcp_pcb *pcb,const unsigned char* addr,int data_len,int no)
{
//	struct pbuf *p;

	int cur_len=1460*3;
	int data_count=0;
// // // 	err_t err1;
	

	
//	if(p!=NULL)
	{
		tcp_write(pcb,&no,4,0);
// // // 		err1=tcp_output(pcb);
		while(data_len>0)
		{
			tcp_write(pcb,addr+data_count*cur_len,cur_len,0);
// // // 			err1=tcp_output(pcb);
// // // 			if(err1 == ERR_OK)
			{
				data_len=data_len-cur_len;
				data_count++;		
			}
		}
		tcp_output(pcb);
	}
//	pbuf_free(p);
//	if(p == NULL)
//	{
//		tcp_abort(pcb);
//		tcp_close(pcb);
//	}
	return ERR_OK;
}


static err_t
        server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    /* Tell TCP that this is the structure we wish to be passed for our callbacks. */
    tcp_arg(pcb, NULL);
    /* Tell TCP that we wish to be informed of incoming data by a call to the http_recv() function. */
    tcp_recv(pcb, server_recv);
    return ERR_OK;
}


void Init_tcp_server(void)
{
    struct tcp_pcb *pcb;
    printf("\nInit_tcp_server \n");
    pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY,8888);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, server_accept);
}




