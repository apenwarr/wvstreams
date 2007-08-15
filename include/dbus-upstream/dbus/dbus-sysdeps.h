/* -*- mode: C; c-file-style: "gnu" -*- */
/* dbus-sysdeps.h Wrappers around system/libc features (internal to D-BUS implementation)
 * 
 * Copyright (C) 2002, 2003  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
 *
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef DBUS_SYSDEPS_H
#define DBUS_SYSDEPS_H

#include "config.h"

#include <dbus/dbus-errors.h>

/* this is perhaps bogus, but strcmp() etc. are faster if we use the
 * stuff straight out of string.h, so have this here for now.
 */
#include <string.h>

/* and it would just be annoying to abstract this */
#include <errno.h>

DBUS_BEGIN_DECLS

/* The idea of this file is to encapsulate everywhere that we're
 * relying on external libc features, for ease of security
 * auditing. The idea is from vsftpd. This also gives us a chance to
 * make things more convenient to use, e.g.  by reading into a
 * DBusString. Operating system headers aren't intended to be used
 * outside of this file and a limited number of others (such as
 * dbus-memory.c)
 */

typedef struct DBusString DBusString;

#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define _DBUS_GNUC_PRINTF( format_idx, arg_idx )    \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define _DBUS_GNUC_SCANF( format_idx, arg_idx )     \
  __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define _DBUS_GNUC_FORMAT( arg_idx )                \
  __attribute__((__format_arg__ (arg_idx)))
#define _DBUS_GNUC_NORETURN                         \
  __attribute__((__noreturn__))
#else   /* !__GNUC__ */
#define _DBUS_GNUC_PRINTF( format_idx, arg_idx )
#define _DBUS_GNUC_SCANF( format_idx, arg_idx )
#define _DBUS_GNUC_FORMAT( arg_idx )
#define _DBUS_GNUC_NORETURN
#endif  /* !__GNUC__ */

void _dbus_abort (void) _DBUS_GNUC_NORETURN;

const char* _dbus_getenv (const char *varname);
dbus_bool_t _dbus_setenv (const char *varname,
			  const char *value);

int _dbus_read      (int               fd,
                     DBusString       *buffer,
                     int               count);
int _dbus_write     (int               fd,
                     const DBusString *buffer,
                     int               start,
                     int               len);
int _dbus_write_two (int               fd,
                     const DBusString *buffer1,
                     int               start1,
                     int               len1,
                     const DBusString *buffer2,
                     int               start2,
                     int               len2);

typedef unsigned long dbus_pid_t;
typedef unsigned long dbus_uid_t;
typedef unsigned long dbus_gid_t;

#define DBUS_PID_UNSET ((dbus_pid_t) -1)
#define DBUS_UID_UNSET ((dbus_uid_t) -1)
#define DBUS_GID_UNSET ((dbus_gid_t) -1)

#define DBUS_PID_FORMAT "%lu"
#define DBUS_UID_FORMAT "%lu"
#define DBUS_GID_FORMAT "%lu"

/**
 * Struct representing socket credentials
 */
typedef struct
{
  dbus_pid_t pid; /**< process ID or DBUS_PID_UNSET */
  dbus_uid_t uid; /**< user ID or DBUS_UID_UNSET */
  dbus_gid_t gid; /**< group ID or DBUS_GID_UNSET */
} DBusCredentials;

int _dbus_connect_unix_socket (const char     *path,
                               dbus_bool_t     abstract,
                               DBusError      *error);
int _dbus_listen_unix_socket  (const char     *path,
                               dbus_bool_t     abstract,
                               DBusError      *error);
int _dbus_connect_tcp_socket  (const char     *host,
                               dbus_uint32_t   port,
                               DBusError      *error);
int _dbus_listen_tcp_socket   (const char     *host,
                               dbus_uint32_t   port,
                               DBusError      *error);
int _dbus_accept              (int             listen_fd);

dbus_bool_t _dbus_read_credentials_unix_socket (int              client_fd,
                                                DBusCredentials *credentials,
                                                DBusError       *error);
dbus_bool_t _dbus_send_credentials_unix_socket (int              server_fd,
                                                DBusError       *error);


void        _dbus_credentials_clear                (DBusCredentials       *credentials);
void        _dbus_credentials_from_current_process (DBusCredentials       *credentials);
dbus_bool_t _dbus_credentials_match                (const DBusCredentials *expected_credentials,
                                                    const DBusCredentials *provided_credentials);


typedef struct DBusUserInfo  DBusUserInfo;
typedef struct DBusGroupInfo DBusGroupInfo;

/**
 * Information about a UNIX user
 */
struct DBusUserInfo
{
  dbus_uid_t  uid;            /**< UID */
  dbus_gid_t  primary_gid;    /**< GID */
  dbus_gid_t *group_ids;      /**< Groups IDs, *including* above primary group */
  int         n_group_ids;    /**< Size of group IDs array */
  char       *username;       /**< Username */
  char       *homedir;        /**< Home directory */
};

/**
 * Information about a UNIX group
 */
struct DBusGroupInfo
{
  dbus_gid_t  gid;            /**< GID */
  char       *groupname;      /**< Group name */
};

dbus_bool_t _dbus_user_info_fill     (DBusUserInfo     *info,
                                      const DBusString *username,
                                      DBusError        *error);
dbus_bool_t _dbus_user_info_fill_uid (DBusUserInfo     *info,
                                      dbus_uid_t        uid,
                                      DBusError        *error);
void        _dbus_user_info_free     (DBusUserInfo     *info);

dbus_bool_t _dbus_group_info_fill     (DBusGroupInfo    *info,
                                       const DBusString *groupname,
                                       DBusError        *error);
dbus_bool_t _dbus_group_info_fill_gid (DBusGroupInfo    *info,
                                       dbus_gid_t        gid,
                                       DBusError        *error);
void        _dbus_group_info_free     (DBusGroupInfo    *info);


unsigned long _dbus_getpid (void);
dbus_uid_t    _dbus_getuid (void);
dbus_gid_t    _dbus_getgid (void);

typedef struct DBusAtomic DBusAtomic;

/**
 * An atomic integer.
 */
struct DBusAtomic
{
  volatile dbus_int32_t value; /**< Value of the atomic integer. */
};

dbus_int32_t _dbus_atomic_inc (DBusAtomic *atomic);
dbus_int32_t _dbus_atomic_dec (DBusAtomic *atomic);

#define _DBUS_POLLIN      0x0001    /* There is data to read */
#define _DBUS_POLLPRI     0x0002    /* There is urgent data to read */
#define _DBUS_POLLOUT     0x0004    /* Writing now will not block */
#define _DBUS_POLLERR     0x0008    /* Error condition */
#define _DBUS_POLLHUP     0x0010    /* Hung up */
#define _DBUS_POLLNVAL    0x0020    /* Invalid request: fd not open */

/**
 * A portable struct pollfd wrapper. 
 */
typedef struct
{
  int fd;            /**< File descriptor */
  short events;      /**< Events to poll for */
  short revents;     /**< Events that occurred */
} DBusPollFD;

int _dbus_poll (DBusPollFD *fds,
                int         n_fds,
                int         timeout_milliseconds);

void _dbus_sleep_milliseconds (int milliseconds);

void _dbus_get_current_time (long *tv_sec,
                             long *tv_usec);


dbus_bool_t _dbus_file_get_contents   (DBusString       *str,
                                       const DBusString *filename,
                                       DBusError        *error);
dbus_bool_t _dbus_string_save_to_file (const DBusString *str,
                                       const DBusString *filename,
                                       DBusError        *error);

dbus_bool_t    _dbus_create_file_exclusively (const DBusString *filename,
                                              DBusError        *error);
dbus_bool_t    _dbus_delete_file             (const DBusString *filename,
                                              DBusError        *error);
dbus_bool_t    _dbus_create_directory        (const DBusString *filename,
                                              DBusError        *error);
dbus_bool_t    _dbus_delete_directory        (const DBusString *filename,
					      DBusError        *error);

dbus_bool_t _dbus_concat_dir_and_file (DBusString       *dir,
                                       const DBusString *next_component);
dbus_bool_t _dbus_string_get_dirname  (const DBusString *filename,
                                       DBusString       *dirname);
dbus_bool_t _dbus_path_is_absolute    (const DBusString *filename);

typedef struct DBusDirIter DBusDirIter;

DBusDirIter* _dbus_directory_open          (const DBusString *filename,
                                            DBusError        *error);
dbus_bool_t  _dbus_directory_get_next_file (DBusDirIter      *iter,
                                            DBusString       *filename,
                                            DBusError        *error);
void         _dbus_directory_close         (DBusDirIter      *iter);

dbus_bool_t  _dbus_check_dir_is_private_to_user    (DBusString *dir,
                                                    DBusError *error);

void        _dbus_generate_random_bytes_buffer (char       *buffer,
                                                int         n_bytes);
dbus_bool_t _dbus_generate_random_bytes        (DBusString *str,
                                                int         n_bytes);
dbus_bool_t _dbus_generate_random_ascii        (DBusString *str,
                                                int         n_bytes);

const char *_dbus_errno_to_string  (int errnum);
const char* _dbus_error_from_errno (int error_number);

void _dbus_disable_sigpipe (void);

void _dbus_fd_set_close_on_exec (int fd);

void _dbus_exit (int code) _DBUS_GNUC_NORETURN;

/**
 * Portable struct with stat() results
 */
typedef struct
{
  unsigned long mode;  /**< File mode */
  unsigned long nlink; /**< Number of hard links */
  dbus_uid_t    uid;   /**< User owning file */
  dbus_gid_t    gid;   /**< Group owning file */
  unsigned long size;  /**< Size of file */
  unsigned long atime; /**< Access time */
  unsigned long mtime; /**< Modify time */
  unsigned long ctime; /**< Creation time */
} DBusStat;

dbus_bool_t _dbus_stat             (const DBusString *filename,
                                    DBusStat         *statbuf,
                                    DBusError        *error);
dbus_bool_t _dbus_full_duplex_pipe (int              *fd1,
                                    int              *fd2,
                                    dbus_bool_t       blocking,
                                    DBusError        *error);
dbus_bool_t _dbus_close            (int               fd,
                                    DBusError        *error);

void        _dbus_print_backtrace  (void);

dbus_bool_t _dbus_become_daemon   (const DBusString *pidfile,
				   int               print_pid_fd,
                                   DBusError        *error);
dbus_bool_t _dbus_write_pid_file  (const DBusString *filename,
                                   unsigned long     pid,
                                   DBusError        *error);
dbus_bool_t _dbus_change_identity (unsigned long     uid,
                                   unsigned long     gid,
                                   DBusError        *error);

typedef void (* DBusSignalHandler) (int sig);

void _dbus_set_signal_handler (int               sig,
                               DBusSignalHandler handler);

dbus_bool_t _dbus_file_exists     (const char *file);
dbus_bool_t _dbus_user_at_console (const char *username,
                                   DBusError  *error);

/* Define DBUS_VA_COPY() to do the right thing for copying va_list variables. 
 * config.h may have already defined DBUS_VA_COPY as va_copy or __va_copy. 
 */
#if !defined (DBUS_VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#    define DBUS_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))
#  elif defined (DBUS_VA_COPY_AS_ARRAY)
#    define DBUS_VA_COPY(ap1, ap2)   memcpy ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#    define DBUS_VA_COPY(ap1, ap2)   ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !DBUS_VA_COPY */

/* On x86 there is an 80-bit FPU, and if you do "a == b" it may have a
 * or b in an 80-bit register, thus failing to compare the two 64-bit
 * doubles for bitwise equality.
 */
#define _DBUS_BYTE_OF_PRIMITIVE(p, i) \
    (((const char*)&(p))[(i)])
#define _DBUS_DOUBLES_BITWISE_EQUAL(a, b)                                       \
     (_DBUS_BYTE_OF_PRIMITIVE (a, 0) == _DBUS_BYTE_OF_PRIMITIVE (b, 0) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 1) == _DBUS_BYTE_OF_PRIMITIVE (b, 1) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 2) == _DBUS_BYTE_OF_PRIMITIVE (b, 2) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 3) == _DBUS_BYTE_OF_PRIMITIVE (b, 3) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 4) == _DBUS_BYTE_OF_PRIMITIVE (b, 4) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 5) == _DBUS_BYTE_OF_PRIMITIVE (b, 5) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 6) == _DBUS_BYTE_OF_PRIMITIVE (b, 6) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 7) == _DBUS_BYTE_OF_PRIMITIVE (b, 7))

dbus_bool_t _dbus_parse_uid (const DBusString  *uid_str,
                             dbus_uid_t        *uid);

DBUS_END_DECLS

#endif /* DBUS_SYSDEPS_H */
