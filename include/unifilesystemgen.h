#ifndef __UNIFILESYSTEMGEN_H
#define __UNIFILESYSTEMGEN_H

#include "uniconfgen.h"
#include <sys/types.h>

/**
 * This generator is unfinished, and presently does an assert(0) in its
 * constructor so that no one will use it.
 */

/**
 * Creates a UniConf tree that mirrors some point in the Linux filesystem,
 * with restrictions. The root of the point to be mirrored is specified by
 * giving both a directory name and the name of a file in that directory,
 * which need not exist. Additionally, the mode for creation of all files
 * and directories must be given.
 *
 * UniConf keys are mapped to filesystem paths in the obvious way (delimit
 * and precede the list of names with "/", then append to the path for the
 * point being mirrored).
 *
 * Keys corresponding to regular files have value equal to the content of
 * that file (plus a terminating NUL). Keys whose corresponding pathname
 * does not exist have value equal to the null string. Keys corresponding
 * to things other than regular files have value equal to the empty string,
 * unless their last segment is "." or "..", in which case they have value
 * equal to the null string.
 *
 * Due to these definitions, the UniFileSystemGen violates the UniConfGen
 * semantics in that it is not possible for a key to have simultaneously a
 * non-empty value and children and it is not possible for a key named
 * "." or ".." to exist. Any set operation that by the UniConfGen semantics
 * would cause either of those things to be true will instead do nothing.
 * These shortcomings are permanent.
 *
 * If an unrecoverable error occurs during set, it will fail, but will
 * possibly still have an effect. If an unrecoverable error occurs
 * during get, it will return the null string. If an unrecoverable error
 * occurs during iterator(), it will return a NULL pointer.
 *
 * Presently, callbacks are never triggered.
 *
 * The generator makes no attempt to ensure that the files it reads contain
 * printable strings. If you read a file containing embedded NULs, then the
 * WvString you get will not be the actual content of the files, nor will it
 * necessarily just be the content up to the first NUL.
 *
 * (If you really have to know, it will be the content of the files with every
 * NUL and all the data up to the next 2kB boundary in the file removed. Look
 * at the code to see why.)
 */
class UniFileSystemGen : public UniConfGen
{
public:
    UniFileSystemGen(WvStringParm _dir, WvStringParm _file, mode_t _mode);
    WvString get(const UniConfKey &key);
    void set(const UniConfKey &key, WvStringParm value);
    Iter *iterator(const UniConfKey &key);
    void flush_buffers() {}
private:
    WvString dir, file;
    mode_t mode;
};

#endif
