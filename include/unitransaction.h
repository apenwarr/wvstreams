/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * Allows one to wrap a UniConf tree with a transaction model.  Use
 * UniTransaction::commit() to commit, and
 * UniTransaction::refresh() to rollback.
 */

#ifndef _UNITRANSACTION_H
#define _UNITRANSACTION_H

#include "unibachelorgen.h"
#include "uniconfroot.h"
#include "unitransactiongen.h"
#include "uniunwrapgen.h"

class UniTransaction : public UniConfRoot
{
    friend class UniConf;
    friend class UniConf::Iter;
    friend class UniConf::RecursiveIter;

public:
    /**
     * Wraps an existing UniConf tree with a transaction generator.
     */
    UniTransaction(const UniConf &base)
	: UniConfRoot(new UniTransactionGen(
			  new UniBachelorGen(
			      new UniUnwrapGen(base))), false)
    {
    }
};

#endif /* _UNITRANSACTION_H */
