#define PARM_CONFIG__FILE__
//#include "system.h"
#include "common.h"
#include "marvell_ops.h"


/***************************************************************
*	        �����������
*˵����  marvel_ssid       ssid����
*        marvel_mode       ����ģʽѡ��
*        marvel_key        ����          eg:   û���룺{""}    
*        wireless_ip       ģ�鱾��ip��ַ
****************************************************************/



//  adhocģʽ����
/*																						 
char marvel_ssid[32]={"wificam-adhoc"};
char marvel_mode='1';						//  1:ADHOC 0:infa 
char marvel_key[32]={"12345"};			    	//  WEPģʽ����Ҫ5��13λ 
char wireless_ip[4]={192,168,43,123};			        //  ����ip��ַ  Ϊ0 ��ʹ��DHCP

char wireless_mask[4]={255,255,255,0};
char wireless_gw[4]={192,168,43,1};
char wireless_dns[4]={202,96,134,133};
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};
 */

char marvel_mode='1';
char marvel_ssid[32]={"WIFIBOARD_ADHOC"};				//�����Ľڵ�����
char marvel_key[32]={"1234567890123"};					//������Ҫ���������
char wireless_ip[4]={192,168,10,10};					//wifi ��IP
char wireless_mask[4]={255,255,255,0};					//�������벻�ø���
char wireless_gw[4]={192,168,10,10};						//���ز��ø���
char wireless_dns[4]={202,99,96,68};                    // DNSҲ�ɲ��ظ���
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};   //mac ��ַ���Բ��ظ���

 

  /*

//   infraģʽ����
char marvel_ssid[32]={"wificamera"};
char marvel_mode='0';						//  1:ADHOC 0:infa 
char marvel_key[32]={""};			    	//  WEPģʽ����Ҫ5��13λ 
char wireless_ip[4]={0};			        //  ����ip��ַ  Ϊ0 ��ʹ��DHCP

char wireless_mask[4]={255,255,255,0};
char wireless_gw[4]={192,168,43,123};
char wireless_dns[4]={202,96,134,133};
char wireless_mac[6]={0x00,0x1D,0x2f,0x00,0x4e,0x2d};

  */




