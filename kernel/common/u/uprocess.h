/*
 * File:  uprocess.h
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */


#ifndef _UPROCESS_H
#define _UPROCESS_H


#include "u.h"
#include "uthread.h"
#include "usecurity.h"


#ifdef _WIN32
#define U_DETACHED_PROCESS			DETACHED_PROCESS

typedef HANDLE UPHANDLE;
typedef DWORD  UPID;
typedef DWORD  UFlag;
#else
#include <signal.h>
#define U_DETACHED_PROCESS			1

typedef int    UPHANDLE;
typedef pid_t  UPID;
typedef int    UFlag;
#endif



#ifdef __cplusplus
extern "C" {
#endif

/* return value 0 indicates success */
int uSetEnvironmentVariable(const char* name, const char* value);

/* return value 0 indicates success */
int uGetEnvironmentVariable(const char* name, char* buf, int size);


/* return value 0 indicates success */
int uCreateProcess(
           char *command_line,		/* command line string */
           bool inherit_handles,	/* handle inheritance option */
           const char *cur_dir,		/* current directory name */
           UFlag flags,
           UPHANDLE *process_handle,
           UTHANDLE *thread_handle,
           UPID *process_id,
           UTID *thread_id,
           USECURITY_ATTRIBUTES* sa
    );

/* return value 0 indicates success */
int uTerminateProcess(UPID pid, UPHANDLE h, int exit_code);
/* momently terminates current process */
void uExitProcess(int exit_code);

UPID uGetCurrentProcessId();
/* return value: 1 - exists, 0 - does not exist, -1 - error */
int uIsProcessExist(UPID pid, UPHANDLE h);

int uOpenProcess(UPID pid, UPHANDLE /*out*/ *h);

int uCloseProcess(UPHANDLE h);

int uWaitForChildProcess(UPID pid, UPHANDLE h, int *status);
int uWaitForProcess(UPID pid, UPHANDLE h);


extern char *program_name_argv_0;
/* The result if written to buf. The size of the buf should be
 * not less than U_MAX_PATH + 1. The function return buf. */
char* uGetImageProcPath(char *buf);

#ifdef __cplusplus
}
#endif


#endif
