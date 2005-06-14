/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * Allows one to wrap a UniConf tree with a transaction model.  Use
 * UniConfTransaction::commit() to commit, and
 * UniConfTransaction::refresh() to rollback.
 */

#ifndef _UNICONFTRANSACTION_H
#define _UNICONFTRANSACTION_H

#include "uniconfroot.h"
#include "unitransactiongen.h"
#include "uniunwrapgen.h"

class UniConfTransaction : public UniConfRoot
{
    friend class UniConf;
    friend class UniConf::Iter;
    friend class UniConf::RecursiveIter;

public:
    /**
     * Wraps an existing UniConf tree with 
     */
    UniConfTransaction(const UniConf &base, bool refresh = true)
	: UniConfRoot(new UniTransactionGen(new UniUnwrapGen(base)))
    {
    }

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfTransaction()
    {
    }

};

#endif /* _UNICONFTRANSACTION_H */
