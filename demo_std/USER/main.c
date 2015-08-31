#include "lwip.h"
#include "marvell_ops.h"
#include "parmcfg.h"
#include "netapp.h"

// #include "dcmi_ov2640.h"
// #include "Nvic_Exit.h"
#include "misc.h"
// #include "stm32f4xx.h"
#include "led.h"
#include "sdio_user.h"
#include "tim.h"
#include "delay.h"

char ssid_found =0;
int data_flag=0;
extern const unsigned char marvel_firmware_array[122916];
struct tcp_pcb *my_pcb;

int  main (void)
{  
    extern struct netif netif;
    struct lbs_private *priv;
    char dhcp_ok=0;
    int i=0;
	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		LED_init();
		SDIO_init();
		USART_init(115200);
// 		timer_init();
		delay_init(168);
	
    priv=init_marvell_driver();               //网卡初始化

    if(marvel_mode == '1'){
        printf("\nADHOC\n");
    }else{
        printf("\nINFRA\n");
    }


    for(i=0;i<10;i++)						  //wifi扫描
    {
        lbs_scan_worker(priv);
        if(marvel_mode == '1'){	              //ADHOC模式只扫描一次
            break;
        }

        if(ssid_found == 1)					  //找到我们要连接的ssid
        {
            printf("%s found!\n",marvel_ssid);
            break;
        }
    }



    marvel_assoc_network(priv,marvel_ssid,marvel_key,marvel_mode);//关联网络

    Init_lwIP();                              //设置本机网络信息


        if(marvel_mode =='0'){		             //infra模式
            while(1){								 //DHCP
                unsigned char *pchr;

                if((priv->connect_status==LBS_DISCONNECTED)&&marvel_mode=='0')	   //掉线则重启
                {
                    printf("!!!!!reset !\n");
//                     NVIC_SETFAULTMASK();
//                     NVIC_GenerateSystemReset();
										__enable_irq();
										NVIC_SystemReset();
                    while(1);
                }

                ethernetif_input(&netif);

                if((!dhcp_ok)&&(netif.dhcp!=NULL &&
                                netif.dhcp->offered_ip_addr.addr!=0 &&
                                netif.dhcp->offered_sn_mask.addr!=0 &&
                                netif.dhcp->offered_gw_addr.addr!=0)){

                    dhcp_ok=1;

                    printf("\nDHCP \n");
                    pchr=(unsigned char *)&netif.dhcp->offered_ip_addr.addr;
                    printf("dhcp ip=%d:%d:%d:%d\n",pchr[0],pchr[1],pchr[2],pchr[3]);
                    pchr=(unsigned char *)&netif.dhcp->offered_gw_addr.addr;
                    printf("dhcp gw=%d:%d:%d:%d\n\n",pchr[0],pchr[1],pchr[2],pchr[3]);
                    netif_set_addr(&netif, &(netif.dhcp->offered_ip_addr),
                                   &(netif.dhcp->offered_sn_mask),&(netif.dhcp->offered_gw_addr));

                    netif_set_default(&netif);
                    netif_set_up(&netif);
                    ip_init();

                    Init_tcp_server();
                    UDP_broadcast();

                }
                else if(dhcp_ok)
                    break;
            }
        } else{
#if LWIP_DHCP_SERVER
            dhcpsrv_start(&netif);               //AD-HOC 且 LWIP_DHCP_SERVER  则启动DHCP服务
#endif

            Init_tcp_server();
        }


        printf("############("__DATE__ " - " __TIME__ ")############\n");

        while(1)
				{
            if((priv->connect_status==LBS_DISCONNECTED)&&marvel_mode=='0')//AP掉线,重启
            {
                printf("!!!!!reset !\n");
//                 NVIC_SETFAULTMASK();
//                 NVIC_GenerateSystemReset();
								__enable_irq();
								NVIC_SystemReset();
                while(1);
            }

            ethernetif_input(&netif);
						timer_thread();
						if(data_flag!=0&&my_pcb->snd_buf>1460*39+4)
							data_send(my_pcb,marvel_firmware_array,1460*39,0);
						

        }
    }





