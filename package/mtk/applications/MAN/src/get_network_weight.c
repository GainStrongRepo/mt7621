/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    	get_network_weight.c
*/
#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>          
#include <sys/ioctl.h>    
#include <errno.h>        
#include <fcntl.h>
#include <stdbool.h>
#include <linux/netlink.h>
#include <sys/types.h>

#define OID_WH_EZ_GET_DEVICE_INFO		          0x2008
#define OID_WH_EZ_GET_ROAM_CANDIDATES   	      0x2009
#define OID_WH_EZ_UPDATE_STA_INFO				  0x2010
#define RT_PRIV_IOCTL                             (SIOCIWFIRSTPRIV + 0x0E)
#define MAC_ADDR_LEN 				6
#define NETWORK_WEIGHT_LEN  				(MAC_ADDR_LEN + 1)
#define MAX_LEN_OF_SSID 			32 
#define LEN_PMK    				32
#define EZ_MAX_DEVICE_SUPPORT   7
#define FALSE 					0
#define TRUE 					1
#define PORT1 5001
#define PORT2 5002
#define PORT3 5003
#define BUFLEN 512  //Max length of buffer
#define TAG_LEN			4
#define	MAC_ADDR_LEN	6
#define BAND_WITH_EZSTATUS_LEN 10
#define INTERFACE_LEN   20
#define MAX_BAND       20

unsigned int try_roaming_count = 3;
int s;
int NetStatus;
unsigned char curr_sta_cnt;
#define TAG_LEN			4
#define	MAC_ADDR_LEN	6

unsigned char chipbit = 0;
#define MT7615D_2G_PROFILE "/etc/wireless/mt7615e/mt7615e.1.2G.dat"
#define MT7615D_5G_PROFILE "/etc/wireless/mt7615e/mt7615e.1.5G.dat"
#define MT7615N_FIRST_CARD_PROFILE "/etc/wireless/mt7615e/mt7615e.1.dat"
#define MT7615N_SECOND_CARD_PROFILE "/etc/wireless/mt7615e/mt7615e.2.dat"
#define MT7603_PROFILE "/etc/wireless/mt7603e/mt7603e.dat"
#define MT7612_PROFILE "/etc/wireless/mt76x2e/mt7612e.dat"
#define MT7628_PROFILE "/etc/wireless/mt7628/mt7628.dat"

#define CHIP_7615D		1
#define CHIP_FIRST_7615N	2
#define CHIP_SECOND_7615N	4
#define CHIP_7603		8
#define CHIP_7612		16
#define CHIP_7628		32

#define BAND1			1
#define BAND2			2
#define BAND3			3
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif
unsigned char is_triband = 0;
unsigned char is_man_nonman = 0;
char NonEzBands = 0;
char EzBands = 0;
char is_multipro = 0;
int cmd = 0;
char inf_name[8] = {0};
char sys_ssid[32];
char sys_pmk[32];
char sys_pmk_string[65];
unsigned char sys_network_weight[NETWORK_WEIGHT_LEN];
char sys_ssid1[32];
char sys_ssid2[32];
char sys_psk1[32];
char sys_psk2[32];
char sys_pmk1[32];
char sys_pmk2[32];
char sys_pmk_string1[65];
char sys_pmk_string2[65];
char No_of_Band[1];	

#define PRINT_MAC(addr) \
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

typedef  unsigned char UINT8;

typedef struct   __attribute__((__packed__)) _ez_node_number {
	unsigned char path_len; //path len is the length of the entire node number including the root_mac
	unsigned char root_mac[MAC_ADDR_LEN];
	unsigned char path[EZ_MAX_DEVICE_SUPPORT];
}EZ_NODE_NUMBER;

#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
	printf(__fmt);

typedef struct device_info_to_app_s
{
	
	unsigned char dual_chip_dbdc;
	unsigned char ssid_len1;
	unsigned char ssid_len2;
	unsigned char internet_access;
	char ssid1[MAX_LEN_OF_SSID];
	char ssid2[MAX_LEN_OF_SSID];
	unsigned char pmk1[LEN_PMK];
	unsigned char pmk2[LEN_PMK];
	unsigned char device_connected[2];
	unsigned char no_of_blank_scan_5g;
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	char peer2p4mac[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	unsigned char sta_mac[10][MAC_ADDR_LEN];
} device_info_to_app_t;


typedef struct roam_candidate_s
{
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char ez_device;
	char 		  rssi;
	char 		  channel; 
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
}roam_candidate_t;
typedef struct roam_candidate_to_app_s
{

	unsigned int no_of_candidates;
	unsigned char ssid[32];
	unsigned char ssid_length;
	roam_candidate_t roam_candidate[10];
	
} roam_candidate_to_app_t;

typedef struct ez_config_data_cmd_s {
	UINT8 ssid_len;
	UINT8 ssid[98];
	UINT8 psk_len;
	UINT8 psk[194];
	UINT8 encryp_len;
	UINT8 encryptype[15];
	UINT8 auth_len;
	UINT8 authmode[27];
			
}ez_config_data_cmd_t, *p_ez_config_data_cmd_t;

device_info_to_app_t device_info;

typedef struct device_info_from_user_s                //structure to store inputs from user
{
	char band;
	char Ez_status;
	unsigned char ap_interface[INTERFACE_LEN];
	unsigned char cli_interface[INTERFACE_LEN];
} device_info_from_user_t;

device_info_from_user_t input_device_info[MAX_BAND];
enum {
	SINGLE_BAND = 0x1,
	DBDC_NON_MULTI = 0x2,
	DBDC_MULTI = 0x3,
	DUAL_BAND = 0x4,
	TRIBAND_NON_MULTI = 0x5,
	TRIBAND_MULTI = 0x6,
	DUAL_BAND_MAN_NONMAN = 0x7
};



/*
    ========================================================================
    Routine Description:
        To get chip combo

    Arguments:
        NULL

    Return Value:
       NULL

    ========================================================================
*/

void get_chip_combo(void)
{
	if (!access(MT7615N_SECOND_CARD_PROFILE, 0)) {
		chipbit |= CHIP_SECOND_7615N;
		printf("CHIP_SECOND_7615N\n");
	}
	if (!access(MT7615N_FIRST_CARD_PROFILE, 0)) {
		chipbit |= CHIP_FIRST_7615N;
		printf("CHIP_FIRST_7615N\n");
	}
	if (!access(MT7615D_2G_PROFILE, 0)) {
		chipbit |= CHIP_7615D;
		printf("CHIP_7615D\n");
	}
	if (!access(MT7615D_5G_PROFILE, 0)) {
		chipbit |= CHIP_7615D;
		printf("CHIP_7615D\n");
	}
	if (!access(MT7603_PROFILE, 0)) {
		chipbit |= CHIP_7603;
		printf("CHIP_7603\n");
	}
	if (!access(MT7612_PROFILE, 0)) {
		chipbit |= CHIP_7612;
		printf("CHIP_7612\n");
	}
	if (!access(MT7628_PROFILE, 0)) {
		chipbit |= CHIP_7628;
		printf("CHIP_7628\n");
	}
	return;
}




/*
    ========================================================================
    Routine Description:
        To set ManConf default value to corresponding profile

    Arguments:
        NULL

    Return Value:
       NULL

    ========================================================================
*/
void set_ManConf_default2dat()
{
	char cmd[1024];

        /*
        Single Chip with DBDC 
	        nvram_set 2860 ManConf "2 2Ez rax0 apclix0 5Ez ra0 apcli0" 
        Dual Chip 
                nvram_set 2860 ManConf "2 2Ez ra0 apcli0 5Ez rai0 apclii0"
        Triband 
                nvram_set 2860 ManConf "3 5Ez rai0 apclii0 2NEz rax0 apclix0 5NEz ra0 apcli0"
        */

        
	if ((chipbit & CHIP_7615D) && (chipbit & CHIP_SECOND_7615N)) {
		sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"3 5Ez rai0 apclii0 2NEz rax0 apclix0 5NEz ra0 apcli0\"");
        	system(cmd);		
	}
	else if (chipbit & CHIP_7615D) {
		sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"2 2Ez rax0 apclix0 5Ez ra0 apcli0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_FIRST_7615N) && (chipbit & CHIP_SECOND_7615N)) {
		sprintf(cmd, "wificonf path %s", MT7615N_FIRST_CARD_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_SECOND_7615N)) {
		sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_7612)) {
		sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	else if ((chipbit & CHIP_7628) && (chipbit & CHIP_7612)) {
		sprintf(cmd, "wificonf path %s", MT7628_PROFILE);
		system(cmd);

        	sprintf(cmd, "wificonf set ManConf \"2 2Ez ra0 apcli0 5Ez rai0 apclii0\"");
        	system(cmd);		
	}
	return;
}



/*
input band-
	for tri-band
		2860:5g LB
		wifi3:2G
		rtdev:5G-HB
	for 7615D
		2860:5G
		rtdev:2G
	for 7603 + 7615
		2860:2G
		rtdev:5G
	for 7603 + 7612
		2860:2G
		rtdev:5G
*/
void switch_profile(char *nvram_type)
{
	char cmd[1024];

	memset(cmd, 0, sizeof(cmd));
	if ((chipbit & CHIP_7615D) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
			system(cmd);
		}
		else if (!memcmp(nvram_type, "wifi3", min(strlen("wifi3"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_2G_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
	}
	else if (chipbit & CHIP_7615D) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615D_5G_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615D_2G_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_FIRST_7615N) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7615N_FIRST_CARD_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
		
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_SECOND_7615N)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7615N_SECOND_CARD_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_7603) && (chipbit & CHIP_7612)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7603_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7612_PROFILE);
			system(cmd);
		}
	}
	else if ((chipbit & CHIP_7628) && (chipbit & CHIP_7612)) {
		if (!memcmp(nvram_type, "2860", min(strlen("2860"), strlen(nvram_type)))) {
			sprintf(cmd, "wificonf path %s", MT7628_PROFILE);
			system(cmd);
		}
		else {
			sprintf(cmd, "wificonf path %s", MT7612_PROFILE);
			system(cmd);
		}
	}
	return;
}


int ap_oid_query_info(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
        struct iwreq wrq;
        strcpy(wrq.ifr_name, DeviceName);
        wrq.u.data.length = PtrLength;
        wrq.u.data.pointer = (caddr_t) ptr;
        wrq.u.data.flags = OidQueryCode;
        return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}
int  get_ra0_mac(char *buf, char* ap_interface)
{
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, ap_interface);
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
	printf("\n---------  mac1 ----------------\n");
    for (i = 0; i < 6; ++i)
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    puts("\n");
	printf("\n---------  ap_interface ----------------\n");
	   printf(" ap_interface %s", ap_interface);
	
	printf("\n---------  mac3 ----------------\n");

  }

  memcpy(buf, s.ifr_hwaddr.sa_data, 6);
  close(fd);
  return 0;

}

int get_rai0_mac (char *buf,char *ap_interface)
{
#if 1	
	FILE *fp;
	int i =0;
	char command [100] = {0};
	char peer_mac[20] = {'\0'};
	sprintf(command, "iwconfig %s | grep \"Access Point\" | awk -F \" \" \'{print $5}\' > temp_file_mac_rai",ap_interface);
	system(command);
	fp = fopen("temp_file_mac_rai", "r");
	fgets(peer_mac, 18, fp);
	fclose(fp);
	printf("%s\n", peer_mac);
	if (peer_mac[0] != 'N')
	for (i = 0; i < 17 ; i++)
	{
		if (i % 3 == 0)
		{
			buf[i/3] = (peer_mac[i] <= '9') ? ((peer_mac[i] - '0') << 4) : ((peer_mac[i] - 'A' + 10) << 4) ;
		}
		else if (i % 3 == 1)
		{
			buf[i/3] |= (peer_mac[i] <= '9') ? ((peer_mac[i] - '0')) : ((peer_mac[i] - 'A' + 10));
		} else {
			continue;
		}
	}
#endif
	return 0;
}

int get_ez_idx()		
{
	int i;
	for(i = 0; i <(No_of_Band[0]-'0'); i++) 

	{
		if(input_device_info[i].Ez_status == '1')
			return i;
	}
}

int service_terminate = 0;
void handle_signal_term(void)
{
	service_terminate = 1;
	printf("handle_signal_term: service terminate.\n");
}

void* server()
{
	int server_socket;
	struct sockaddr_in server_address, client_address;
	char buf[512];
	unsigned int clientLength;
	int checkCall, message;
	int broadcast = 1;
	char command[BUFLEN];
	ez_config_data_cmd_t p_config_data;
	int len;

	fd_set fdsr;
	int maxsock;
	struct timeval tv;
	int ret;
	
	/*Create socket */
	server_socket=socket(AF_INET, SOCK_DGRAM, 0);
	maxsock = server_socket;

	if(server_socket == -1)
		perror("Error: socket failed");

	setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST,	&broadcast, sizeof(broadcast));

	bzero((char*) &server_address, sizeof(server_address));

	/*Fill in server's sockaddr_in*/
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(PORT1);

	/*Bind server socket and listen for incoming clients*/
	checkCall = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	if(checkCall == -1)
		perror("Error: bind call failed");

	while(1)
	{
		if (service_terminate)
			break;

		FD_ZERO(&fdsr);
		FD_SET(server_socket, &fdsr);

		// timeout setting
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			break;
		} else if (ret == 0) {
			//perror("timeout\n");
			continue;
		}

		clientLength = sizeof(client_address);
		memset(buf, '\0', BUFLEN);
		message = 0;
		message = recvfrom(server_socket, buf, BUFLEN, 0,
		(struct sockaddr*) &client_address, &clientLength);

		if(message == -1)
			perror("Error: recvfrom call failed");

		if (!memcmp(&buf[0], "5001", 4))
		{
			memset(&p_config_data, 0, sizeof(ez_config_data_cmd_t));
			len = TAG_LEN;

			p_config_data.ssid_len = (UINT8)buf[len];
			len += 1;

			memcpy(&p_config_data.ssid[0], &buf[len], p_config_data.ssid_len);
			len += p_config_data.ssid_len;

			p_config_data.psk_len = (UINT8)buf[len];
			len += 1;

			memcpy(&p_config_data.psk, &buf[len], p_config_data.psk_len);
			len += p_config_data.psk_len;

			p_config_data.encryp_len = (UINT8)buf[len];
			len += 1;
	
			memcpy(&p_config_data.encryptype[0], &buf[len], p_config_data.encryp_len);
			len += p_config_data.encryp_len;

			p_config_data.auth_len = (UINT8)buf[len];
			len += 1;

			memcpy(&p_config_data.authmode[0], &buf[len], p_config_data.auth_len);

			if(is_triband || is_man_nonman)
                        {
				sprintf(command, "iwpriv %s set ez_set_ssid_psk=\"%s;%s;%s;%s\"", input_device_info[get_ez_idx()].ap_interface,
						p_config_data.ssid, p_config_data.psk, p_config_data.encryptype, p_config_data.authmode);
			} else {
				sprintf(command, "iwpriv %s set ez_set_ssid_psk=\"%s;%s\"",input_device_info[get_ez_idx()].ap_interface,p_config_data.ssid, p_config_data.psk);		}	
		
			system(command);
			printf("%s %d %s\n", __FUNCTION__,__LINE__,command);
			NetStatus = 0;
		
	}
}

	checkCall = close(server_socket);
	printf("server: close server socket.\n");

	if(checkCall == -1)
		perror("Error: bind call failed");
}
					

void *recv_sta_update()
{
	int server_socket;
	struct sockaddr_in server_address, client_address;
	char buf[512];
	char mac[MAC_ADDR_LEN]={0};
	unsigned int clientLength;
	int checkCall, message;
	int broadcast = 1;

	fd_set fdsr;
	int maxsock;
	struct timeval tv;
	int ret;

	/*Create socket */
	server_socket=socket(AF_INET, SOCK_DGRAM, 0);
	maxsock = server_socket;

	if(server_socket == -1)
		perror("Error: socket failed");

	setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST,	&broadcast, sizeof(broadcast));

	bzero((char*) &server_address, sizeof(server_address));

	/*Fill in server's sockaddr_in*/
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(PORT3);

	/*Bind server socket and listen for incoming clients*/
	checkCall = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));

	if(checkCall == -1)
		perror("Error: bind call failed");

	if(is_triband || is_man_nonman)
		get_rai0_mac(mac,input_device_info[0].ap_interface);	
	else
		get_ra0_mac(mac, input_device_info[0].ap_interface);
	
	while(1)
	{
		if (service_terminate)
			break;

		FD_ZERO(&fdsr);
		FD_SET(server_socket, &fdsr);

		// timeout setting
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			break;
		} else if (ret == 0) {
			//perror("timeout\n");
			continue;
		}

		clientLength = sizeof(client_address);
		memset(buf, '\0', BUFLEN);
		message = 0;
		message = recvfrom(server_socket, buf, BUFLEN, 0,
		(struct sockaddr*) &client_address, &clientLength);

		if(message == -1)
			perror("Error: recvfrom call failed");

		if (!memcmp(&buf[0], "5004", 4))
		{
			if (!memcmp(&buf[TAG_LEN], mac, MAC_ADDR_LEN))
			{
				//This is send by me do nothing.
			}
			else
			{	
				ap_oid_query_info(OID_WH_EZ_UPDATE_STA_INFO, s, input_device_info[get_ez_idx()].ap_interface, &buf[TAG_LEN + MAC_ADDR_LEN], message - (TAG_LEN + MAC_ADDR_LEN));
			}
		}
	}

	checkCall = close(server_socket);
	printf("recv_sta_update: close server socket.\n");

	if(checkCall == -1)
		perror("Error: bind call failed");

}

int main(int argc, char *argv[])
{
	FILE *fp;
	char command[100];
	int init_done = 0;
	pthread_t td1, td2;
	char no_of_bands_input, band_with_EzStatus[BAND_WITH_EZSTATUS_LEN];
	int ret=0, i=0, argument_no, index_no;
	int nvram_input_index=0;;
	char c;
	char nvram_input_args[10][10];
        
        get_chip_combo();

        set_ManConf_default2dat();
        
	memset(command, 0, 100);
        switch_profile("2860");
	sprintf(command, "wificonf get ManConf > Manconfig");
	system(command);
	fp = fopen("Manconfig", "r");
	memset(nvram_input_args,0,sizeof(nvram_input_args));
	while(! feof(fp))
	{
		c = fgetc(fp);
		int temp=0;
		while(c!=' ' && (!feof(fp)) && (c!='\n'))
		{
			nvram_input_args[nvram_input_index][temp] = c;
			temp++;
			c = fgetc(fp);
		}	
		nvram_input_index++;
		if(c == '\n')
			break;	
		}
	fclose(fp);
	no_of_bands_input=nvram_input_args[0][0];

	printf("arguments = %d, no_of_bands = %c\n", nvram_input_index, no_of_bands_input);
	if ((nvram_input_index < 4) || (nvram_input_index != (((no_of_bands_input - '0') * 3) + 1)))
		{
		printf("INSUFFICIENT ARGUMENTS arguments = %d, no_of_bands = %c\n", (nvram_input_index), no_of_bands_input);
		return 0;
	}


	is_triband = FALSE;
	is_man_nonman = FALSE;

	switch(no_of_bands_input)
        {
		case '1':
	
			input_device_info[0].band = nvram_input_args[1][0];
			if((nvram_input_args[1][1] == 'E') || (nvram_input_args[1][1] == 'e'))
                {
				input_device_info[0].Ez_status = '1';
			}
			else
			{
				printf("Wrong Input... Enter Easy setup band info !!\n");				
				return 0;
			}
			memcpy(&input_device_info[0].ap_interface[0], &nvram_input_args[2], strlen(nvram_input_args[2]));
			memcpy(&input_device_info[0].cli_interface[0], &nvram_input_args[3], strlen(nvram_input_args[3]));
			cmd = SINGLE_BAND;	
			printf("INT0 %s %s \n",&input_device_info[0].ap_interface[0],&input_device_info[0].cli_interface[0]);

		break;				
		case '2':
		
			for(i=0; i<2; i++)
			{
				/* argv[2][0] will be band for first interface and argv[5][0] for second second as per user input
				 to select which index of input_device_info will have to be updated*/
				
				argument_no = (3*i) + 1; 

				/* 2.4G band info will be store on index 0 and 5G band on index 1*/				
				if(nvram_input_args[argument_no][0] == '2')    
					index_no = 0;
				else if(nvram_input_args[argument_no][0] == '5')
					index_no = 1;
				else {
					printf(" Worng band %c entered !! Band support 2G or 5G only ....\n",nvram_input_args[argument_no][0]);					
					return 0;
				}
				input_device_info[index_no].band = nvram_input_args[argument_no][0];
				
				
				if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
				{
					input_device_info[index_no].Ez_status = '1';
			        }
				else
				{
					input_device_info[index_no].Ez_status = '0';
				}		
				printf("%s %s \n",nvram_input_args[argument_no + 1],nvram_input_args[argument_no + 2]);
				memcpy(&input_device_info[index_no].ap_interface[0], nvram_input_args[argument_no + 1], strlen(nvram_input_args[argument_no + 1]));
				memcpy(&input_device_info[index_no].cli_interface[0], nvram_input_args[argument_no + 2], strlen(nvram_input_args[argument_no + 2]));
			}
				
			printf("INT0 %s %s \n INT1 %s %s \n",&input_device_info[0].ap_interface[0],&input_device_info[0].cli_interface[0],&input_device_info[1].ap_interface[0],&input_device_info[1].cli_interface[0]);
			if(((input_device_info[0].Ez_status == '0') && (input_device_info[1].Ez_status == '1'))
				|| ((input_device_info[0].Ez_status == '1') && (input_device_info[1].Ez_status == '0'))) {
				
				cmd = DUAL_BAND_MAN_NONMAN;
				is_man_nonman = TRUE;
			}
			else if((input_device_info[0].Ez_status == '1') && (input_device_info[1].Ez_status == '1')) {
				cmd = DUAL_BAND;
			}
			else {
				printf( "Entered configuration is not supported !!\n");				
				return 0;
			}

		break;			
		case '3':

			for(i=0; i<3; i++)
			{			
				/* argv[2][0] will be band for first interface and argv[5][0] for second second as per user input
				 to select which index of input_device_info will have to be updated*/
		
				argument_no = (3*i) + 1;  
			
				/* 5G EZ band will be store on index 0 , 2.4G NEZ band on index 1 and 5G NEZ band on index 1*/								
				if(nvram_input_args[argument_no][0] == '5') {
					if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
						index_no = 0;
				else 
						index_no = 2;
				}
				else if(nvram_input_args[argument_no][0] == '2') {
					index_no = 1;
			        }
				else {
					printf(" Worng band %c entered !! Band support 2G or 5G only ....\n", nvram_input_args[argument_no][0]); 				
					return 0;
			        }
				input_device_info[index_no].band = nvram_input_args[argument_no][0];

				if((nvram_input_args[argument_no][1] == 'E') || (nvram_input_args[argument_no][1] == 'e'))
			        {
					input_device_info[index_no].Ez_status = '1';
			        }
	                        else
			        {
					input_device_info[index_no].Ez_status = '0';
			        }
				memcpy(&input_device_info[index_no].ap_interface[0], nvram_input_args[argument_no + 1], strlen(nvram_input_args[argument_no + 1]));
				memcpy(&input_device_info[index_no].cli_interface[0],nvram_input_args[argument_no + 2], strlen(nvram_input_args[argument_no + 2]));
			}
			printf("INT0 %s %s \n INT1 %s %s \n INT2 %s %s \n",&input_device_info[0].ap_interface[0],&input_device_info[0].cli_interface[0],&input_device_info[1].ap_interface[0],&input_device_info[1].cli_interface[0],&input_device_info[2].ap_interface[0],&input_device_info[2].cli_interface[0]);
		
			cmd = TRIBAND_MULTI;
			is_triband = TRUE;
		break;
		
		default:
			printf(" the no of bands should be 1 or 2 or 3 \n"); 					
				return 0;
			}
	
	memcpy(&No_of_Band, &no_of_bands_input, 1); 
	printf("No_of_Band : %c\n", No_of_Band[0]);
			
	s = socket(AF_INET, SOCK_DGRAM, 0);
	signal(SIGTERM, handle_signal_term);
	pthread_create(&td1, NULL, server, NULL);
	pthread_create(&td2, NULL, recv_sta_update, NULL);	
	pthread_join( td1, NULL);
	pthread_join( td2, NULL);
	printf("ERROR : server thread killed\n");
	printf("ERROR : recv_sta_update thread Killed\n");
	return 0;
}







