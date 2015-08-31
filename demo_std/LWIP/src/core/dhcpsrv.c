/***********
this code is for lwip dhcp server
**********/
#include "lwip/opt.h"
#if (LWIP_DHCP&&LWIP_DHCP_SERVER)
#include "lwip/stats.h"
#include "lwip/mem.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/sys.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/dns.h"
#include "netif/etharp.h"

#include <string.h>
//just for debug
#include "stdio.h"
#include "common.h"
#define debug_printf printf


static void list_init_head(struct dhcp_client *head);
static void list_add_tail(struct dhcp_client *head,struct dhcp_client *node);
static void list_del_node(struct dhcp_client *node);
static int search_bitmap(u8_t *bitmap,int size);
static u32_t new_ipaddr(struct netif *netif);
static struct dhcp_client * find_vaild_client(struct netif *netif,u8_t *hwaddr);
static struct dhcp_client * add_dhcp_client(struct netif *netif,u8_t *hwaddr,u8_t *name);
static void dhcp_free_request(struct dhcp_srv *dhcp);
static err_t dhcp_unfold_request(struct dhcp_srv *dhcp);
static u8_t *dhcp_get_option_ptr(struct dhcp_srv *dhcp, u8_t option_type);
static err_t dhcp_create_replay(struct netif *netif);
static void dhcp_option(struct dhcp_srv *dhcp, u8_t option_type, u8_t option_len);
static void dhcp_option_byte(struct dhcp_srv *dhcp, u8_t value);
static void dhcp_option_long(struct dhcp_srv *dhcp, u32_t value);//this is network byte sequence
static void dhcp_option_trailer(struct dhcp_srv *dhcp);
static void dhcp_delete_request(struct netif *netif);
static int dhcp_replay_ask(struct netif *netif,int ask);
static int dhcp_relay_offer(struct netif *netif);
static u8_t dhcp_get_option_byte(u8_t *ptr);
static u32_t dhcp_get_option_long(u8_t *ptr);
static void dhcpsrv_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t dhcpsrv_start(struct netif *netif);

#define list_for_each(pos,head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos,n,head) \
	for(pos=(head)->next,n=pos->next;pos!=head;pos=n,n=pos->next)

static void list_init_head(struct dhcp_client *head)
{
	head->pre=head;
	head->next=head;
}

static void list_add_tail(struct dhcp_client *head,struct dhcp_client *node)
{
	
	struct dhcp_client *pre=head->pre;
	pre->next=node;
	node->pre=pre;
	node->next=head;
	head->pre=node;
}


static void list_del_node(struct dhcp_client *node)
{
	struct dhcp_client *pre=node->pre;
	struct dhcp_client *next=node->pre;
	pre->next=next;
	next->pre=pre;
	node->pre=node->next=node;
}

/*search the bitmap for a unused bit(bit=0),if find a empty bit ,return index and set 
the bit,else return -1*/
static int search_bitmap(u8_t *bitmap,int size)
{
	int i=0,index=0;
	for(index=0;index<size;index++,bitmap++){
		i=0;
		while(i<8){
			if((*bitmap)&(1<<i++))//invailed
				continue;
			*bitmap=(*bitmap)|(1<<(--i));//set the bit map
			return ((index*8)+i);
		}
	}
	return -1;
}
/*
static int check_bitmap(u8_t *bitmap,int bit)
{
	
	int i=bit%8,index=bit/8;
	if(BIT_MAP_SIZE>index)//invaild:1 vaild:0
		return ((bitmap[index]&(1<<i))&&1);
	else
		return 1;
}
*/
static void clear_bitmap(u8_t *bitmap,int bit)
{
	
	int i=bit%8,index=bit/8;
	if(BIT_MAP_SIZE>index)//invaild:1 vaild:0
		bitmap[index]=(bitmap[index]&(~(1<<i)));
}



/*create a new ip address for client .if failed return 0,else return a vailed ip address
ip address is all Network byte sequence,so this function return a network byte sequence ip address*/
static u32_t new_ipaddr(struct netif *netif)
{
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	int ret;
	u32_t ipaddr;
	u32_t netmask=ntohl(netif->netmask.addr);
	ret=search_bitmap(dhcp->bitmap,BIT_MAP_SIZE);
	if((ret<0)||((ret+DHCP_ADDR_START)>DHCP_ADDR_END)){//invaild bit 
	  
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("new_ipaddr(): allocated ip bitmap failed!"));
		return 0;
	}
	/*#define IP4_ADDR(ipaddr, a,b,c,d)  (ipaddr)->addr = htonl(((u32_t)((a) & 0xff) << 24) | \
							     ((u32_t)((b) & 0xff) << 16) | ((u32_t)((c) & 0xff) << 8) | \
							     (u32_t)((d) & 0xff))*/
	ipaddr=(ntohl(netif->ip_addr.addr))&netmask;//acording the netmask assigned the host address
	ipaddr+=(ret+DHCP_ADDR_START);
	return htonl(ipaddr);
}

/*find alive client in the client pool by hardware address*/
static struct dhcp_client * 
find_vaild_client(struct netif *netif,u8_t *hwaddr)
{
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	struct dhcp_client *client;
	list_for_each(client,&dhcp->head_node){
		if(!memcmp(hwaddr,client->hwaddr,DHCP_CHADDR_LEN))//mach
			return client;
	}
	return NULL;
}




/*add a new client to dhcp server pool,this fun return a new client address*/
static struct dhcp_client * 
add_dhcp_client(struct netif *netif,u8_t *hwaddr,u8_t *name)
{	
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	struct dhcp_client *client;
	u32_t newip;
    client = mem_malloc(sizeof(struct dhcp_client ));
	if(client==NULL){		
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("add_dhcp_client(): allocated client failed!"));
		return NULL;
	}
	memset(client,0,sizeof(struct dhcp_client));
	list_add_tail(&dhcp->head_node,client);//list the client into server pool
	newip=new_ipaddr(netif);//network byte sequence
	if(!newip){
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("add_dhcp_client(): allocated new ip address failed!"));
		mem_free(client);
		return NULL;
	}
	client->ipaddr.addr=newip;//network byte sequence
	memcpy(client->hwaddr,hwaddr,DHCP_CHADDR_LEN);
	memcpy(client->name,name,DHCP_SNAME_LEN);
	return client;
}

static void del_dhcp_client(struct netif *netif,struct dhcp_client *client)
{	
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	u32_t ipaddr,netmask=ntohl(netif->netmask.addr);
	int ret;
	if(!client)
		return ;
	ipaddr=ntohl(client->ipaddr.addr);
	ret=(ipaddr&(~netmask))-DHCP_ADDR_START;
	clear_bitmap(dhcp->bitmap,ret);	//realse ip
	list_del_node(client);
	mem_free((void *)client);
}


/**
 * Free the incoming DHCP message including contiguous copy of
 * its DHCP options.
 *
 */
static void dhcp_free_request(struct dhcp_srv *dhcp)
{
  if (dhcp->msg_in != NULL) {
    mem_free((void *)dhcp->msg_in);
    dhcp->msg_in = NULL;
  }
  if (dhcp->options_in) {
    mem_free((void *)dhcp->options_in);
    dhcp->options_in = NULL;
    dhcp->options_in_len = 0;
  }
  LWIP_DEBUGF(DHCP_DEBUG, ("dhcp_free_reply(): free'd\n"));
}


/**
 * Extract the DHCP message and the DHCP options.
 *
 * Extract the DHCP message and the DHCP options, each into a contiguous
 * piece of memory. As a DHCP message is variable sized by its options,
 * and also allows overriding some fields for options, the easy approach
 * is to first unfold the options into a conitguous piece of memory, and
 * use that further on.
 *
 */
static err_t
dhcp_unfold_request(struct dhcp_srv *dhcp)
{
  u16_t ret;
  LWIP_ERROR("dhcp != NULL", (dhcp != NULL), return ERR_ARG;);
  LWIP_ERROR("dhcp->p != NULL", (dhcp->p != NULL), return ERR_VAL;);
  /* free any left-overs from previous unfolds */
  dhcp_free_request(dhcp);
  /* options present? */
  if (dhcp->p->tot_len > (sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN)) {
    dhcp->options_in_len = dhcp->p->tot_len - (sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN);
    dhcp->options_in = mem_malloc(dhcp->options_in_len);
    if (dhcp->options_in == NULL) {
      LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("dhcp_unfold_reply(): could not allocate dhcp->options\n"));
      return ERR_MEM;
    }
  }
  dhcp->msg_in = mem_malloc(sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN);
  if (dhcp->msg_in == NULL) {
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("dhcp_unfold_reply(): could not allocate dhcp->msg_in\n"));
    mem_free((void *)dhcp->options_in);
    dhcp->options_in = NULL;
    return ERR_MEM;
  }

  /** copy the DHCP message without options */
  ret = pbuf_copy_partial(dhcp->p, dhcp->msg_in, sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN, 0);
  LWIP_ASSERT("ret == sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN", ret == sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN);
  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_unfold_reply(): copied %"U16_F" bytes into dhcp->msg_in[]\n",
     sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN));

  if (dhcp->options_in != NULL) {
    /** copy the DHCP options */
    ret = pbuf_copy_partial(dhcp->p, dhcp->options_in, dhcp->options_in_len, sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN);
    LWIP_ASSERT("ret == dhcp->options_in_len", ret == dhcp->options_in_len);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_unfold_reply(): copied %"U16_F" bytes to dhcp->options_in[]\n",
      dhcp->options_in_len));
  }
  LWIP_UNUSED_ARG(ret);
  return ERR_OK;
}

/**
 * Find the offset of a DHCP option inside the DHCP message.
 *
 * @param dhcp DHCP client
 * @param option_type
 *
 * @return a byte offset into the UDP message where the option was found, or
 * zero if the given option was not found.
 */
static u8_t *dhcp_get_option_ptr(struct dhcp_srv *dhcp, u8_t option_type)
{
  u8_t overload = DHCP_OVERLOAD_NONE;

  /* options available? */
  if ((dhcp->options_in != NULL) && (dhcp->options_in_len > 0)) {
    /* start with options field */
    u8_t *options = (u8_t *)dhcp->options_in;
    u16_t offset = 0;
    /* at least 1 byte to read and no end marker, then at least 3 bytes to read? */
    while ((offset < dhcp->options_in_len) && (options[offset] != DHCP_OPTION_END)) {
      /* LWIP_DEBUGF(DHCP_DEBUG, ("msg_offset=%"U16_F", q->len=%"U16_F, msg_offset, q->len)); */
      /* are the sname and/or file field overloaded with options? */
      if (options[offset] == DHCP_OPTION_OVERLOAD) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("overloaded message detected\n"));
        /* skip option type and length */
        offset += 2;
        overload = options[offset++];
      }
      /* requested option found */
      else if (options[offset] == option_type) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("option found at offset %"U16_F" in options\n", offset));
        return &options[offset];
      /* skip option */
      } else {
         LWIP_DEBUGF(DHCP_DEBUG, ("skipping option %"U16_F" in options\n", options[offset]));
        /* skip option type */
        offset++;
        /* skip option length, and then length bytes */
        offset += 1 + options[offset];
      }
    }
    /* is this an overloaded message? */
    if (overload != DHCP_OVERLOAD_NONE) {
      u16_t field_len;
      if (overload == DHCP_OVERLOAD_FILE) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("overloaded file field\n"));
        options = (u8_t *)&dhcp->msg_in->file;
        field_len = DHCP_FILE_LEN;
      } else if (overload == DHCP_OVERLOAD_SNAME) {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("overloaded sname field\n"));
        options = (u8_t *)&dhcp->msg_in->sname;
        field_len = DHCP_SNAME_LEN;
      /* TODO: check if else if () is necessary */
      } else {
        LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("overloaded sname and file field\n"));
        options = (u8_t *)&dhcp->msg_in->sname;
        field_len = DHCP_FILE_LEN + DHCP_SNAME_LEN;
      }
      offset = 0;

      /* at least 1 byte to read and no end marker */
      while ((offset < field_len) && (options[offset] != DHCP_OPTION_END)) {
        if (options[offset] == option_type) {
           LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("option found at offset=%"U16_F"\n", offset));
          return &options[offset];
        /* skip option */
        } else {
          LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("skipping option %"U16_F"\n", options[offset]));
          /* skip option type */
          offset++;
          offset += 1 + options[offset];
        }
      }
    }
  }
  return NULL;
}

/**
 * Create a DHCP request, fill in common headers
 *
 * @param netif the netif under DHCP control
 */
static err_t
dhcp_create_replay(struct netif *netif)
{
  struct dhcp_srv *dhcp=netif->dhcpsrv;
  struct dhcp_client *client=dhcp->cur_client;
  u16_t i;
 
  LWIP_ERROR("dhcp_create_request: netif != NULL", (netif != NULL), return ERR_ARG;);
  LWIP_ERROR("dhcp_create_request: dhcp != NULL", (dhcp != NULL), return ERR_VAL;);
  LWIP_ASSERT("dhcp_create_request: dhcp->p_out == NULL", dhcp->p_out == NULL);
  LWIP_ASSERT("dhcp_create_request: dhcp->msg_out == NULL", dhcp->msg_out == NULL);
  dhcp->p_out = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcp_msg), PBUF_RAM);
  if (dhcp->p_out == NULL) {
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("dhcp_create_request(): could not allocate pbuf\n"));
    return ERR_MEM;
  }
  LWIP_ASSERT("dhcp_create_request: check that first pbuf can hold struct dhcp_msg",
           (dhcp->p_out->len >= sizeof(struct dhcp_msg)));


  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2,
              ("transaction id xid(%"X32_F")\n", dhcp->xid));

  dhcp->msg_out = (struct dhcp_msg *)dhcp->p_out->payload;

  dhcp->msg_out->op = DHCP_BOOTREPLY;  ///boot replay message
  /* TODO: make link layer independent */
  dhcp->msg_out->htype = DHCP_HTYPE_ETH;  ///ethernet hardvare type
  /* TODO: make link layer independent */
  dhcp->msg_out->hlen = DHCP_HLEN_ETH;	   ///ethernet hardware adress length
  dhcp->msg_out->hops = 0;
  dhcp->msg_out->xid = htonl(dhcp->xid);
  dhcp->msg_out->secs = 0;
  dhcp->msg_out->flags =htons(0x8000);
  dhcp->msg_out->ciaddr.addr = 0;
  /*if (dhcp->state==DHCP_BOUND || dhcp->state==DHCP_RENEWING || dhcp->state==DHCP_REBINDING) {
    dhcp->msg_out->ciaddr.addr = netif->ip_addr.addr;
  }*/
  dhcp->msg_out->yiaddr.addr =client->ipaddr.addr;//network byte sequence
  dhcp->msg_out->siaddr.addr = 0;
  dhcp->msg_out->giaddr.addr = 0;
  for (i = 0; i < DHCP_CHADDR_LEN; i++) {
    /* copy netif hardware address, pad with zeroes */
    dhcp->msg_out->chaddr[i] = (i < netif->hwaddr_len) ? client->hwaddr[i] : 0/* pad byte*/;
  }
  for (i = 0; i < DHCP_SNAME_LEN; i++) {
    dhcp->msg_out->sname[i] = 0;
  }
  for (i = 0; i < DHCP_FILE_LEN; i++) {
    dhcp->msg_out->file[i] = 0;
  }
  dhcp->msg_out->cookie = htonl(0x63825363UL);	//option前4个字段固定，为了和bootp兼容
  dhcp->options_out_len = 0;
  /* fill options field with an incrementing array (for debugging purposes) */
  for (i = 0; i < DHCP_OPTIONS_LEN; i++) {	  /// 将规定的option 发送过去
    dhcp->msg_out->options[i] = (u8_t)i; /* for debugging only, no matter if truncated */
  }
  return ERR_OK;
}
/*
 * Concatenate an option type and length field to the outgoing
 * DHCP message.
 *连接  option type   和  length field   给  输出的dhcp messg
 */
static void
dhcp_option(struct dhcp_srv *dhcp, u8_t option_type, u8_t option_len)
{
 
  LWIP_ASSERT("dhcp_option: dhcp->options_out_len + 2 + option_len <= DHCP_OPTIONS_LEN", dhcp->options_out_len + 2U + option_len <= DHCP_OPTIONS_LEN);
  dhcp->msg_out->options[dhcp->options_out_len++] = option_type;
  dhcp->msg_out->options[dhcp->options_out_len++] = option_len;
}
/*
 * Concatenate a single byte to the outgoing DHCP message.
 *
 */
static void
dhcp_option_byte(struct dhcp_srv *dhcp, u8_t value)
{
  LWIP_ASSERT("dhcp_option_byte: dhcp->options_out_len < DHCP_OPTIONS_LEN", dhcp->options_out_len < DHCP_OPTIONS_LEN);
  dhcp->msg_out->options[dhcp->options_out_len++] = value;
}
/*
static void
dhcp_option_short(struct dhcp_srv *dhcp, u16_t value)
{
  LWIP_ASSERT("dhcp_option_short: dhcp->options_out_len + 2 <= DHCP_OPTIONS_LEN", dhcp->options_out_len + 2U <= DHCP_OPTIONS_LEN);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t)((value & 0xff00U) >> 8);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t) (value & 0x00ffU);
}
*/
static void
dhcp_option_long(struct dhcp_srv *dhcp, u32_t value)//this is network byte sequence
{
  LWIP_ASSERT("dhcp_option_long: dhcp->options_out_len + 4 <= DHCP_OPTIONS_LEN", dhcp->options_out_len + 4U <= DHCP_OPTIONS_LEN);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t)((value & 0xff000000UL) >> 24);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t)((value & 0x00ff0000UL) >> 16);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t)((value & 0x0000ff00UL) >> 8);
  dhcp->msg_out->options[dhcp->options_out_len++] = (u8_t)((value & 0x000000ffUL));
}
/**
 * Add a DHCP message trailer
 *
 * Adds the END option to the DHCP message, and if
 * necessary, up to three padding bytes.
 *
 * @param dhcp DHCP state structure
 */
static void
dhcp_option_trailer(struct dhcp_srv *dhcp)
{
  LWIP_ERROR("dhcp_option_trailer: dhcp != NULL", (dhcp != NULL), return;);
  LWIP_ASSERT("dhcp_option_trailer: dhcp->msg_out != NULL\n", dhcp->msg_out != NULL);
  LWIP_ASSERT("dhcp_option_trailer: dhcp->options_out_len < DHCP_OPTIONS_LEN\n", dhcp->options_out_len < DHCP_OPTIONS_LEN);
  dhcp->msg_out->options[dhcp->options_out_len++] = DHCP_OPTION_END;
  /* packet is too small, or not 4 byte aligned? */
  while ((dhcp->options_out_len < DHCP_MIN_OPTIONS_LEN) || (dhcp->options_out_len & 3)) {
    /* LWIP_DEBUGF(DHCP_DEBUG,("dhcp_option_trailer:dhcp->options_out_len=%"U16_F", DHCP_OPTIONS_LEN=%"U16_F, dhcp->options_out_len, DHCP_OPTIONS_LEN)); */
    LWIP_ASSERT("dhcp_option_trailer: dhcp->options_out_len < DHCP_OPTIONS_LEN\n", dhcp->options_out_len < DHCP_OPTIONS_LEN);
    /* add a fill/padding byte */
    dhcp->msg_out->options[dhcp->options_out_len++] = 0;
  }
}
/**
 * Free previously allocated memory used to send a DHCP request.
 *
 * @param netif the netif under DHCP control
 */
static void
dhcp_delete_request(struct netif *netif)
{
  struct dhcp_srv *dhcp;
  LWIP_ERROR("dhcp_delete_request: netif != NULL", (netif != NULL), return;);
  dhcp = netif->dhcpsrv;
  LWIP_ERROR("dhcp_delete_request: dhcp != NULL", (dhcp != NULL), return;);
  LWIP_ASSERT("dhcp_delete_request: dhcp->p_out != NULL", dhcp->p_out != NULL);
  LWIP_ASSERT("dhcp_delete_request: dhcp->msg_out != NULL", dhcp->msg_out != NULL);
 
  if (dhcp->p_out != NULL) {
   
    pbuf_free(dhcp->p_out);
  }
  dhcp->p_out = NULL;
  dhcp->msg_out = NULL;
}

static int dhcp_replay_ask(struct netif *netif,int ask)
{
	int ret;
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	struct dhcp_client *client=dhcp->cur_client;
	struct ip_addr *addr=&client->ipaddr;
	ret=dhcp_create_replay(netif);
  	if (ret == ERR_OK) {
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_nask: making replay\n"));
    dhcp_option(dhcp, DHCP_OPTION_MESSAGE_TYPE, DHCP_OPTION_MESSAGE_TYPE_LEN);
    if(!ask)
		dhcp_option_byte(dhcp, DHCP_NAK);
    else{
	   
		debug_printf("assigned ip %d,%d,%d,%d from DHCP server \n",
		(u16_t)(ntohl(addr->addr) >> 24 & 0xff), (u16_t)(ntohl(addr->addr) >> 16 & 0xff),
		(u16_t)(ntohl(addr->addr) >>  8 & 0xff), (u16_t)(ntohl(addr->addr) & 0xff));
	
		dhcp_option_byte(dhcp, DHCP_ACK);	 ////server 发送 ask 信号
    }
    dhcp_option(dhcp, DHCP_OPTION_SERVER_ID, DHCP_OPTION_SERVER_ID_LEN);//server id
    dhcp_option_long(dhcp,ntohl(netif->ip_addr.addr));

    dhcp_option(dhcp, DHCP_OPTION_LEASE_TIME, DHCP_OPTION_LEASE_TIME_SIZE);	 ///租约时间 
    dhcp_option_long(dhcp,ntohl(0x803a0900));

    dhcp_option(dhcp, DHCP_OPTION_SUBNET_MASK, 4/*num options*/);//subnetmask
	dhcp_option_long(dhcp,ntohl(netif->netmask.addr));

	dhcp_option(dhcp, DHCP_OPTION_ROUTER, 4/*num options*/);//router
	dhcp_option_long(dhcp,ntohl(netif->gw.addr));

	
	dhcp_option(dhcp, DHCP_OPTION_DNS_SERVER, 8/*num options*/);//dns
	dhcp_option_long(dhcp,ntohl(dns_getserver(0).addr));
	dhcp_option_long(dhcp,ntohl(dns_getserver(1).addr));
	
    dhcp_option_trailer(dhcp);

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: realloc()ing\n"));
    pbuf_realloc(dhcp->p_out, sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN + dhcp->options_out_len);

    udp_connect(dhcp->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
	
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: sendto(DISCOVER, IP_ADDR_BROADCAST, DHCP_SERVER_PORT)\n"));
    udp_sendto_if(dhcp->pcb, dhcp->p_out, IP_ADDR_BROADCAST, DHCP_CLIENT_PORT, netif);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: deleting()ing\n"));
    dhcp_delete_request(netif);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp_ask: SELECTING\n"));
  } 
	else { 

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("dhcp_ask: could not allocate DHCP request\n"));
  }
  //LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp_ask(): set request timeout %"U16_F" msecs\n", msecs));
  return ret;
}

static int dhcp_relay_offer(struct netif *netif)
{
	int ret;
	struct dhcp_srv *dhcp=netif->dhcpsrv;
	//struct dhcp_client *client=dhcp->cur_client;

	ret=dhcp_create_replay(netif);
  	if (ret == ERR_OK) {
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_nask: making replay\n"));

    dhcp_option(dhcp, DHCP_OPTION_MESSAGE_TYPE, DHCP_OPTION_MESSAGE_TYPE_LEN);
	dhcp_option_byte(dhcp, DHCP_OFFER);	  ///offer 选项
	
    dhcp_option(dhcp, DHCP_OPTION_SERVER_ID, DHCP_OPTION_SERVER_ID_LEN);//server id
    dhcp_option_long(dhcp,ntohl(netif->ip_addr.addr));


    dhcp_option(dhcp, DHCP_OPTION_LEASE_TIME, DHCP_OPTION_LEASE_TIME_SIZE);	 ///？ time in second
    dhcp_option_long(dhcp,ntohl(0x803a0900));

    dhcp_option(dhcp, DHCP_OPTION_T1, DHCP_OPTION_LEASE_TIME_SIZE);////??T1 renewal time  更新时间
    dhcp_option_long(dhcp,ntohl(0xc0900400));
	
    dhcp_option(dhcp, DHCP_OPTION_T2, DHCP_OPTION_LEASE_TIME_SIZE);	//////T2 rebind time  重新连接时间
    dhcp_option_long(dhcp,ntohl(0x50fd0700));

    dhcp_option(dhcp, DHCP_OPTION_SUBNET_MASK, 4/*num options*/);//subnetmask
	dhcp_option_long(dhcp,ntohl(netif->netmask.addr));

	dhcp_option(dhcp, DHCP_OPTION_ROUTER, 4/*num options*/);//router
	dhcp_option_long(dhcp,ntohl(netif->gw.addr));

	
	dhcp_option(dhcp, DHCP_OPTION_DNS_SERVER, 8/*num options*/);//dns
	dhcp_option_long(dhcp,ntohl(dns_getserver(0).addr));
	dhcp_option_long(dhcp,ntohl(dns_getserver(1).addr));
	
    dhcp_option_trailer(dhcp);

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: realloc()ing\n"));
    pbuf_realloc(dhcp->p_out, sizeof(struct dhcp_msg) - DHCP_OPTIONS_LEN + dhcp->options_out_len);
    udp_connect(dhcp->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: sendto(DISCOVER, IP_ADDR_BROADCAST, DHCP_SERVER_PORT)\n"));
    udp_sendto_if(dhcp->pcb, dhcp->p_out, IP_ADDR_BROADCAST, DHCP_CLIENT_PORT, netif);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_ask: deleting()ing\n"));
    dhcp_delete_request(netif);
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp_ask: SELECTING\n"));

  } 
	else { 

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("dhcp_ask: could not allocate DHCP request\n"));
  }
 // LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp_ask(): set request timeout %"U16_F" msecs\n", msecs));
  return ret;
}


static u8_t
dhcp_get_option_byte(u8_t *ptr)
{
  LWIP_DEBUGF(DHCP_DEBUG, ("option byte value=%"U16_F"\n", (u16_t)(*ptr)));
  return *ptr;
}
/**
 * Return the 32-bit value of DHCP option data.
 *
 * @param client DHCP client.
 * @param ptr pointer obtained by dhcp_get_option_ptr().
 *
 * @return byte value at the given address.
 */
static u32_t dhcp_get_option_long(u8_t *ptr)
{
  u32_t value;
  value = (u32_t)(*ptr++) << 24;
  value |= (u32_t)(*ptr++) << 16;
  value |= (u32_t)(*ptr++) << 8;
  value |= (u32_t)(*ptr++);
  LWIP_DEBUGF(DHCP_DEBUG, ("option long value=%"U32_F"\n", value));
  return value;
}


static void dhcpsrv_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	  struct netif *netif = (struct netif *)arg;
	  struct dhcp_srv *dhcp = netif->dhcpsrv;
	  struct dhcp_msg *reply_msg = (struct dhcp_msg *)p->payload;
	  struct dhcp_client *client=NULL;
	  static struct dhcp_client tmp_client;
	  u8_t *options_ptr;
	  u8_t msg_type;
	  u32_t ipaddr;
	  u32_t netmask=ntohl(netif->netmask.addr);
	  int ret;
	  int invaild=1;
	//  debug_printf("dhcp server recv a packet!\n");
	//  debug_data_stream("packet",p->payload,p->tot_len);

	  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 3, ("dhcp_recv(pbuf = %p) from DHCP client %"U16_F".%"U16_F".%"U16_F".%"U16_F" port %"U16_F"\n", (void*)p,
		(u16_t)(ntohl(addr->addr) >> 24 & 0xff), (u16_t)(ntohl(addr->addr) >> 16 & 0xff),
		(u16_t)(ntohl(addr->addr) >>  8 & 0xff), (u16_t)(ntohl(addr->addr) & 0xff), port));
	  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("pbuf->len = %"U16_F"\n", p->len));
	  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("pbuf->tot_len = %"U16_F"\n", p->tot_len));
	  /* prevent warnings about unused arguments */
	  LWIP_UNUSED_ARG(pcb);
	  LWIP_UNUSED_ARG(addr);
	  LWIP_UNUSED_ARG(port);
	  memset((u8_t *)&tmp_client,0,sizeof(struct dhcp_client));
	  dhcp->p = p;
	  /* TODO: check packet length before reading them */
	  if (reply_msg->op != DHCP_BOOTREQUEST) {
		////boot  request message 
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("not a DHCP request message, but type %"U16_F"\n", (u16_t)reply_msg->op));
		goto free_pbuf_and_return;
	  }
	  
	  if(reply_msg->cookie!=htonl(0x63825363UL)){
	    
		  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("invailed magic"));
		  goto free_pbuf_and_return;
	  }	 
	  	
	  dhcp->xid=ntohl(reply_msg->xid);//current client xid ,used for replay  
	  /* option fields could be unfold? */
	  if (dhcp_unfold_request(dhcp) != ERR_OK) {
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 2, ("problem unfolding DHCP message - too short on memory?\n"));
		goto free_pbuf_and_return;
	  }  
	  /* iterate through hardware address and match against DHCP message */
	 // printf("iterate  through hardware address and match against DHCP message\n ");
	  client=find_vaild_client(netif,reply_msg->chaddr);
	  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("searching DHCP_OPTION_MESSAGE_TYPE\n"));
	  /* obtain pointer to DHCP message type */
	 // printf("get pointer (dhcp message type\n)");
	  options_ptr = dhcp_get_option_ptr(dhcp, DHCP_OPTION_MESSAGE_TYPE);
	  if (options_ptr == NULL) {
	  printf("error cant find pointer\n");
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("DHCP_OPTION_MESSAGE_TYPE option not found\n"));
		goto free_pbuf_and_return;
	  }
	  /* read DHCP message type */
	  msg_type = dhcp_get_option_byte(options_ptr + 2);
	  /* message type is DHCP discover? */
	  if (msg_type == DHCP_DISCOVER) {
	      //  printf("DHCP_DISCOVER\N\n");
			LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("DHCP_DISCOVER received\n"));
			if(!client){//have own the ip address
			    printf("have owm the ip ");
				client=add_dhcp_client(netif,reply_msg->chaddr,reply_msg->sname);
				if(!client){
				//    printf("????? no valid client for dhcp\n");
					LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("DHCP_RELAY no vaild client for dhcp!\n"));
					memcpy(tmp_client.hwaddr,reply_msg->chaddr,DHCP_CHADDR_LEN);//client mac address
					dhcp->cur_client=&tmp_client;
					dhcp_replay_ask(netif,0);  /////0 表示没有东西给还它吗？
					goto free_pbuf_and_return;
				}	
		   }
		   	
		  dhcp->cur_client=client;
		  dhcp_relay_offer(netif);/////这里就开始offer吗  
	  }
	  /* received a DHCP_NAK in appropriate state? */
	  else if(msg_type == DHCP_REQUEST){
	    //   printf("DHCP_REQUEST\n");
			options_ptr = dhcp_get_option_ptr(dhcp, DHCP_OPTION_SERVER_ID);
			if(!options_ptr)//some packet have no server id option,so we send a noask
			  	{    printf("have no server id option,so we send noask\n");	
				   goto dhcp_nask;}
			ipaddr=dhcp_get_option_long(options_ptr+2);
			if(ipaddr!=ntohl(netif->ip_addr.addr))//server id didn't mach,this situation is multi-dhcp server
			  	{	
			    printf("this is multi-dhcp server ,becuse de server id didnt mach\n");
				goto free_pbuf_and_return;	}		
			options_ptr = dhcp_get_option_ptr(dhcp, DHCP_OPTION_REQUESTED_IP);
			ipaddr=dhcp_get_option_long(options_ptr+2);
			ret=(ipaddr)&(~netmask);
			if((ret<DHCP_ADDR_START)||(ret>DHCP_ADDR_END)||
				((ipaddr&netmask)!=
				(ntohl(netif->ip_addr.addr)&netmask))){//invaild ip address
				 invaild=1;
				  printf("invalid ip address\n");
				 debug_printf("request invaild ip address!\n");
			}
			else if(client){//if have registered in the client pool,must use the old address
			    printf("have registered in the client pool ,so use de old address\n");
				if(client->ipaddr.addr==htonl(ipaddr))//client match,ok
				     printf("client match ok\n");
					invaild=0;
			}
			if(client)
				dhcp->cur_client=client;
			else{
				memcpy(tmp_client.hwaddr,reply_msg->chaddr,DHCP_CHADDR_LEN);
				dhcp->cur_client=&tmp_client;
				debug_printf("cann't find requested client!\n");
			}
   		dhcp_nask:
			dhcp_replay_ask(netif,!invaild);
	  }
	  /* received a DHCP_OFFER in DHCP_SELECTING state? */
	  else if (msg_type == DHCP_RELEASE) {
	  printf("DHCP_RELEASE\n");
		LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | 1, ("DHCP_RELEASE release the client ip address\n"));
		del_dhcp_client(netif,client);
	  }
	free_pbuf_and_return:
	  dhcp_free_request(dhcp);
	  pbuf_free(p);
	  dhcp->p = NULL;
}




#define DHCP_MAX_MSG_LEN_MIN_REQUIRED  576


err_t dhcpsrv_start(struct netif *netif)
{
  struct dhcp_srv *dhcp;
  err_t result = ERR_OK;
  u32_t netmask=ntohl(netif->netmask.addr);
  

  LWIP_ERROR("netif != NULL", (netif != NULL), return ERR_ARG;);
  dhcp = netif->dhcpsrv;
  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("dhcp_start(netif=%p) %c%c%"U16_F"\n", (void*)netif, netif->name[0], netif->name[1], (u16_t)netif->num));

  /*check the client pool config*/
  if(((~netmask)<DHCP_ADDR_END)||
	 (DHCP_ADDR_END<DHCP_ADDR_START)){
	 
	 //LWIP_ERROR("dhcp client pool config error",0, return ERR_ARG;);
	 return ERR_ARG;
  }

  /* check MTU of the netif */
  if (netif->mtu < DHCP_MAX_MSG_LEN_MIN_REQUIRED) {
   
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_start(): Cannot use this netif with DHCP: MTU is too small\n"));
    return ERR_MEM;
  }
  /* no DHCP client attached yet? */
  if (dhcp == NULL) {
   
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_start(): starting new DHCP server\n"));

    dhcp = mem_malloc(sizeof(struct dhcp_srv));
    if (dhcp == NULL) {
	 
      LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_start(): could not allocate dhcp\n"));
      return ERR_MEM;
    }
    /* store this dhcp client in the netif */
    netif->dhcpsrv= dhcp;

    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_start(): allocated dhcp"));
  /* already has DHCP client attached */
  } else {
 
    LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE | 3, ("dhcp_start(): restarting DHCP configuration\n"));
    if (dhcp->pcb != NULL) {
	
      udp_remove(dhcp->pcb);
    }
    if (dhcp->p != NULL) {
      pbuf_free(dhcp->p);
    }
  }
  /* clear data structure */
  memset(dhcp, 0, sizeof(struct dhcp_srv));		///dhcp主要用这个结构的  
  /*inital the dhcp server's client list head*/
  list_init_head(&dhcp->head_node);	///dhcp 请求连接表的头 

  /* allocate UDP PCB */
  dhcp->pcb = udp_new();  //果然用udp  来处理那个连接
  if (dhcp->pcb == NULL) {
   
    LWIP_DEBUGF(DHCP_DEBUG  | LWIP_DBG_TRACE, ("dhcp_start(): could not obtain pcb\n"));
    mem_free((void *)dhcp);
    netif->dhcpsrv=dhcp=NULL;
    return ERR_MEM;
  }
  dhcp->pcb->so_options|=SOF_BROADCAST;	///?pcd的so_option?
  /* set up local and remote port for the pcb */
  udp_bind(dhcp->pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);	 ///设置本地和远程的端口？ 那ip 是多少呢
  udp_connect(dhcp->pcb, IP_ADDR_ANY,DHCP_CLIENT_PORT );
  /* set up the recv callback and argument ,wait for client packet*/
  udp_recv(dhcp->pcb, dhcpsrv_recv, netif);	 ///dchp 请求 接收的回调函数 
  LWIP_DEBUGF(DHCP_DEBUG | LWIP_DBG_TRACE, ("dhcp_start(): starting DHCP configuration\n"));
  debug_printf("start dhcp server!\n");
  return result;
}


#endif



