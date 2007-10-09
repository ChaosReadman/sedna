/*
 * File:  locks.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "common/sedna.h"
#include <string>
#include <string.h>

#include "tr/vmm/vmm.h"
#include "tr/locks/locks.h"
#include "common/lm_base.h"
#include "common/u/usem.h"
#include "common/utils.h"
#include "common/SSMMsg.h"
#include "tr/tr_globals.h"
#include "common/errdbg/d_printf.h"

/// #define SE_LOCK_TRACK

using namespace std;

bool is_stop_session();//declared in tr_functions.cpp

LocalLockMgr *local_lock_mrg;

bool is_init_lock_mgr = false;

void lockWrite(node_blk_hdr* block)
{
    VMM_SIGNAL_MODIFICATION(ADDR2XPTR(block));
}

void lockWriteXptr(xptr block)
{
	VMM_SIGNAL_MODIFICATION(block);
}

void lockRead(node_blk_hdr* block)
{
}

void init_local_lock_mgr(SSMMsg* _sm_server_)
{
  local_lock_mrg = se_new LocalLockMgr();
  local_lock_mrg->Init_LocalLockMgr(_sm_server_);
  is_init_lock_mgr = true;
}

void set_tr_mode_lock_mgr(bool flag)
{
  if (is_init_lock_mgr)
  {
     local_lock_mrg->ro_mode(flag);
  }
}

void release_locks()
{
  if (is_init_lock_mgr)
  {
     local_lock_mrg->release();//release all locks acquired by transaction
  }
}

void release_resource(const char* name, resource_kind kind)
{
  if (is_init_lock_mgr)
  {
     
     local_lock_mrg->release_resource(name, kind);//release one resource
  }
}

void release_local_lock_mgr()
{
  if (is_init_lock_mgr == true)
  {
     local_lock_mrg->Release_LocalLockMgr();
     delete local_lock_mrg;
  }

  is_init_lock_mgr = false;
}



/****************************************************************************
                    Local Manager Class implementation
*****************************************************************************/


void LocalLockMgr::Init_LocalLockMgr(SSMMsg* _sm_server_)
{
#ifdef LOCK_MGR_ON
  char buf[1024];

  if ( 0 != USemaphoreCreate(&sem, 0, 1, SEDNA_TRANSACTION_LOCK(sid, ((gov_config_struct*)gov_shm_pointer)->gov_vars.os_primitives_id_min_bound, buf, 1024), NULL, __sys_call_error))
     throw USER_EXCEPTION2(SE4010, "SEDNA_TRANSACTION_LOCK");

  sm_server = _sm_server_;
#endif
}



void LocalLockMgr::Release_LocalLockMgr()
{
#ifdef LOCK_MGR_ON
  if (0 != USemaphoreRelease(sem, __sys_call_error))
     throw USER_EXCEPTION2(SE4011, "SEDNA_TRANSACTION_LOCK");
#endif
}



void LocalLockMgr::put_lock_on_document(const char *name)
{
#ifdef LOCK_MGR_ON
  if(strlen(name) > (MAX_RESOURCE_NAME_LENGTH - 1) )
     throw USER_EXCEPTION(SE4702);

  obtain_lock(db_name, LM_DATABASE, true);
  obtain_lock(name, LM_DOCUMENT);
#endif
}

void LocalLockMgr::put_lock_on_collection(const char *name)
{
#ifdef LOCK_MGR_ON
  if(strlen(name) > (MAX_RESOURCE_NAME_LENGTH - 1) )
     throw USER_EXCEPTION(SE4702);

  obtain_lock(db_name, LM_DATABASE, true);
  obtain_lock(name, LM_COLLECTION);
#endif
}

void LocalLockMgr::put_lock_on_index(const char *name)
{
#ifdef LOCK_MGR_ON
  if(strlen(name) > (MAX_RESOURCE_NAME_LENGTH - 1) )
     throw USER_EXCEPTION(SE4702);

  obtain_lock(db_name, LM_DATABASE, true);
  obtain_lock(name, LM_INDEX);
#endif
}

void LocalLockMgr::put_lock_on_trigger(const char *name)
{
#ifdef LOCK_MGR_ON
  if(strlen(name) > (MAX_RESOURCE_NAME_LENGTH - 1) )
     throw USER_EXCEPTION(SE4702);

  obtain_lock(db_name, LM_DATABASE, true);
  obtain_lock(name, LM_TRIGGER);
#endif
}

    
void LocalLockMgr::put_lock_on_db()
{
#ifdef LOCK_MGR_ON
  if(strlen(db_name) > (MAX_RESOURCE_NAME_LENGTH - 1))
     throw USER_EXCEPTION(SE4702);

  obtain_lock(db_name, LM_DATABASE);
#endif
}


void LocalLockMgr::obtain_lock(const char* name, resource_kind kind, bool intention_mode)
{
#ifdef LOCK_MGR_ON
  if (tr_ro_mode && mode == lm_x)  // cannot acquire eXclusive locks in RO-mode
      throw USER_EXCEPTION(SE4706);

  if (tr_ro_mode) return; // we don't need any locks for ro-transaction
  
  sm_msg_struct msg;

  msg.cmd = 3;
  msg.trid = trid;
  msg.sid = sid;
  int res;

  if (intention_mode == false)
     msg.data.data[0] = ((mode == lm_s) ? 's' : 'x');
  else
     msg.data.data[0] = ((mode == lm_s) ? 'r' : 'w'); //'r' intention read; 'w' intention write


  msg.data.data[1] = (kind == LM_DOCUMENT) ? 'd' : ((kind == LM_COLLECTION)? 'c': ((kind == LM_INDEX)? 'i': ((kind == LM_TRIGGER)? 't': 'b')));
 
  strcpy((msg.data.data)+2, name);

  //d_printf2("lock msg=%s\n", msg.data.data);
  d_printf2("Getting lock on resource=%s...", (msg.data.data)+2);

  if (sm_server->send_msg(&msg) != 0)
      throw USER_EXCEPTION(SE3034);

  d_printf1("OK\n");
 
#ifdef SE_LOCK_TRACK
  if (intention_mode == false)
  {
      elog(EL_LOG, ("[LTRK] Resource '%s' (%c) has been locked with mode = %c.", name, msg.data.data[1], ((mode == 1) ? 's': 'x')));
  }
  else
  {
      elog(EL_LOG, ("[LTRK] Resource '%s' (%c) has been locked with intention mode=%s.", name, msg.data.data[1], ((mode == 1) ? "is" : "ix")));
  }
#endif /* SE_LOCK_TRACK */

  switch(msg.data.data[0])
  {
    case '0': 
    {
      d_printf1("Transaction is blocked\n");
      for (;;)
      {
         res = USemaphoreDownTimeout(sem, 1000, __sys_call_error);
         if (res == 0) 
            break;//unblocked
         else if (res == 2)
         {
            if (is_stop_session()) throw USER_EXCEPTION(SE4608);
            else continue;
         }
         else throw USER_EXCEPTION2(SE4015, "SEDNA_TRANSACTION_LOCK");

      }
      d_printf1("Transaction is unblocked\n");
      break;
    }

    case '1': 
    {
      d_printf1("Transaction has got locks\n");
      break;
    }
      
    case '2':
      throw USER_EXCEPTION(SE4703);
 
    default:
    {
      d_printf2("Unknown reply from Lock Manager: %c\n", msg.data.data[0]);
      throw USER_EXCEPTION(SE4704);     
      break;
    }
  }
#endif
} 



void LocalLockMgr::release()
{
#ifdef LOCK_MGR_ON
  if (tr_ro_mode) return; // we don't need to release any locks for ro-transaction

  sm_msg_struct msg;
  msg.cmd = 4;
  msg.trid = trid;

  d_printf1("\nRelease locks call\n");

  if (sm_server->send_msg(&msg) != 0)
      throw USER_EXCEPTION(SE3034);

#ifdef SE_LOCK_TRACK
  elog(EL_LOG, ("[LTRK] All resources have been released."));
#endif /* SE_LOCK_TRACK */

#endif
}

void LocalLockMgr::release_resource(const char* name, resource_kind kind)
{
#ifdef LOCK_MGR_ON
  if (tr_ro_mode) return; // we don't need to release any locks for ro-transaction

  sm_msg_struct msg;
  msg.cmd = 5;
  msg.trid = trid;

  msg.data.data[1] = (kind == LM_DOCUMENT) ? 'd' : ((kind == LM_COLLECTION)? 'c': ((kind == LM_INDEX)? 'i': ((kind == LM_TRIGGER)? 't': 'b')));
 
  strcpy((msg.data.data)+2, name);

  if (sm_server->send_msg(&msg) != 0)
      throw USER_EXCEPTION(SE3034);

#ifdef SE_LOCK_TRACK
  elog(EL_LOG, ("[LTRK] Resource '%s' (%c) has been released.", name, msg.data.data[1]));
#endif /* SE_LOCK_TRACK */

#endif
}