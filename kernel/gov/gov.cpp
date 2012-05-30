/*
 * File:  gov.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "common/errdbg/d_printf.h"
#include "common/base.h"

#include "gov/cpool.h"
#include "gov/gov_globals.h"
#include "gov/gov_functions.h"
#include "common/procutils/version.h"
#include "u/ugnames.h"
#include "common/globalobjects/sednaregistry.h"
#include "common/globalobjects/globalnames.h"

int main(int argc, char** argv)
{
    program_name_argv_0 = argv[0];
    char buf[1024];

    /* Under Solaris there is no SO_NOSIGPIPE/MSG_NOSIGNAL/SO_NOSIGNAL,
     * so we must block SIGPIPE with sigignore.
     */
#if defined(SunOS)
    sigignore(SIGPIPE);
#endif

    try {
        GlobalParameters sednaGlobalOptions;
        if ( parseSednaOptions(argc, argv, &sednaGlobalOptions, program_name_argv_0) ) { return 1; }

// !FIXME: do we really need SEDNA_DATA in this form?
        
        memcpy(SEDNA_DATA, sednaGlobalOptions.global.dataDirectory.c_str(), sednaGlobalOptions.global.dataDirectory.size());

        check_data_folder_existence();
        RenameLastSoftFaultDir();

        if (uSocketInit(__sys_call_error) == U_SOCKET_ERROR)
            throw SYSTEM_EXCEPTION("Failed to initialize socket library");

// TODO:        InitGlobalNames(os_primitives_id_min_bound,INT_MAX);
//              SetGlobalNames();
      global_name SE_EVENT_LOG_SHM = createSednaGlobalName(GLOBAL_NAME(SE_EVENT_LOG_SHM));
      global_name SE_EVENT_LOG_SEM = createSednaGlobalName(GLOBAL_NAME(SE_EVENT_LOG_SEM));
      
      if (event_logger_start_daemon(el_convert_log_level(sednaGlobalOptions.global.logLevel), SE_EVENT_LOG_SHM, SE_EVENT_LOG_SEM))
          throw SYSTEM_EXCEPTION("Failed to initialize event log");

      log_out_system_information();

//       FIXME: wtf? it's now in trn, do we need this?
//       create_global_memory_mapping(sednaGlobalOptions.global.osObjectsOffset);

      ProcessManager procManager(sednaGlobalOptions);
      Worker * govWorker = new Worker();
      govWorker->createListener();
      govWorker->run();


      if (uSocketCleanup(__sys_call_error) == U_SOCKET_ERROR) throw SYSTEM_EXCEPTION("Failed to clean up socket library");

//       FIXME: wtf? it's now in trn, do we need this?
//       release_global_memory_mapping();

      elog(EL_LOG, ("SEDNA event log is down"));
      event_logger_shutdown_daemon(SE_EVENT_LOG_SHM);


      fprintf(res_os, "GOVERNOR has been shut down successfully\n");
      fflush(res_os);
      return 0;

    } catch (SednaUserException &e) {
        fprintf(stderr, "%s\n", e.what());
        event_logger_release();

        return 1;
    } catch (SednaException &e) {
        sedna_soft_fault(e, EL_GOV);
    } catch (ANY_SE_EXCEPTION) {
        sedna_soft_fault(EL_GOV);
    }

    return 0;
}
