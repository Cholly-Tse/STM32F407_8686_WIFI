#ifndef __MARVELL_OPS__H__
#define __MARVELL_OPS__H__

//#include "system.h"//ƽ̨���
#include "common.h"//Ӳ���޹ر�׼ͷ�ļ�
#include "host.h"
#include "card.h"
#include "if_sdio.h"
#include "core.h"
#include "dev.h"
#include "rxtx.h"
#include "cmd.h"
#include "assoc.h"
//#include "system.h"
#include "sdio_func.h"
#include "hostcmdcode.h"


#ifndef	 __MARVELL_OPS_FILE__//ϵͳ�ؼ���ȫ�ֱ���
extern struct lbs_private *pgmarvel_priv;
extern u8 sdio_sys_wait;
#endif

struct lbs_private * init_marvell_driver(void);
void marvel_assoc_network(struct lbs_private *priv,char *ssid,char *key,char mode);
void lbs_scan_worker(struct lbs_private *priv);
u16 lbs_rev_pkt(void);
char  lbs_hard_start_xmit(struct lbs_private *priv,struct eth_packet * tx_ethpkt);
int wpa_L2_send_pkt(u8 *buf,u16 len);



/*************��Դ����****************************************************************/
void lbs_ps_wakeup(struct lbs_private *priv); 
void lbs_ps_sleep(struct lbs_private *priv);
void init_sleep_mode(void);
void lbs_send_confirmsleep(struct lbs_private *priv);





#if 0
/**********************************************************************
����ע�Ͳ�������Ϊ�ṹ��ؼ���Ա����������Ϊ�������ݣ�������δʹ�á����ṹ��
��Ҫ���ڹ�����������������������ݡ��󲿷�������ʹ���в����漰����rx_pkt��mac
��network�������ر���Ҫʱ����ʹ�á�
***********************************************************************/
struct lbs_private {
	struct eth_packet rx_pkt;/*�������յ������ݰ����Ⱥ�������ָ��*/
	char name[DEV_NAME_LEN];/*�������������ƣ�Ŀǰδʹ��*/
	void *card;/*ָ��ö�ٵ���sdio card��������������ʹ��*/
	/* remember which channel was scanned last, != 0 if currently scanning */
	int scan_channel;/*��¼��һ���û�ָ��ɨ����ŵ��ţ�������δʹ��*/
	u8 scan_ssid[IW_ESSID_MAX_SIZE + 1];/*��¼��һ���û�ָ��ɨ����������ƣ�������δʹ��*/
	u8 scan_ssid_len;/*��¼��һ���û�ָ��ɨ����������Ƴ��ȣ�������δʹ��*/
	/** Hardware access */
	int (*hw_host_to_card) (struct lbs_private *priv, u8 type, u8 *payload, u16 nb);/*������д�����ݵĺ���ָ��*/
	//void (*reset_card) (struct lbs_private *priv);
	struct lbs_offset_value offsetvalue;/*���ڴ洢��ȡ��MAC��baseband��RF�ȼĴ�����ֵ*/
	/* Wake On LAN */
 	uint32_t wol_criteria;/****δʹ��****/
	uint8_t wol_gpio;
	uint8_t wol_gap; 
	/** Wlan adapter data structure*/
	/** STATUS variables */
	u32 fwrelease;/*********�̼���Ϣ**********/
	u32 fwcapinfo;
	/* TX packet ready to be sent... */
	int tx_pending_len;	/*�ȴ����͵����ݱ����ȣ������ڲ�ʹ��*/	/* -1 while building packet */
	/* protected by hard_start_xmit serialization */
	/** command-related variables */
	u16 seqnum;/*��ǰ��������кţ���������Ӧ�����кŶ�Ӧ*/
	 struct cmd_ctrl_node *cmd_array;/*����ڵ㻺������*/
	/** Current command */
	 struct cmd_ctrl_node *cur_cmd;/*��ǰ����ִ�е�����*/
	int cur_cmd_retcode;
	/** command Queues */
	/** Free command buffers */
	 struct list_head cmdfreeq;/*���ڹ������������ṹ������������������͹�����������*****/
	/** Pending command buffers */
	 struct list_head cmdpendingq;
	/* Events sent from hardware to driver */
	struct kfifo *event_fifo;/*ģ��̼����¼�ͨ�����ʹ��*/
	/* nickname */
	u8 nodename[16];
	int nr_retries;
	int cmd_timed_out;/****���ʱ*****/
	/** current ssid/bssid related parameters*/
	struct current_bss_params curbssparams;
	/* IW_MODE_* */
	u8 mode;/*��ǰ����ģʽ*/
	/* Scan results list */
	struct list_head network_list;
	struct list_head network_free_list;/*�������������ڴ������ɨ����*/
	struct bss_descriptor *networks;/*ɨ�赽�ĵ�ǰ��������ʹ�����������ʹ��list_for_each_entry_bssdes�����*/
	struct bss_descriptor *cur_bss;/*��ǰ������������Ϣ*/
	u16 beacon_period;/*�����beacon֡����*/
	u8 beacon_enable;
	u8 adhoccreate;/*�Ƿ񴴽���adhoc����*/
	/** capability Info used in Association, start, join */
	u16 capability;/*����֧�ֵĹ�����Ϣ*/
	/** MAC address information */
	u8 current_addr[ETH_ALEN];/*�����̼���MAC��ַ*/
	u8 multicastlist[MRVDRV_MAX_MULTICAST_LIST_SIZE][ETH_ALEN];/*�����ಥMAC��ַ*/
	u32 nr_of_multicastmacaddr;/*�����ಥ��ַ����*/
	uint16_t enablehwauto;
	uint16_t ratebitmap;/*֧�ֵ�����λͼ*/
	u8 txretrycount;
	/** NIC Operation characteristics */
	u16 mac_control;
	u32 connect_status;
	u32 mesh_connect_status;
	u16 regioncode;
	s16 txpower_cur;
	s16 txpower_min;
	s16 txpower_max;
	/** POWER MANAGEMENT AND PnP SUPPORT */
	u8 surpriseremoved;
	u16 psmode;		/* Wlan802_11PowermodeCAM=disable
				   Wlan802_11PowermodeMAX_PSP=enable */
	u32 psstate;
	u8 needtowakeup;
	struct assoc_request * pending_assoc_req;/********�ȴ�����Ĺ�������***********/
	struct assoc_request * in_progress_assoc_req;/********���ڴ���Ĺ�������***********/
	/** Encryption parameter */
	struct lbs_802_11_security secinfo;/*802.11���������Ϣ������ʹ��*/
	/** WEP keys */
	struct enc_key wep_keys[4];/***********wep����ʹ�õ�4����Կ*******/
	u16 wep_tx_keyidx;/***********wep����ʹ�õ���Կ�����*******/
	/** WPA keys */
	 struct enc_key wpa_mcast_key;/***********wpa���ܶಥ��Կ*******/
	 struct enc_key wpa_unicast_key;/***********wpa���ܵ�����Կ*******/
	/** Requested Signal Strength*/
	u16 SNR[MAX_TYPE_B][MAX_TYPE_AVG];
	u16 NF[MAX_TYPE_B][MAX_TYPE_AVG];
	u8 RSSI[MAX_TYPE_B][MAX_TYPE_AVG];
	u8 rawSNR[DEFAULT_DATA_AVG_FACTOR];
	u8 rawNF[DEFAULT_DATA_AVG_FACTOR];
	u16 nextSNRNF;
	u16 numSNRNF;
	u8 radio_on;
	/** data rate stuff */
	u8 cur_rate;
	/** RF calibration data */
#define	MAX_REGION_CHANNEL_NUM	2
	/** region channel data */
	struct region_channel region_channel[MAX_REGION_CHANNEL_NUM];
	struct region_channel universal_channel[MAX_REGION_CHANNEL_NUM];
	/** 11D and Domain Regulatory Data */
	struct lbs_802_11d_domain_reg domainreg;
	struct parsed_region_chan_11d parsed_region_chan;
	/** FSM variable for 11d support */
	u32 enable11d;
	/**	MISCELLANEOUS */
	u32 monitormode;
	u8 fw_ready;
	/* Command responses sent from the hardware to the driver */
	u8 resp_idx;
	u32 resp_len[1];/*������Ӧ�����ݳ���*/
	u8 resp_buf[1][LBS_UPLD_SIZE];/*������Ӧ�����ݻ���յ����ݰ�����ڴ�*/
	int mesh_open;
	int mesh_fw_ver;
	int infra_open;
	int mesh_autostart_enabled;
	
	u32 mac_offset;
	u32 bbp_offset;
	u32 rf_offset;
	u8 dnld_sent;

	uint16_t mesh_tlv;
	u8 mesh_ssid[IW_ESSID_MAX_SIZE + 1];
	u8 mesh_ssid_len;
	/*
 * In theory, the IE is limited to the IE length, 255,
 * but in practice 64 bytes are enough.
 */
    #define MAX_WPA_IE_LEN 64
	/** WPA Information Elements*/
	u8 wpa_ie[MAX_WPA_IE_LEN];/*wpa-psk ��psk��Կ*/
	u8 wpa_ie_len;
	u8 eapol_finish;/*wpa����epolͨ�����*/
	
};
#endif









#endif

