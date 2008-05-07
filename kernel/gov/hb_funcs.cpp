
#include "gov/hb_funcs.h"
#include "gov/gov_globals.h"
#include "gov/hb_files.h"

#include "common/base.h"
#include "common/SSMMsg.h"
#include "common/ipc_ops.h"

#include <string>

#define MAX_SE_SOCKET_STR (SE_SOCKET_MSG_BUF_SIZE - 5)

using namespace std;

static string hbDbName;    // name of the db to archive
static hb_state status = HB_START; // HB_START->HB_WAIT (sm wait case)->HB_ARCHIVELOG->HB_NEXTFILE->HB_END
vector <string> hbFiles;

// resets hot-backup structures
static void hbResetState()
{
	hbDbName.clear();
	hbFiles.clear();
	status = HB_START;
}

static void hbSendMsgToSm(sm_msg_struct *msg)
{
	SSMMsg *sm_server = NULL;
	char buf[1024];

	sm_server = new SSMMsg(SSMMsg::Client, 
                           sizeof(sm_msg_struct), 
                           CHARISMA_SSMMSG_SM_ID(get_db_id_by_name(gov_table->get_config_struct(), hbDbName.c_str()), buf, 1024), 
                           SM_NUMBER_OF_SERVER_THREADS);

	if (sm_server->init() != 0) 
	    throw SYSTEM_EXCEPTION("Failed to initialize SSMMsg service (message service)");

	if (sm_server->send_msg(msg) != 0) 
	    throw SYSTEM_EXCEPTION("Can't send message via SSMMsg");

	if (sm_server->shutdown() != 0)
	    throw SYSTEM_EXCEPTION("Failed to shutdown SSMMsg service (message service)");

	delete sm_server;
}

// retrieves db name, check its existence and if sm is running
static int hbRetrieveAndCheckDbName(char *dbname, int len)
{
	hbDbName = string(dbname, len);	
	
	if (!(gov_table->is_database_run(hbDbName.c_str()))) return -1;

	return 0;
}

// retrieves log file names, ph-file name etc. and stores them in hbFiles array
static int RetrieveAllFileNames()
{
    char buf[MAX_SE_SOCKET_STR];
    int lnum;

	// retrieve all log file names
	sm_msg_struct msg;
	
	msg.cmd = 39;
	msg.data.hb_struct.state = HB_ARCHIVELOG;

	hbSendMsgToSm(&msg);

	if (msg.data.hb_struct.state == HB_ERR) return -1;

	while (msg.data.hb_struct.lnumber != -1)
	{
		if (hbMakeLogFileName(buf, MAX_SE_SOCKET_STR, hbDbName.c_str(), msg.data.hb_struct.lnumber) == -1)
			return -1;
		
	    hbFiles.push_back(string(buf, MAX_SE_SOCKET_STR));

		msg.cmd = 39;
		msg.data.hb_struct.state = HB_GETPREVLOG;

		hbSendMsgToSm(&msg);

		if (msg.data.hb_struct.state == HB_ERR) return -1;
	}

	// retrieve ph file name
	msg.cmd = 39;
	msg.data.hb_struct.state = HB_GETPERSTS;
        
	hbSendMsgToSm(&msg);

	if (msg.data.hb_struct.state == HB_ERR) return -1;

	if (hbMakePhFileName(buf, MAX_SE_SOCKET_STR, hbDbName.c_str(), msg.data.hb_struct.ts) == -1)
		return -1;

    hbFiles.push_back(string(buf, MAX_SE_SOCKET_STR));

    // retrieve vmm.dat file
	if (hbMakeVmmFileName(buf, MAX_SE_SOCKET_STR) == -1)
		return -1;

    hbFiles.push_back(string(buf, MAX_SE_SOCKET_STR));

    // retrieve db config file
	if (hbMakeConfFileName(buf, MAX_SE_SOCKET_STR, hbDbName.c_str()) == -1)
		return -1;

    hbFiles.push_back(string(buf, MAX_SE_SOCKET_STR));

    // retrieve sednaconf file
	int res = hbMakeConfGlobalFileName(buf, MAX_SE_SOCKET_STR);
	if (res == -1) return -1;
    if (res != 0) hbFiles.push_back(string(buf, MAX_SE_SOCKET_STR));

    return 0;
}

// process hbp specific error (connection lost or error in hbp)
// do: send HB_ERROR to sm to end hot-backup process, shutdown and close corresponding socket
void hbProcessErrorHbp()
{
	if (status == HB_START) return; // no need to notify sm since hot-back isn't in progress

	sm_msg_struct msg;

	msg.cmd = 39;
	msg.data.hb_struct.state = HB_END;

	hbSendMsgToSm(&msg);

	hbResetState();
}

// processes start message
// can issue wait request or send data file name to hbp
int hbProcessStartRequest(msg_struct *msg)
{
	hb_state req = (hb_state)msg->instruction;
	
	if ((status == HB_WAIT && req == HB_START_CHECKPOINT) || (status != HB_WAIT && status != HB_START))
	{
		hbProcessErrorHbp();
	
		msg->instruction = HB_ERR;
		msg->length = 0;
		
		return -1;
	}

	if (status == HB_START)
	{
		if (msg->length <= 5 || msg->body[0] != 0) return -1;

		__int32 len;
		net_int2int(&len, &(msg->body[1]));

		if (hbRetrieveAndCheckDbName(&(msg->body[5]), len) != 0) return -1;
	}

	sm_msg_struct smmsg;
	
	smmsg.cmd = 39;
	smmsg.data.hb_struct.state = req;

	hbSendMsgToSm(&smmsg);

	if (smmsg.cmd == HB_CONT) // need to send data file name
	{
		status = HB_ARCHIVELOG;
		msg->instruction = HB_CONT;
		
		msg->body[0] = 0;
		
		int len = hbMakeDataFileName(&(msg->body[5]), MAX_SE_SOCKET_STR, hbDbName.c_str());

		if (len == -1) // path doesn't fit into message buffer and that's strange :)
		{
			hbProcessErrorHbp();
	
			msg->instruction = HB_ERR;
			msg->length = 0;
		
			return -1;
		}

		int2net_int(len, &(msg->body[1]));
		msg->length = len + 5;
	}
	else
	{
		msg->instruction = status = smmsg.data.hb_struct.state;
		msg->length = 0;
		
		if (status == HB_ERR) return -1;
	}

	return 0;
}

// processes archive log request
int hbProcessLogArchRequest(msg_struct *msg)
{
	if (status != HB_ARCHIVELOG)
	{
		hbProcessErrorHbp();
	
		msg->instruction = HB_ERR;
		msg->length = 0;
		
		return -1;
	}

	int res = RetrieveAllFileNames();

	if (res == 0 && hbFiles.size() > 0) // need to send file name
	{
		status = HB_NEXTFILE;
		msg->instruction = HB_CONT;
        msg->body[0] = 0;
		strncpy(&(msg->body[5]), hbFiles[0].c_str(), hbFiles[0].length());
		int2net_int(hbFiles[0].length(), &(msg->body[1]));

		msg->length = hbFiles[0].length() + 5;

		hbFiles.erase(hbFiles.begin());

		return 0;
	}
	else
	{
		msg->instruction = status = HB_ERR;
		msg->length = 0;
		
		return -1;
	}
}

// processes next copy file request
static int hbProcessNextFile(msg_struct *msg)
{
	if (status != HB_NEXTFILE)
	{
		hbProcessErrorHbp();
	
		msg->instruction = HB_ERR;
		msg->length = 0;
		
		return -1;
	}

	if (hbFiles.size() > 0)
	{
		msg->instruction = HB_CONT;
        msg->body[0] = 0;
		strncpy(&(msg->body[5]), hbFiles[0].c_str(), hbFiles[0].length());
		int2net_int(hbFiles[0].length(), &(msg->body[1]));

		msg->length = hbFiles[0].length() + 5;

		hbFiles.erase(hbFiles.begin());
	}
	else
	{
		status = HB_END;
		msg->instruction = HB_CONT;
        msg->length = 0;
	}

	return 0;
}

// processes end request from hbp
int hbProcessEndRequest(msg_struct *msg)
{
	if (status != HB_END)
	{
		hbProcessErrorHbp();
	
		msg->instruction = HB_ERR;
		msg->length = 0;
		
		return -1;
	}
	
	sm_msg_struct smsg;

	smsg.cmd = 39;
	smsg.data.hb_struct.state = HB_END;

	hbSendMsgToSm(&smsg);

	if (smsg.data.hb_struct.state == HB_END)
	{
		msg->instruction = HB_END;
		msg->length = 0;
	}
	else
	{
		msg->instruction = HB_ERR;
		msg->length = 0;
		
		return -1;
	}

	return 1;
}

// processes message from hbp
// return: -1 - error; 0 - all ok, cont.; 1 - graceful end requested;
int hbProcessMessage(msg_struct *msg)
{
	hb_state cmd = (hb_state)msg->instruction;
	int res;

    if (cmd == HB_START || cmd == HB_START_CHECKPOINT)
		res = hbProcessStartRequest(msg);
	else if (cmd == HB_ARCHIVELOG)
     	res = hbProcessLogArchRequest(msg);
    else if (cmd == HB_NEXTFILE)
     	res = hbProcessNextFile(msg);
	else if (cmd == HB_END)
		res = hbProcessEndRequest(msg);
	else
	{
		hbProcessErrorHbp();
		res = -1;
	}

	if (res != 0) hbResetState();
	
	return res;
}