/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hotspot_cli.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot_ctrl.h"
#include "hotspot_cli.h"

static struct hotspot_ctrl *ctrl_conn;

int RTDebugLevel = RT_DEBUG_TRACE;

static int hotspot_cli_open_connection(const char *ctrl_path)
{
	ctrl_conn = hotsot_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		DBGPRINT(RT_DEBUG_ERROR, "hotspot_ctrl_open fail\n");
		return -1;
	}

	return 0;
}

static void hotspot_cli_close_connection(void)
{
	hotspot_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
}

static int _hotspot_ctrl_command(struct hotspot_ctrl *ctrl, const char *cmd, 
													char *rsp, size_t *rsp_len)
{
	int ret;

	if (ctrl_conn == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "Connect to hotspot daemon first\n");
		return -1;
	}
	
	ret = hotspot_ctrl_command(ctrl, cmd, os_strlen(cmd), rsp, rsp_len);

	if (ret == -2) {
		DBGPRINT(RT_DEBUG_ERROR, "Timeout\n");
		return -2;
	} else if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Command fail\n");
		return -1;
	}

	return 0;
}

static int hotspot_cli_cmd_help(struct hotspot_ctrl *ctrl, int argc, char *argv[]);

static struct hotspot_cli_set_param hs_cli_set_params[] = {
	{"btmtoken", "BTM Req Dialog Token", INTEGER_TYPE},
	{"TSF", "BSS Termination TSF", INTEGER_TYPE},	
	{"bss_termination_duration", "BSS Termination Duration", INTEGER_TYPE},
	{"abridged", "Abridged", INTEGER_TYPE},
	{"validity_interval", "Validity Interval", INTEGER_TYPE},
	{"session_information_url", "Session Information URL", STRING_TYPE},
	{"repeat", "Number of Repetitions", INTEGER_TYPE},
	{"interval", "Randomization Interval", INTEGER_TYPE},	
	{"mduration", "Measurement Duration", INTEGER_TYPE},
	{"mode", "Measurement Mode", INTEGER_TYPE},
	{"rep_conditon", "Report Conditon", INTEGER_TYPE},
	{"ref_value", "Reference Value", INTEGER_TYPE},	
	{"detail", "Reporting Detail"
		"1 for no fixed length fields or elements"
		"2 for all fixed length fields and any requested elements in the request IE;"
		"3 for all fixed length fields and elements", INTEGER_TYPE},	
	{"operating_class_id", "Operating Class ID", STRING_TYPE},	
	{"channel_id", "Channel ID", STRING_TYPE},	
	{"request_id", "Request ID", STRING_TYPE},
}; 


static int hotspot_cli_cmd_btm(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"btm [on/off]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=btm %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}


static int hotspot_cli_cmd_qbtm(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"qbtm [peer_mac]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=qbtm %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_set(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[2048]; //cmd[256];
	int i = 0, ret;
	struct hotspot_cli_set_param *param = hs_cli_set_params;
	struct hotspot_cli_set_param *match = NULL;
	char rsp[2048];
	size_t rsp_len = 0;

	//os_memset(cmd, 0, 256);
	os_memset(cmd,0, 2048);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	while (param->param) {
		if (os_strcmp(param->param, argv[2]) == 0) {
			match = param;
			break;
		}
		
		param++;
	}

	if (match) {
		sprintf(&cmd[i], "interface=%s\n", argv[0]);
		i += 11 + os_strlen(argv[0]);

		sprintf(&cmd[i], "cmd=set %s %s", argv[2], argv[3]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown parameter\n");
		return -1;
	}

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}


static int hotspot_cli_cmd_btmreq(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 8)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"btmreq [peer_mac] [bssid] [channel] [preference] [disassoc_timer] [timeout]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=btmreq %s %s %s %s %s %s",
		argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_btmreq_raw(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 5)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"btmreq_raw [peer_mac] [bssid] [channel]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=btmreq_raw %s %s %s",
		argv[2], argv[3], argv[4]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_nrrsp(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 6)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"nrrsp [peer_mac] [bssid] [channel] [token]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=nrrsp %s %s %s %s",
		argv[2], argv[3], argv[4], argv[5]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_nrrsp_raw(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 6)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"nrrsp_raw [peer_mac] [bssid] [channel] [token]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=nrrsp_raw %s %s %s %s",
		argv[2], argv[3], argv[4], argv[5]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_rrm(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"rrm[on/off]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=rrm %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_qrrm(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"qrrm [peer_mac]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=qrrm %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_bcnreq(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 7 && argc != 8)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"bcnreq [peer_mac] [regclass] [channel] [BSSID] [timeout] <ssid>\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	if(argc == 7)
		sprintf(&cmd[i], "cmd=bcnreq %s %s %s %s %s", 
			argv[2], argv[3], argv[4], argv[5], argv[6]);
	else
		sprintf(&cmd[i], "cmd=bcnreq %s %s %s %s %s %s", 
			argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_bcnreq_raw(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 6)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"bcnreq_raw [peer_mac] [regclass] [channel] [ssid]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	
	sprintf(&cmd[i], "cmd=bcnreq_raw %s %s %s %s", 
		argv[2], argv[3], argv[4], argv[5]);
	
	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_nr_by_daemon(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	if (argc != 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
			"The params are error, please follow format: "
			"nrbyd [y/n]\n");
		return -1;
	}
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=nrbyd %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static struct hotspot_cli_cmd hs_cli_cmds[] = {
	{"help", hotspot_cli_cmd_help, "command usage"},
	{"btm [on/off]", hotspot_cli_cmd_btm, "enable or disable btm"},
	{"qbtm [peer_mac]", hotspot_cli_cmd_qbtm, "query peer btm cap"},
	{"set [param] [value]", hotspot_cli_cmd_set, "set kv parameter"},
	{"btmreq [peer_mac] [bssid] [channel] [preference] [disassoc_timer] [timeout]", 
		hotspot_cli_cmd_btmreq,
		"send bss transition request to peer address"},
	{"btmreq_raw [peer_mac] [bssid] [channel]", 
		hotspot_cli_cmd_btmreq_raw,
		"send bss transition request raw data to peer address"},
	{"nrrsp [peer_mac] [bssid] [channel] [token]", 
		hotspot_cli_cmd_nrrsp,
		"send neighbor report response to peer address"},
	{"nrrsp_raw [peer_mac] [bssid] [channel] [token]", 
		hotspot_cli_cmd_nrrsp_raw,
		"send neighbor report response raw data to peer address"},
	{"rrm [on/off]", hotspot_cli_cmd_rrm, "enable or disable rrm"},
	{"qrrm [sta_mac]", hotspot_cli_cmd_qrrm, "query peer rrm cap"},
	{"nrbyd [y/n]", hotspot_cli_cmd_nr_by_daemon, 
		"handle neighbor report request by daemon"},
	{"bcnreq [peer_mac] [regclass] [channel] [BSSID] [timeout] <ssid>", 
		hotspot_cli_cmd_bcnreq, 
		"send beacon request to peer address"},
	{"bcnreq_raw [peer_mac] [regclass] [channel] [ssid]", 
		hotspot_cli_cmd_bcnreq_raw, 
		"send beacon request raw data to peer address"},
};

static int hotspot_cli_cmd_help(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	struct hotspot_cli_cmd *cmd;
	int cmd_num = sizeof(hs_cli_cmds) / sizeof(struct hotspot_cli_cmd);
	int i;

	cmd = hs_cli_cmds;
	DBGPRINT_RAW(RT_DEBUG_OFF, "kv [interface] [cmd] [args]\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "Command Usage:\n");
	
	for (i = 0; i < cmd_num; i++, cmd++) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "  %-60s %-50s\n", cmd->cmd, cmd->usage);
	}

	return 0;
}

static int hotspot_cli_request(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	struct hotspot_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	cmd = hs_cli_cmds;

	while (cmd->cmd) {
		if (os_strncmp(cmd->cmd, argv[1], os_strlen(argv[1])) == 0) {
			match = cmd;
			break;
		}
		cmd++;
	}

	if (match) {
		ret = match->cmd_handler(ctrl, argc, &argv[0]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown command\n");
		ret = -1;
	}

	return ret;
}


int optind = 1;
int main(int argc, char *argv[])
{
	int ret = 0;
	char socket_path[64]={0};
	int i;

	//os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot%s",argv[1]);	
	os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot");

	for(i=0; i<argc; i++)
		DBGPRINT(RT_DEBUG_TRACE, "argv[%d] %s\n",argv[i]);

	hotspot_cli_open_connection(socket_path);

	ret = hotspot_cli_request(ctrl_conn, argc - optind, &argv[optind]);

	hotspot_cli_close_connection();

	return ret;
}
