/*
 * File:  uhdd.h
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _UHDD_H
#define _UHDD_H

#include "u.h"
#include "usecurity.h"



#ifdef _WIN32

/* UShareMode */
#define U_SHARE_READ			FILE_SHARE_READ
#define U_SHARE_WRITE			FILE_SHARE_WRITE
/* UAccess */
#define U_WRITE					GENERIC_WRITE
#define U_READ					GENERIC_READ
#define U_READ_WRITE			(GENERIC_READ | GENERIC_WRITE)
#define U_ALL_ACCESS            GENERIC_ALL
/* uCreateFile/UOpenFile error return value */
#define U_INVALID_FD			INVALID_HANDLE_VALUE
/* UFlag (for create/open) */
#define U_WRITE_THROUGH			FILE_FLAG_WRITE_THROUGH
#define U_NO_BUFFERING			FILE_FLAG_NO_BUFFERING
/* UFlag (for setting file pointer) */
#define U_FILE_BEGIN			FILE_BEGIN
#define U_FILE_END				FILE_END
#define U_FILE_CURRENT			FILE_CURRENT


typedef HANDLE UFile;
typedef DWORD UFlag;
typedef DWORD UAccess;
typedef DWORD UShareMode;

#else

/* UShareMode */
#define U_SHARE_READ			0
#define U_SHARE_WRITE			0
/* UAccess */
#define U_WRITE					O_WRONLY
#define U_READ					O_RDONLY
#define U_READ_WRITE			O_RDWR
#define U_ALL_ACCESS            O_RDWR
/* uCreateFile/UOpenFile error return value */
#define U_INVALID_FD			(-1)
/* UFlag (for create/open) */
#define U_WRITE_THROUGH			O_SYNC
/* !!! It has been found that there are some problems with
   !!! O_DIRECT flag in Linux, so it has been temporary substituted wih
   !!! the O_SYNC flag */
/* #define U_NO_BUFFERING                        O_DIRECT */
#define U_NO_BUFFERING			O_SYNC
/* UFlag (for setting file pointer) */
#define U_FILE_BEGIN			SEEK_SET
#define U_FILE_END				SEEK_END
#define U_FILE_CURRENT			SEEK_CUR

typedef int UFile;
typedef int UFlag;
typedef int UAccess;
typedef int UShareMode;

#endif

struct file_struct
{
    UFile f;
    char name[1024];
};

#ifdef __cplusplus
extern "C"
{
#endif

/* return U_INVALID_FD in the case of error*/
    UFile uCreateFile(const char *name, UShareMode share, UAccess accs, UFlag attr, USECURITY_ATTRIBUTES* sa, sys_call_error_fun fun);

/* return U_INVALID_FD in the case of error*/
    UFile uOpenFile(const char *name, UShareMode share, UAccess accs, UFlag attr, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uCloseFile(UFile fd, sys_call_error_fun fun);

/* If the function succeeds to delete the file, it returns nonzero.*/
/* If the function fails to detele file, it returns zero.*/
    int uDeleteFile(const char *name, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uDelDir(const char *dir, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uMoveFile(const char* old_name, const char* new_name, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero. If the return value */
/* is nonzero and the number of bytes read is zero, the file pointer was beyond */
/* the current end of the file at the time of the read operation.*/
/* If the function fails, the return value is zero*/
    int uReadFile(UFile fd, void *buf, int to_read, int *already_read, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uWriteFile(UFile fd, const void *buf, int to_write, int *already_written, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uSetFilePointer(UFile fd, __int64 offs, __int64 * res_pos, UFlag meth, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uSetEndOfFile(UFile fd, __int64 offs, UFlag meth, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uMkDir(const char *name, USECURITY_ATTRIBUTES* sa, sys_call_error_fun fun);

/* Returns true if the file exists*/
    int uIsFileExist(const char *name, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uCopyFile(const char *existing_file, const char *new_file, int fail_if_exists, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uGetFileSize(UFile fd, __int64 * file_size, sys_call_error_fun fun);

/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uGetFileSizeByName(const char* name, __int64 * file_size, sys_call_error_fun fun);


/* If the function succeeds, the return value is nonzero.*/
/* If the function fails, the return value is zero.*/
    int uGetDiskSectorSize(int *sector_size, const char *path, sys_call_error_fun fun);

/* If the function succeeds, it returns 0.*/
/* If the function fails, it returns nonzero.*/
    int uGetUniqueFileStruct(const char *directoryName, struct file_struct *fs, int sid, sys_call_error_fun fun);

/* If the function succeeds, it returns 0.*/
/* If the function fails, it returns nonzero.*/
/* FILE* f - open unique file in the directory directoryName*/
    int uGetUniqueFileName(const char *directoryName, char *file_name, sys_call_error_fun fun);

/* If the function succeeds, the return value is pointer to the absolute path*/
/* If the function fails, the return value is NULL*/
    char *uGetAbsoluteFilePath(const char *relPath, char *absPath, int maxLength, sys_call_error_fun fun);

/* If the function succeeds, the return value is pointer to the absolute path of current working directory*/
/* If the function fails, the return value is NULL*/
    char *uGetCurrentWorkingDirectory(char *buf, int maxLength, sys_call_error_fun fun);

/* If the function succeeds, the return value is 0*/
/* If the function fails, the return value is -1*/
    int uChangeWorkingDirectory(const char *path, sys_call_error_fun fun);

    char *uGetDirectoryFromFilePath(const char *path, char *buf, int buf_len, sys_call_error_fun fun);

    char *uGetFileNameFromFilePath(const char *path, char *buf, int buf_len, sys_call_error_fun fun);

#ifdef __cplusplus
}
#endif

#endif
