#define PARM_CONFIG__FILE__
//#include "system.h"
#include "common.h"
#include "marvell_ops.h"


/***************************************************************
*	        网络参数设置
*说明：  marvel_ssid       ssid名称
*        marvel_mode       两种模式选择
*        marvel_key        密码          eg:   没密码：{""}    
*        wireless_ip       模块本身ip地址
****************************************************************/



//  adhoc模式设置
/*																						 
char marvel_ssid[32]={"wificam-adhoc"};
char marvel_mode='1';						//  1:ADHOC 0:infa 
char marvel_key[32]={"12345"};			    	//  WEP模式下需要5或13位 
char wireless_ip[4]={192,168,43,123};			        //  本机ip地址  为0 则使用DHCP

char wireless_mask[4]={255,255,255,0};
char wireless_gw[4]={192,168,43,1};
char wireless_dns[4]={202,96,134,133};
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};
 */

char marvel_mode='1';
char marvel_ssid[32]={"WIFIBOARD_ADHOC"};				//产生的节点名字
char marvel_key[32]={"1234567890123"};					//连接需要输入的密码
char wireless_ip[4]={192,168,10,10};					//wifi 的IP
char wireless_mask[4]={255,255,255,0};					//子网掩码不用更改
char wireless_gw[4]={192,168,10,10};						//网关不用更改
char wireless_dns[4]={202,99,96,68};                    // DNS也可不必更改
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};   //mac 地址可以不必更改

 

  /*

//   infra模式设置
char marvel_ssid[32]={"wificamera"};
char marvel_mode='0';						//  1:ADHOC 0:infa 
char marvel_key[32]={""};			    	//  WEP模式下需要5或13位 
char wireless_ip[4]={0};			        //  本机ip地址  为0 则使用DHCP

char wireless_mask[4]={255,255,255,0};
char wireless_gw[4]={192,168,43,123};
char wireless_dns[4]={202,96,134,133};
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};

  */




