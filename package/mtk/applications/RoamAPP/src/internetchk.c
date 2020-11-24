
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
    	internetchk.c
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <string.h>
#include <stdlib.h>
#include "internetchk.h"
#include "wireless_copy.h"
#include <stdbool.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>   
#include <net/if.h>       
#include <sys/ioctl.h>    
#include <errno.h>        

int NetStatus;					// Current Inertnet status
int ConnectionStatei0 = -1; 	// rai0 Internet status
int ConnectionState0 = -1;		// ra0 Internet status
bool ra0Status = Down;			// ra0 interface status
bool rai0Status = Down;			// rai0 interface status

/*
=====================================================================
Description:
	This function is used for checking interface status through IOCTL.
	
=====================================================================
*/

static int CheckLink(char *ifname) {

	struct ifreq if_req;
	int socId;
	int rv;

	socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	if (socId < 0)
		DBGPRINT(DEBUG_ERROR,"Socket failed. Errno = %d\n", errno);
		
    (void) strncpy(if_req.ifr_name, ifname, sizeof(if_req.ifr_name));

	rv = ioctl(socId, SIOCGIFFLAGS, &if_req);
    close(socId);

    if ( rv == -1) 
		DBGPRINT(DEBUG_ERROR,"Ioctl failed. Errno = %d\n", errno);

    return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
}

/*
=====================================================================
Description:
	This function is used for checking internet status by making connection on given IP
	using socket.
	
=====================================================================
*/

static int Connect_IP(char *ip) { 
	int res,soc; 
	int valopt; 
	socklen_t lon; 
	long arg; 
	fd_set myset; 
	struct timeval tv; 
	struct sockaddr_in addr; 

	
	// Create socket 
	soc = socket(AF_INET, SOCK_STREAM, 0); 
	if (soc < 0) 
	{ 
		DBGPRINT(DEBUG_ERROR,"Error creating socket (%d %s)\n", errno, strerror(errno));
		return 0; 
	} 

	addr.sin_family = AF_INET; 
	addr.sin_port = htons(80); 
	addr.sin_addr.s_addr = inet_addr(ip);

	// Set non-blocking 
	if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) 
	{ 
		DBGPRINT(DEBUG_ERROR, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
		return 0; 
	} 
	
	arg |= O_NONBLOCK; 

	if( fcntl(soc, F_SETFL, arg) < 0) { 
		DBGPRINT(DEBUG_ERROR, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
		return 0; 
	} 

	// Trying to connect with timeout 
	res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 

	if (res < 0) 
	{ 
		if (errno == EINPROGRESS) 
		{
			DBGPRINT(DEBUG_TRACE, "EINPROGRESS in connect() - selecting\n");
		      
			tv.tv_sec = 5; 
			tv.tv_usec = 0; 
			FD_ZERO(&myset); 
			FD_SET(soc, &myset); 

			res = select(soc+1, NULL, &myset, NULL, &tv); 

			if (res < 0 && errno != EINTR) 
			{ 
				DBGPRINT(DEBUG_ERROR, "Error connecting %d - %s\n", errno, strerror(errno));
				return 0; 
			} 
			else if (res > 0) 
			{ 
				// Socket selected for write 
				lon = sizeof(int); 
				if (getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) 
				{
					DBGPRINT(DEBUG_ERROR, "Error in getsockopt() %d - %s\n", errno, strerror(errno));
	                return 0; 
	            } 
				// Check the value returned... 
				if (valopt) 
				{ 
					DBGPRINT(DEBUG_ERROR, "Connection ERROR To IP : %s, valopt %d, strerror()- %s\n", ip, valopt, strerror(valopt));
                 	return 0; 
              	} 
				else
				{
					DBGPRINT(DEBUG_TRACE, "Connected To IP : %s\n",ip);
					return 1;
				}
			} 
			else
			{			 
				DBGPRINT(DEBUG_ERROR, "Timeout in select() - Cancelling!\n");
				return 0;
			}
		}
		else 
		{ 
			DBGPRINT(DEBUG_ERROR, "Error connecting %d - %s\n", errno, strerror(errno));
			return 0; 
		}
	}
	else
	{
		DBGPRINT(DEBUG_ERROR, "Error connecting %d - %s\n", errno, strerror(errno));
		return 0;
	}	

	close(soc);
  
}

/*
=====================================================================
Description:
	This function is used for send internet status to Driver through IOCTL
	
=====================================================================
*/

static bool SendToDriver(const char *ifname)
{
	int socket_desc;
	struct iwreq iwr;
	p_internet_command_t p_internet_command;

	p_internet_command = malloc(sizeof(p_internet_command_t));

	if(p_internet_command == NULL)
	{
		DBGPRINT(DEBUG_ERROR, "no memory!!! \n");
		return 0;
	}
	
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);

	iwr.u.data.flags = OID_802_11_INTERNET_COMMAND;
	iwr.u.data.flags |= OID_GET_SET_TOGGLE;

	p_internet_command->Net_status = (bool)(NetStatus);
	
	if (p_internet_command)
	{
		iwr.u.data.pointer = (caddr_t)p_internet_command;
		iwr.u.data.length = sizeof(p_internet_command_t);
	}

	//Create socket
	socket_desc = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket_desc == -1)
	{
		DBGPRINT(DEBUG_ERROR, "Could not create socket");
		return 0;
	}
	if (ioctl(socket_desc, RT_PRIV_IOCTL, &iwr) < 0) {
		
		DBGPRINT(DEBUG_ERROR, "SendToDriver failed: \n");
		free(p_internet_command);
		close(socket_desc);
		return 0;
	}
	
	free(p_internet_command);
	close(socket_desc);

	return 1;
}

static void SendToRa0()
{

	if((ConnectionState0 != NetStatus) && (ra0Status == UP))
	{
			
		if(SendToDriver("ra0") == FAILED)
		{
			if(SendToDriver("ra0") == SUCCESS)
				ConnectionState0 = NetStatus;
		}
		else
			ConnectionState0 = NetStatus;
	}

	return ;
}

static void SendToRai0()
{
	if((ConnectionStatei0 != NetStatus) && (rai0Status == UP))
	{
		if(SendToDriver("rai0") == FAILED)
		{
			if(SendToDriver("rai0") == SUCCESS)
				ConnectionStatei0 = NetStatus;
		}
		else
			ConnectionStatei0 = NetStatus;
	}

	return ;
}

/*
=====================================================================
Description:
	- This function is used for periodically call for checking interface status.
	- update interface status. 
	- If status change, Send the status to Driver. 
	
=====================================================================
*/

void * Inf_status()
{

	while(1)
	{
		bool StatusRa0, StatusRai0;

		StatusRa0 = CheckLink(IFNAME_2G);
		StatusRai0 = CheckLink(IFNAME_5G);

		if(StatusRa0 == UP)
		{
			if(StatusRa0 != ra0Status)
				SendToDriver(IFNAME_2G);
			
			ra0Status = UP;
		}	
		else
			ra0Status = Down;

		if(StatusRai0 == UP)
		{
			if(StatusRai0 != rai0Status)
				SendToDriver(IFNAME_5G);
			
			rai0Status = UP;
		}
		else
			rai0Status = Down;

		DBGPRINT(DEBUG_TRACE, "ra0Status : %d, rai0Status : %d \n", ra0Status, rai0Status);
		
		sleep(5);
	}
}

/*
=====================================================================
Description:
	- This is main function.
	- Used for Create thread and call funtion for connection.
	
=====================================================================
*/

void internet_status_process()
{

	pthread_t td;
	
	pthread_create(&td, NULL, Inf_status, NULL);

	while(1)
	{
		if((ra0Status == Down) && (rai0Status == Down))
		{
			DBGPRINT(DEBUG_OFF, "both inf down going sleep ...\n");
			sleep(5);
		}
		else
		{
			if(Connect_IP(SERVER1) == CONNECTED)
			{
				NetStatus = CONNECTED;
			}
			else 
			{
				if(Connect_IP(SERVER2) == CONNECTED)
					NetStatus = CONNECTED;
				else
					NetStatus = DISCONNECTED;
			}
				
			SendToRa0(); 
			SendToRai0();
		}

		sleep(5);
	}

	return ;
}

