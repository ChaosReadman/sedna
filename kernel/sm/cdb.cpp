/*
 * File:  cdb.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "sedna.h"
#include <string>
#include <iostream>
#include "bm_core.h"
#include "sm_globals.h"
#include "blk_mngmt.h"
#include "bm_functions.h"
#include "sm_globals.h"
#include "cdb_globals.h"
#include "sm_functions.h"
#include "pers_heap.h"
#include "plmgr_core.h"
#include "llmgr_core.h"
#include "db_utils.h"
#include "plmgr.h"
#include "d_printf.h"
#include "uhdd.h"
#include "version.h"
#include "tr_debug.h"
#include "trmgr.h"
#include "ugc.h"
#include "gmm.h"
#include "pping.h"

using namespace std;


void create_db(__int64 data_file_max_size, 
               __int64 tmp_file_max_size,
               int data_file_extending_portion,		// in PAGE_SIZE
               int tmp_file_extending_portion,		// in PAGE_SIZE
               int pers_heap_size					// in bytes
              )
{
    // create files
    USECURITY_ATTRIBUTES *sa;	
    if(uCreateSA(&sa, U_SEDNA_DEFAULT_ACCESS_PERMISSIONS_MASK, 0, __sys_call_error)!=0) throw USER_EXCEPTION(SE3060);
    
    string data_file_name = string(db_files_path) + string(db_name) + ".data";
    data_file_handler = uCreateFile(data_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, sa, __sys_call_error);
    if (data_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);

    string tmp_file_name = string(db_files_path) + string(db_name) + ".tmp";
    tmp_file_handler = uCreateFile(tmp_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, sa, __sys_call_error);
    if (tmp_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);

    string ph_file_name = string(db_files_path) + string(db_name) + ".ph";
    UFile ph_file_handler = uCreateFile(ph_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, sa, __sys_call_error);
    if (ph_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);

    string ph_bu_file_name = string(db_files_path) + string(db_name) + ".ph.bu";
    UFile ph_bu_file_handler = uCreateFile(ph_bu_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, sa, __sys_call_error);
    if (ph_bu_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);
    
    // close created files
    if (uCloseFile(data_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4301);

    if (uCloseFile(tmp_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4301);

    if (uCloseFile(ph_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4301);

    if (uCloseFile(ph_bu_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4301);
    
    // set global names (exactly in this place after data/tmp file creation)
    set_global_names();
    set_global_names(db_name);

    event_logger_init(EL_CDB, db_name, SE_EVENT_LOG_SHARED_MEMORY_NAME, SE_EVENT_LOG_SEMAPHORES_NAME);
    elog(EL_LOG, ("Request for database creation"));


    cdb_ugc(db_name);


    if (uDeleteFile(ph_file_name.c_str(), __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4041);
    
    if (uDeleteFile(ph_bu_file_name.c_str(), __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4041);


    open_global_memory_mapping(SE4400);
    get_vmm_region_values();
    close_global_memory_mapping();

    
    //this call is to debug transactions
    CREATE_DEBUG_LOG(db_name);


    // create and initialize pers heap
    if (0 != pers_create(ph_file_name.c_str(), CHARISMA_PH_SHARED_MEMORY_NAME, 
                         PH_ADDRESS_SPACE_START_ADDR, pers_heap_size, sa))
        throw USER_EXCEPTION(SE4302);

    if (pers_open(ph_file_name.c_str(), CHARISMA_PH_SHARED_MEMORY_NAME, 
                  PERS_HEAP_SEMAPHORE_STR, PH_ADDRESS_SPACE_START_ADDR) != 0)
        throw USER_EXCEPTION(SE4302);

    persistent_db_data* pdb = (persistent_db_data*)pers_malloc(sizeof(persistent_db_data));
    
    if (pdb == NULL)
        throw USER_EXCEPTION(SE4303);

    pdb->init();

    if (pers_close() != 0)
        throw USER_EXCEPTION(SE4304);

    //backup persistent heap
    UFile bu_file_handler = uCreateFile(ph_bu_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, sa, __sys_call_error);
    if (bu_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);

    if (uCloseFile(bu_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4305);

    if (uCopyFile(ph_file_name.c_str(), ph_bu_file_name.c_str(), false, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4306);


    mb = (bm_masterblock*)(((__uint32)bm_master_block_buf + MASTER_BLOCK_SIZE) / MASTER_BLOCK_SIZE * MASTER_BLOCK_SIZE);
    // set values for master block
    mb->free_data_blocks = XNULL;
    mb->free_tmp_blocks = XNULL;

    mb->data_file_cur_size = (__int64)PAGE_SIZE;
    mb->tmp_file_cur_size = (__int64)0;

    mb->data_file_max_size = data_file_max_size;
    mb->tmp_file_max_size = tmp_file_max_size;

    mb->data_file_extending_portion = data_file_extending_portion;
    mb->tmp_file_extending_portion = tmp_file_extending_portion;

    mb->indirection_table_free_entry = XNULL;
    mb->pdb = pdb;

    // open files
    data_file_handler = uOpenFile(data_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, __sys_call_error);
    if (data_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);

    tmp_file_handler = uOpenFile(tmp_file_name.c_str(), 0, U_READ_WRITE, U_NO_BUFFERING, __sys_call_error);
    if (tmp_file_handler == U_INVALID_FD)
        throw USER_EXCEPTION(SE4301);


    // write first page to data file, which serves as master block
    char tmp[PAGE_SIZE];

    int number_of_bytes_written = 0;
    int res = uWriteFile(data_file_handler, tmp, PAGE_SIZE, &number_of_bytes_written, __sys_call_error);
     if (res == 0 || number_of_bytes_written != PAGE_SIZE)
        throw USER_EXCEPTION2(SE4045, "data file");

    // flush master block to disk
    xptr tmp_p = XNULL;
    indirection_table_free_entry = &tmp_p;
    flush_master_block(false);

    // close opened files
    if (uCloseFile(data_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4305);

    if (uCloseFile(tmp_file_handler, __sys_call_error) == 0)
        throw USER_EXCEPTION(SE4305);

    if(uReleaseSA(sa, __sys_call_error)!=0) throw USER_EXCEPTION(SE3063);
}

// buffer for physical log
char phys_log_buf[0x100000]; //1MB

void create_phys_log(int phys_log_size)
{
#ifdef PHYS_LOG
  UFile phys_log_handle;
  USECURITY_ATTRIBUTES *sa;
  
  string phys_log_file_name = string(db_files_path) + string(db_name) + ".plog";

  if(uCreateSA(&sa, U_SEDNA_DEFAULT_ACCESS_PERMISSIONS_MASK, 0, __sys_call_error)!=0) throw USER_EXCEPTION(SE3060);
  //create phys log file
  phys_log_handle = uCreateFile(
                            phys_log_file_name.c_str(),
                            0,
                            U_READ_WRITE,
                            U_WRITE_THROUGH,
                            sa,
                            __sys_call_error
                            );


  if (phys_log_handle == U_INVALID_FD)
     throw USER_EXCEPTION2(SE4040, "physical log file");

  
  if(uReleaseSA(sa, __sys_call_error)!=0) throw USER_EXCEPTION(SE3063);
  
  //write the header of phys log
  file_head pl_head;
  int res, sector_size;

  res = uGetDiskSectorSize(&sector_size, db_files_path, __sys_call_error);
  

  if ( res == 0 )
     throw USER_EXCEPTION(SE4051);

  pl_head.version = 0;
  pl_head.next_lsn = sector_size;
  pl_head.prev_lsn = NULL_LSN;
  pl_head.ph_bu_to_ph= true;
  pl_head.last_checkpoint_lsn = NULL_LSN;
  pl_head.cp_num = 0;
  pl_head.is_stopped_successfully = true;
  pl_head.sedna_db_version=SEDNA_DATA_STRUCTURES_VER;

  int nbytes_written;

  res = uWriteFile(
               phys_log_handle,
               &pl_head,
               sizeof(file_head),
               &nbytes_written,
               __sys_call_error
              );


  if ( res == 0 || nbytes_written != sizeof(file_head))
     throw USER_EXCEPTION2(SE4045, "physical log");


  //set the size of the phys log file and 
  //set the headers of sectors to initial state

  res = uSetFilePointer(
                     phys_log_handle,
                     sector_size,
                     NULL,
                     U_FILE_BEGIN,
                     __sys_call_error
                   );

  if ( res == 0 )
     throw USER_EXCEPTION2(SE4046, "physical log");


  int file_rmndr = phys_log_size - sector_size;
  sector_head sect_head;
  sect_head.durable_lsn = NULL_LSN;
  sect_head.version = -1;

  d_printf2("phys_log_size=%d\n", phys_log_size);

  //init remainder of phys log
  int offs = 0;

  //init buf to write in phys log
  while (offs < 0x100000)
  {
    memcpy(((char*)phys_log_buf)+offs, &sect_head, sizeof(sector_head));        
    offs += sector_size;
  }


  //write n times buf to phys log. n is equal to number of Mb of phys log  
  for (int i=0; i<(phys_log_size/0x100000); i++)
  {
    res = uWriteFile(
                 phys_log_handle,
                 phys_log_buf,
                 0x100000,
                 &nbytes_written,
                 __sys_call_error                 
                );

    if (res == 0 || nbytes_written != 0x100000)
      throw SYSTEM_EXCEPTION("Can't write to phys log file");

  }


  res = uSetEndOfFile(phys_log_handle, 0, U_FILE_CURRENT, __sys_call_error);

  if (res == 0)
      throw USER_EXCEPTION2(SE4047, "physical log");

  res = uCloseFile(phys_log_handle, __sys_call_error);

  if ( res == 0 )
      throw USER_EXCEPTION2(SE4043, "physical log");
#endif
}



int main(int argc, char **argv)
{
    program_name_argv_0 = argv[0];
    __int64 data_file_max_size = 0x80000000;	// = 2Gb
    __int64 tmp_file_max_size = 0x80000000;		// = 2Gb
    int data_file_extending_portion = 1600;		// = 100Mb (in pages)
    int tmp_file_extending_portion = 1600;		// = 100Mb (in pages)
    int data_file_initial_size = 1600;			// = 10Mb (in pages)
    int tmp_file_initial_size = 1600;			// = 10Mb (in pages)
    int persistent_heap_size = 0xA00000;		// = 10Mb
//    int phys_log_size = 0xA00000;                       // = 10Mb
//    int phys_log_ext_portion = 0xA00000;                // = 10Mb
    pping_client ppc(5151, EL_SM);
    bool is_ppc_closed = true;


    try {

        SafeMemoryContextInit();
        if (! set_sedna_data(NULL))
            throw USER_EXCEPTION(SE4411);

        if (argc == 1)
           print_cdb_usage();
        else
           setup_cdb_globals(argc,
                                 argv,
                                 data_file_max_size,
                                 tmp_file_max_size,
                                 data_file_extending_portion,
                                 tmp_file_extending_portion,
                                 data_file_initial_size,
                                 tmp_file_initial_size,
                                 persistent_heap_size,
                                 phys_log_size,
                                 phys_log_ext_portion);


        if (_cdb_s_help_ == 1 || _cdb_l_help_ == 1)
        {
           print_cdb_usage();
           return 0;
        } 
        if( _cdb_version_ == 1)
        {
           print_version_and_copyright("Sedna Create Data Base Utility");
           return 0;
        }

#ifdef REQUIRE_ROOT
        if (!uIsAdmin(__sys_call_error)) throw USER_EXCEPTION(SE3064);
#endif

        //////////// CHECK IF THE DATABASE ALREADY EXISTS //////////////////////
        string data_files_path = string(SEDNA_DATA) + string("/data/");
        data_files_path += string(db_name) + "_files";

        string cfg_file_name = string(SEDNA_DATA) + string("/cfg/");
        cfg_file_name += string(db_name) + "_cfg.xml";

        if (uIsFileExist(data_files_path.c_str(), __sys_call_error) || uIsFileExist(cfg_file_name.c_str(), __sys_call_error))
        {
            string reason = "A database with the name '";
            reason +=  string(db_name);
            reason += "' already exists";
            throw USER_EXCEPTION2(SE4307, reason.c_str());
        }
        ////////////////////////////////////////////////////////////////////////

        fprintf(res_os, "Creating a data base (it can take a few minutes)...\n");


        //!!! Now all parameters checked

        try {
             if (uSocketInit(__sys_call_error) == U_SOCKET_ERROR) throw USER_EXCEPTION(SE3001);
             SednaUserException e = USER_EXCEPTION(SE4400);
             ppc.startup(e);
             is_ppc_closed = false;



             create_cfg_file(db_name,
                             max_trs_num,
                             bufs_num,
                             (phys_log_size / 0x100000),
                             (phys_log_ext_portion / 0x100000)
                            );


             create_data_directory();



             create_db(data_file_max_size, 
                       tmp_file_max_size,
                       data_file_extending_portion,
                       tmp_file_extending_portion,
                       persistent_heap_size
                      );



             d_printf1("create_db call successful\n");

             create_phys_log(phys_log_size);

             d_printf1("create_phys_log call successful\n");


             create_logical_log((string(db_files_path) + string(db_name) + ".0llog").c_str(),
                                0,
                                0,
                                NULL_FILE,
                                NULL_LSN,
                                sizeof(logical_log_file_head),
                                true 
                               );

             d_printf1("create_logical_log call successful\n");


             init_checkpoint_sems();


             ll_phys_log_startup(sedna_db_version);
             d_printf1("phys_log_startup call successful\n");

             ll_phys_log_set_phys_log_flag(false);

             ll_phys_log_startup_shared_mem();
             d_printf1("ll_phys_log_startup_shared_mem call successful\n");

             bm_startup();
             d_printf1("sm_startup call successful\n");

             extend_data_file(data_file_initial_size);
             d_printf1("extend_data_file call successful\n");

             extend_tmp_file (tmp_file_initial_size);
             d_printf1("extend_tmp_file call successful\n");

             bm_shutdown();
             d_printf1("sm_shutdown call successful\n");

             ll_phys_log_clear((LONG_LSN)(-1));
             d_printf1("ll_phys_log_clear call successful\n");

             ll_phys_log_set_ph_bu_to_ph(true);
             d_printf1("ll_phys_log_set_ph_bu_to_ph\n");

             ll_phys_log_shutdown();
             d_printf1("phys_log_shutdown call successful\n");

             release_checkpoint_sems();

             int load_res;
             load_res = load_metadata_in_database(db_name);

             if (load_res == 0)
             {
                fprintf(res_os, "The database '%s' has been created successfully\n", db_name);
                fflush(res_os);
             }
             else
                throw USER_EXCEPTION2(SE4211, db_name);

             elog(EL_LOG, ("Request for database creation satisfied"));
             event_logger_release();

             ppc.shutdown();
             is_ppc_closed = true;
             if (uSocketCleanup(__sys_call_error) == U_SOCKET_ERROR) throw USER_EXCEPTION(SE3000);


        } catch (SednaUserException &e) {
             event_logger_release();
             if (!is_ppc_closed) ppc.shutdown();
             cleanup_db(db_name);
             fprintf(stderr, "%s\n", e.getMsg().c_str());
             uSocketCleanup(__sys_call_error);  
             return 1;
        } catch (SednaException &e) {
             cleanup_db(db_name);
             sedna_soft_fault(e, EL_CDB);
        } catch (...) {
             cleanup_db(db_name);
             sedna_soft_fault(EL_CDB);
        }


    } catch (SednaUserException &e) {
        fprintf(stderr, "%s\n", e.getMsg().c_str());
        return 1;
    } catch (SednaException &e) {
        sedna_soft_fault(e, EL_CDB);
    } catch (...) {
        sedna_soft_fault(EL_CDB);
    }


    return 0;
}

