/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * WvEditor: a template to handle interactive, differential editing of
 * a sequence of items.  It is intended to be the 
 */
#ifndef __WVEDITOR_H
#define __WVEDITOR_H

#include "wvtypetraits.h"
#include "wvsorter.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>

inline void dump_indent(FILE *file, size_t level)
{
    if (level >= 1)
        fprintf(file, "%*s", level, " ");
}

inline void dump_charbuf(size_t size, const char *data, FILE *file, size_t level)
{
    size_t i;
    dump_indent(file, level);
    fprintf(file, "[%p] ", (unsigned)data);
    for (i=0; i<size; ++i)
    {
        if (isprint(data[i]))
            fprintf(file, "%c", data[i]);
        else
            fprintf(file, "\\x%02X", (unsigned)data[i]);
    }
    fprintf(file, "\n");
}

/**
 * @internal
 * The untyped base class of WvEditor<T>.
 * 
 * Putting common code in here allows us to prevent it from being
 * replicated by each template instantiation of WvEditor<T>.
 */
template<class T>
class WvEditor
{
    
    struct StoreRef
    {
    	private:
    	
    	    class Store
    	    {
    	    	private:
        	    
    	    	    size_t _size;
    	    	    T *_data;
    	    	    size_t _ref_count;
        	    	
    	    	public:
        	    
    	    	    Store(size_t size) :
    	    	    	_size(size),
    	    	    	_data(new T[size]),
    	    	    	_ref_count(0)
    	    	    {
    	    	    }
    	    	    ~Store()
    	    	    {
    	    	    	assert(_ref_count == 0);
    	    	    	deletev _data;
    	    	    }    	    
            	    
    	    	    size_t size() const
    	    	    {
    	    	    	return _size;
    	    	    }
            	    
    	    	    T &operator [](size_t ind)
    	    	    {
    	    	    	assert(ind >=0 && ind < _size);
    	    	    	return _data[ind];
    	    	    }
    	    	    const T &operator [](size_t ind) const
    	    	    {
    	    	    	assert(ind >=0 && ind < size);
    	    	    	return _data[ind];
    	    	    }
            	    
    	    	    void inc_ref_count()
    	    	    {
    	    	    	if (_ref_count < UINT_MAX)
    	    	    	    ++_ref_count;
    	    	    }
    	    	    void dec_ref_count()
    	    	    {
    	    	    	if (_ref_count > 0)
    	    	    	    --_ref_count;
    	    	    }
    	    	    size_t ref_count() const
    	    	    {
    	    	    	return _ref_count;
    	    	    }

    	    	    void dump(size_t start, size_t len, FILE *file, size_t level)
    	    	    {
    	    	    	dump_indent(file, level);
    	    	    	fprintf(file, "Store(%p): _size=%u, _ref_count=%u, _data[%u..%u]=\n",
            	    	    (unsigned)this,
            	    	    (unsigned)_size, (unsigned)_ref_count,
            	    	    (unsigned)start, (unsigned)(start + len - 1));
    	    	    	dump_charbuf(len, &_data[start], file, level+1);
    	    	    }
    	    } *_store;
    	    size_t _start, _len;
    	    static const size_t _pre_buf = 8, _post_buf = 32;
        
        protected:
        
            void deref()
    	    {
    	    	if (_store)
    	    	{
    	    	    _store->dec_ref_count();
    	    	    if (_store->ref_count() == 0)
            	    	delete _store;
            	    _store = 0;
            	}
            }
            
            void copy_from(const StoreRef &store_ref)
            {
            	_store = store_ref._store;
            	_store->inc_ref_count();
            	_start = store_ref._start;
            	_len = store_ref._len;
            }

        public:
        
            StoreRef() : _store(0)
            {
            }
        	
    	    StoreRef(const StoreRef &store_ref)
    	    {
    	    	copy_from(store_ref);
    	    }
    	    StoreRef &operator =(const StoreRef &store_ref)
    	    {
    	    	deref();
    	    	copy_from(store_ref);
    	    	return *this;
    	    }
        	
    	    ~StoreRef()
    	    {
    	    	deref();
    	    }
            
    	    void contract(size_t pre, size_t post)
    	    {
    	    	assert(pre + post <= _len);
            
    	    	_start += pre;
    	    	_len -= pre + post;
    	    }
            
    	    void expand(size_t pre, size_t post)
    	    {
    	    	assert(pre <= _start);
    	    	assert(_store->size() >= _len + pre + post);
            
    	    	_start -= pre;
    	    	_len += pre + post;
    	    }
    	    
    	    void copy_data(size_t size, const T *data)
    	    {
            	_store = new Store(_pre_buf + size + _post_buf);
            	_store->inc_ref_count();
                
    	    	_start = _pre_buf;
    	    	_len = size;
    	    	
    	    	size_t i;
            	for (i=0; i<size; ++i)
            	    (*_store)[_pre_buf+i] = data[i];
    	    }
    	    
    	    void split(size_t pos, StoreRef &after)
    	    {
    	    	assert(pos >= _start);
    	    	assert(pos < _start + _len);
    
    	    	after.deref();
    	    	after.copy_from(*this);
    	    	
    	    	_len = pos - _start;
    	    
            	after._start = pos;
            	after._len -= _len;
    	    }
    	    
    	    T &operator[](size_t pos)
    	    {
    	    	assert(pos >= _start);
    	    	assert(pos < _start + _len);
    	    	return (*_store)[pos];
    	    }
    	    const T &operator[](size_t pos) const
    	    {
    	    	assert(pos >= _start);
    	    	assert(pos < _start + _len);
    	    	return (*_store)[pos];
    	    }
    	    size_t start() const
    	    {
    	    	return _start;
    	    }
    	    size_t len() const
    	    {
    	    	return _len;
    	    }
    	    
    	    size_t store_size() const
    	    {
    	    	assert(_store);
    	    	return _store->size();
    	    }
    	    size_t store_ref_count() const
    	    {
    	    	assert(_store);
    	    	return _store->ref_count();
    	    }
    	    
    	    bool can_combine_with(const StoreRef &next)
    	    {
    	    	return _store == next._store
    	    	    && _start + _len == next._start;
    	    }
    	    void combine_with(StoreRef &next)
    	    {
    	    	_len += next._len;
    	    	next.deref();
    	    }
    	    	    
    	    void dump(FILE *file, size_t level)
    	    {
    	    	dump_indent(file, level);
    	    	fprintf(file, "StoreRef(%p): start=%u, len=%u, store=\n",
            	    (unsigned)this, (unsigned)_start, (unsigned)_len);
    	    	_store->dump(_start, _len, file, level+1);
    	    }
    };

    size_t _count;
    size_t _num_store_refs;
    StoreRef *_store_refs;

protected:    
            
    void copy_from(const WvEditor<T> &l)
    {
	_count = l._count;
	_num_store_refs = l._num_store_refs;

	_store_refs = new StoreRef[_num_store_refs];
	
    	size_t i;
	for (i=0; i<_num_store_refs; ++i)
	    _store_refs[i] = l._store_refs[i];
    }
    
public:

    WvEditor<T>()
    {
    	_count = _num_store_refs = 0;
    	_store_refs = NULL;
    }
    WvEditor<T>(size_t size, const T *data)
    {
    	_count = size;
    	_num_store_refs = 1;
    	_store_refs = new StoreRef[1];
    	_store_refs[1].copy_data(size, data);
    }
    
    ~WvEditor<T>()
    {
    	if (_store_refs)
    	    deletev _store_refs;
    }

    WvEditor<T>(const WvEditor<T> &l)
    {
    	copy_from(l);
    }
    WvEditor<T>& operator =(const WvEditor<T> &l)
    {
    	copy_from(l);
    	return *this;
    }

    void remove(size_t start, size_t len)
    {
    	size_t i=0, j=0, k;
    
    	while (len > 0)
    	{
            size_t del_start, del_len;
    
            for (; i < _num_store_refs && j + _store_refs[i].len() <= start;
                    j += _store_refs[i++].len()) ;
            if (i == _num_store_refs)
            	break;
    
            del_start = _store_refs[i].start() + start - j;
            del_len = _store_refs[i].len() - (start - j);
            if (len < del_len)
            	del_len = len;
    
            if (del_len == _store_refs[i].len())
            {
            	StoreRef *_new_store_refs = new StoreRef[_num_store_refs+1];
            	for (k=0; k<i; ++k)
            	    _new_store_refs[k] = _store_refs[k];
            	for (; k<_num_store_refs-1; ++k)
            	    _new_store_refs[k] = _store_refs[k+1];
            	deletev _store_refs;
            	_store_refs = _new_store_refs;
            	--_num_store_refs;
            }
            else if (del_start == _store_refs[i].start())
            {
            	_store_refs[i].contract(del_len, 0);
            }
            else if (del_start + del_len == _store_refs[i].start() + _store_refs[i].len())
            {
            	_store_refs[i].contract(0, del_len);
            }
            else
            {
            	StoreRef *_new_store_refs = new StoreRef[_num_store_refs+1];
            	for (k=_num_store_refs; k>i; --k)
            	    _new_store_refs[k] = _store_refs[k-1];
            	_new_store_refs[i+1].contract(del_start - _store_refs[i].start() + del_len, 0);
            	_new_store_refs[i] = _store_refs[i];
            	_new_store_refs[i].contract(0, _store_refs[i].len() - (del_start - _store_refs[i].start()));
            	for (; k>0; --k)
            	    _new_store_refs[k-1] = _store_refs[k-1];
            	deletev _store_refs;
            	_store_refs = _new_store_refs;
            	++_num_store_refs;
            }
    
            len -= del_len;
            _count -= del_len;
    	}
    }

    void insert(size_t pos, const WvEditor<T> &editor)
    {
    	size_t i, j, k;
    
    	if (pos > _count)
            pos = _count;
    
    	for (i=0, j=0; i < _num_store_refs && j + _store_refs[i].len() <= pos;
            	j += _store_refs[i++].len()) ;
    
    	StoreRef *_new_store_refs;
    	if (j == pos)
    	{
    	    _new_store_refs = new StoreRef[_num_store_refs + editor._num_store_refs];
    	    for (k=0; k<i; ++k)
    	    	_new_store_refs[k] = _store_refs[k];
    	    for (; k<i+editor._num_store_refs; ++k)
    	    	_new_store_refs[k] = editor._store_refs[k-i];
    	    for (; k<_num_store_refs+editor._num_store_refs; ++k)
    	    	_new_store_refs[k] = _store_refs[k-editor._num_store_refs];
    	    _num_store_refs += editor._num_store_refs;
    	    deletev _store_refs;
    	    _store_refs = _new_store_refs;
    	    _count += editor._count;
    	}
    	else
    	{
    	    _new_store_refs = new StoreRef[_num_store_refs + editor._num_store_refs+1];
    	    for (k=0; k<=i; ++k)
    	    	_new_store_refs[k] = _store_refs[k];
    	    _new_store_refs[i].split(_store_refs[i].start() + pos - j,
    	    	    _new_store_refs[i+1+editor._num_store_refs]);
    	    for (k=0; k<editor._num_store_refs; ++k)
    	    	_new_store_refs[i+1+k] = editor._store_refs[k];
    	    for (k=i+1; k<_num_store_refs; ++k)
    	    	_new_store_refs[editor._num_store_refs+1+k] = _store_refs[k];
    	    _num_store_refs += editor._num_store_refs + 1;
    	}
    	deletev _store_refs;
    	_store_refs = _new_store_refs;
    	_count += editor._count;
    }

    void insert(size_t pos, size_t size, const T *data)
    {
    	size_t i, j, k;
    
    	if (pos > _count)
            pos = _count;
    
    	for (i=0, j=0; i < _num_store_refs && j + _store_refs[i].len() <= pos;
            	j += _store_refs[i++].len()) ;
    
    	if (j == pos)
    	{
            if (i < _num_store_refs
                    && _store_refs[i].store_ref_count() == 1
                    && _store_refs[i].start() >= size)
            {
            	_store_refs[i].expand(size, 0);
            	for (k=0; k<size; ++k)
            	    _store_refs[i][_store_refs[i].start()+k] = data[k];
            }
            else if (i > 0
                    && _store_refs[i-1].store_ref_count() == 1
                    && _store_refs[i-1].store_size()
                    	- (_store_refs[i-1].start() + _store_refs[i-1].len()) >= size)
            {
            	_store_refs[i-1].expand(0, size);
            	for (k=0; k<size; ++k)
            	    _store_refs[i-1][_store_refs[i].start()+_store_refs[i-1].len()-size+k] = data[k];
            }
            else
            {
            	StoreRef *_new_store_refs = new StoreRef[_num_store_refs+1];
    	    	for (k=0; k<i; ++k)
    	    	    _new_store_refs[k] = _store_refs[k];
    	    	_new_store_refs[k++].copy_data(size, data);
    	    	for (; k<_num_store_refs+1; ++k)
    	    	    _new_store_refs[k] = _store_refs[k-1];
    	    	deletev _store_refs;
    	    	_store_refs = _new_store_refs;
            	++_num_store_refs;
            }
    	}
    	else
    	{
    	    StoreRef *_new_store_refs = new StoreRef[_num_store_refs + 2];
    	    for (k=0; k<i; ++k)
    	    	_new_store_refs[k] = _store_refs[k];
    	    _new_store_refs[i] = _store_refs[i];
    	    _new_store_refs[i].split(_store_refs[i].start() + pos - j,
    	    	    _new_store_refs[i+2]);
    	    _new_store_refs[i+1].copy_data(size, data);
    	    for (k=i+3; k<_num_store_refs+2; ++k)
    	    	_new_store_refs[k] = _store_refs[k-2];
    	    deletev _store_refs;
    	    _store_refs = _new_store_refs;
            _num_store_refs += 2;
        }
    
    	_count += size;
    }

    void split(size_t pos, WvEditor<T> &after)
    {
	if (pos > _count)
	    pos = _count;
    
    	if (after._store_refs)
    	    deletev after._store_refs;
    	    
    	if (pos == 0)
    	{
    	    after._store_refs = _store_refs;
    	    after._num_store_refs = _num_store_refs;
    	    after._count = _count;
    	    
    	    _store_refs = 0;
    	    _num_store_refs = 0;
    	    _count = 0;
    	}
    	else
    	{
            size_t i, j, k;
    
            for (i=0, j=0; i < _num_store_refs && j + _store_refs[i].len() <= pos;
                    j += _store_refs[i++].len()) ;
    
            after._count = _count - pos;
            after._num_store_refs = _num_store_refs - i;
            if (with._count > 0)
            {
            	after._store_refs = new StoreRef[after._num_store_refs];
            	if (j == pos)
            	{
            	    for (k=0; k<after._num_store_refs; ++k)
            	    	after._store_refs[k] = _store_refs[i+k];
            	    // We could shrink _store_refs here but C++ makes that
            	    // awfully hard
                    _count = pos;
                    _num_store_refs = i;
            	}
            	else
            	{
            	    for (k=0; k<after._num_store_refs-1; ++k)
            	    	after._store_refs[k+1] = _store_refs[i+1+k];
            	    _store_refs[i].split(_store_refs[i].start() + pos - j,
            	    	    after._store_refs[0]);
            	    // We could shrink _store_refs here but C++ makes that
            	    // awfully hard
                    _count = pos;
                    _num_store_refs = i + 1;
            	}
            }
    	}
    }
    
    void join(WvEditor<T> &with)
    {
    	if (with._count == 0)
    	{
            /* Nothing to do! */
    	}
    	else if(_count == 0)
    	{
    	    _count = with._count;
    	    _num_store_refs = with._num_store_refs;
    	    _store_refs = with._store_refs;
    	    
    	    with._count = 0;
    	    with._num_store_refs = 0;
    	    with._store_refs = 0;
    	}
    	else if (_store_refs[_num_store_refs-1].can_combine_with(after._store_refs[0]))
    	{
    	    StoreRefs *_new_store_refs =
    	    	    new StoreRef[_num_store_refs + with._num_store_refs - 1];
    	    for (k=0; k<_num_store_refs; ++k)
    	    	_new_store_refs[k] = _store_refs[k];
    	    _new_store_refs[_num_store_refs-1].combine_with(with._store_refs[0]);
    	    for (k=1; k<=with._num_store_refs; ++k)
    	    	_new_store_refs[_num_store_refs+k] = with._store_refs[k];
    	    deletev _store_refs;
    	    _store_refs = _new_store_refs;
    	    _num_store_refs += with._num_store_refs - 1;
    	    _count += with._count;
    	    
    	    with._num_store_refs = 0;
    	    deletev with._store_refs;
    	    with._count = 0;
    	}
    	else
    	{
    	    StoreRefs *_new_store_refs =
    	    	    new StoreRef[_num_store_refs + with._num_store_refs];
    	    for (k=0; k<with._num_store_refs; ++k)
    	    	_new_store_refs[k] = with._store_refs[k];
    	    for (k=0; k<_num_store_refs; ++k)
    	    	_new_store_refs[_num_store_refs.k] = _store_refs[k];
    	    deletev _store_refs;
    	    _store_refs = _new_store_refs;
    	    _count += with._count;
    	    _num_store_refs += with._num_store_refs;
    	}
    }
    
    size_t count() const
    {
    	return _count;
    }
    
    const T& operator [](size_t pos) const
    {
    	size_t i, j;
    	
        for (i=0, j=0; i < _num_store_refs && j + _store_refs[i].len() <= pos;
                j += _store_refs[i++].len()) ;
        
        return _store_refs[i][pos - j];
    }

    void dump(FILE *file, size_t level)
    {
    	dump_indent(file, level);
    	fprintf(file, "WvEditor(%p): _count=%u, _num_store_refs=%u\n",
            	(unsigned)this, (unsigned)_count, (unsigned)_num_store_refs);
    
    	size_t i;
    	for (i=0; i<_num_store_refs; ++i)
    	{
            fprintf(file, "WvEditor(...cont...): _store_refs[%u]=\n", (unsigned)i);
            _store_refs[i].dump(file, level+1);
    	}
    }
};

#endif /// __WVEDITOR_H
