#include "unitransactiongen.h"
#include "uniconftree.h"
#include "wvmoniker.h"

// If 'obj' is an IUniConfGen, then we build the UniTransactionGen around
// it, else we create our underlying generator from 's'.
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    IUniConfGen *base = NULL;
    if (obj)
	base = mutate<IUniConfGen>(obj);
    if (!base)
	base = wvcreate<IUniConfGen>(s);
    if (base)
	return new UniTransactionGen(base);
    else
	return NULL;
}

static WvMoniker<IUniConfGen> moniker("transaction", creator);

/* This enum is a field of UniConfChangeTree. It indicates the type of
   change represented by a node in a UniConfChangeTree. */
enum changeMode
{
    /* This indicates that "newvalue" is valid and that
       its value should be written to the underlying generator at commit
       time. This tree *might* have children, which must be applied.
       "newvalue" will be a non-null pointer to a non-null WvString. */
    NEWVALUE,
    /* This indicates that "newtree" is valid (but possibly NULL) and that
       the underlying generator's corresponding subtree should be made
       identical at commit time. This tree will *not* have children (though
       newtree might). */
    NEWTREE,
    /* This indicates that "was_null_or_empty" is valid and that the key
       in the underlying generator should be created at commit time if it
       does not already exist at commit time. This tree *will* have
       children, which must be applied, and at least one of which will
       be non-BLANK. "was_null_or_empty" will be the return value of the
       WvString negation operation on the last known value of the
       corresponding key in the underlying generator; it is necessary
       in order to filter callbacks in certain cases. */
    NEWNODE,
    /* This indicates that none of the fields are valid and that
       nothing should be done for this tree. This tree *will* have children,
       which must be applied, but they will all have mode of NEWTREE with
       newtree == NULL. */
    BLANK
};

class UniConfChangeTree : public UniConfTree<UniConfChangeTree>
{
public:
    changeMode mode;

    WvString newvalue;
    UniConfValueTree *newtree;
    bool was_null_or_empty;

    // Constructs a tree and links it to a parent.
    UniConfChangeTree(UniConfChangeTree *parent, const UniConfKey &key)
	: UniConfTree<UniConfChangeTree>(parent, key), newtree(0) {}

    // Destroys a tree and everything it owns.
    ~UniConfChangeTree()
    {
	if (newtree)//mode == NEWTREE && newtree)
	    delete newtree;
    }
};

// Constructed by UniTransactionGen::iterator() to iterate over a section that
// is to be completely replaced by a particular UniConfValueTree.
class GenStyleValueTreeIter : public UniConfGen::Iter
{
public:
    GenStyleValueTreeIter(UniConfValueTree *node)
	: i(*node) {}

    void rewind() { i.rewind(); }
    bool next() { return i.next(); }
    UniConfKey key() const { return i->key(); }
    WvString value() const { return i->value(); }

private:
    UniConfValueTree::Iter i;
};

// Constructed by UniTransactionGen::iterator() to iterate over a section that
// is being modified but not replaced. We iterate first over all of the values
// that we're changing, except those we're deleting, and second over all
// existing values not iterated over in the first stage, except those we're
// deleting.
class GenStyleChangeTreeIter : public UniConfGen::Iter
{
public:
    GenStyleChangeTreeIter(UniConfChangeTree *_root,
			   const UniConfKey &_section,
			   IUniConfGen *_base)
	: root(_root), section(_section), base(_base),
	  doing_i1(true), i1(*root), i2(base->iterator(section)) {}

    ~GenStyleChangeTreeIter()
    {
	if (i2) delete i2;
    }

    void rewind()
    {
	i1.rewind();
	doing_i1 = true;
    }

    bool next()
    {
	if (doing_i1)
	{
	    for (;;)
	    {
		if (i1.next())
		{
		    if (i1->mode == NEWVALUE ||
			i1->mode == NEWNODE ||
			(i1->mode == NEWTREE && i1->newtree))
			return true;
		}
		else
		    break;
	    }
	    doing_i1 = false;
	    if (i2) i2->rewind();
	}
	if (i2)
	{
	    for (;;)
	    {
		if (i2->next())
		{
		    UniConfChangeTree *node = root->findchild(i2->key());
		    if (!node || node->mode == BLANK)
			return true;
		}
		else
		    break;
	    }
	}
	return false;
    }

    UniConfKey key() const
    {
	if (doing_i1)
	    return i1->key();
	else if (i2)
	    return i2->key();
	else
	    return UniConfKey();
    }

    WvString value() const
    {
	if (doing_i1)
	{
	    if (i1->mode == NEWVALUE)
		return i1->newvalue;
	    else if (i1->mode == NEWTREE)
		return i1->newtree->value();
	    else // i.e. i1->mode == NEWNODE
	    {
		WvString value(base->get(UniConfKey(section, i1->key())));
		return !value ? WvString::empty : value;
	    }
	}
	else
	{
	    return i2->value();
	}
    }

private:
    UniConfChangeTree *root;
    UniConfKey section;
    IUniConfGen *base;

    bool doing_i1;
    UniConfChangeTree::Iter i1;
    UniConfGen::Iter *i2;
};

UniTransactionGen::UniTransactionGen(IUniConfGen *_base)
    : root(NULL), base(_base)
{
    base->setcallback(
	UniConfGenCallback(this, &UniTransactionGen::gencallback), NULL);
}

UniTransactionGen::~UniTransactionGen()
{
    WVRELEASE(base);
    if (root)
	delete root;
}

WvString UniTransactionGen::get(const UniConfKey &key)
{
    UniConfChangeTree *node = root;
    for (int seg = 0;; node = node->findchild(key.segment(seg++)))
    {
	if (!node)
	    // If we couldn't find the next node, then we aren't
	    // changing the requested key, and so the value is whatever
	    // it currently is.
	    return base->get(key);
	else if (node->mode == NEWTREE)
	{
	    // Else if the next node has mode of NEWTREE, then we're changing
	    // the requested key to whatever its value is in the stored
	    // tree.
	    if (node->newtree)
	    {
		UniConfValueTree *subnode = node->newtree->find(
		    key.last(key.numsegments() - seg));
		if (subnode)
		    return subnode->value();
	    }
	    return WvString::null;
	}
	else if (seg == key.numsegments())
	{
	    // Else if this is the last node, then figure out what the node
	    // would do and return the appropriate value. (The node's mode
	    // will be either NEWVALUE, NEWNODE, or BLANK.)
	    if (node->mode == NEWVALUE)
		return node->newvalue;
	    WvString value(base->get(key.first(seg)));
	    return (node->mode == NEWNODE && !value) ? WvString::empty : value;
	}
    }
}

void UniTransactionGen::set(const UniConfKey &key, WvStringParm value)
{
    hold_delta();
    root = set_change(root, key, 0, value);
    unhold_delta();
}

void UniTransactionGen::commit()
{
    if (root)
    {
	// We ignore callbacks during commit() so that we don't waste
	// time in gencallback() for every set() we make during
	// apply_changes().
	base->setcallback(UniConfGenCallback(), NULL);
	apply_changes(root, UniConfKey());
	delete root;
	root = NULL;
	base->setcallback(
	    UniConfGenCallback(this, &UniTransactionGen::gencallback), NULL);
    }
}

bool UniTransactionGen::refresh()
{
    if (root)
    {
	hold_delta();
	cancel_changes(root, UniConfKey());
	delete root;
	root = NULL;
	unhold_delta();
    }
    return true;
}

UniConfGen::Iter *UniTransactionGen::iterator(const UniConfKey &key)
{
    UniConfChangeTree *node = root;
    for (int seg = 0;; node = node->findchild(key.segment(seg++)))
    {
	if (!node)
	    // If we couldn't find the next node, then we aren't changing the
	    // children of the requested key, so they're whatever they
	    // currently are.
	    return base->iterator(key);
	else if (node->mode == NEWTREE)
	{
	    // Else if the next node has mode of NEWTREE, then we're changing
	    // the children of the requested key to whatever they are in the
	    // stored tree.
	    if (node->newtree)
	    {
		UniConfValueTree *subnode = node->newtree->find(
		    key.last(key.numsegments() - seg));
		if (subnode)
		    return new GenStyleValueTreeIter(subnode);
	    }
	    return new UniConfGen::NullIter();
	}
	else if (seg == key.numsegments())
	    // Else if this is the last node, then iterate over its direct
	    // children.
	    return new GenStyleChangeTreeIter(node, key, base);
    }
}

void UniTransactionGen::apply_values(UniConfValueTree *newcontents,
				     const UniConfKey &section)
{
    WvString value(base->get(section));
    if (value != newcontents->value())
	// If the current value in the underlying generator isn't what
	// we want it to be, then change it.
	base->set(section, newcontents->value());
    
    if (!value.isnull())
    {
	UniConfGen::Iter *i = base->iterator(section);
	if (i)
	{
	    for (i->rewind(); i->next();)
	    {
		if (newcontents->findchild(i->key()) == NULL)
		    // Delete all children of the current value in the
		    // underlying generator that do not exist in our
		    // replacement tree.
		    base->set(UniConfKey(section, i->key()), WvString::null);
	    }
	    delete i;
	}
    }
    
    // Repeat for each child in the replacement tree.
    UniConfValueTree::Iter i(*newcontents);
    for (i.rewind(); i.next();)
	apply_values(i.ptr(), UniConfKey(section, i->key()));
}

void UniTransactionGen::apply_changes(UniConfChangeTree *node,
				      const UniConfKey &section)
{
    if (node->mode == NEWTREE)
    {
	// If the current change is a NEWTREE change, then replace the
	// tree in the underlying generator with the stored one.
	if (node->newtree == NULL)
	{
	    if (base->exists(section))
		base->set(section, WvString::null);
	}
	else
	    apply_values(node->newtree, section);
	// Since such changes have no children, return immediately.
	return;
    }
    else if (node->mode == NEWVALUE)
    {
	// Else if the current change is a NEWVALUE change, ...
	if (base->get(section) != node->newvalue)
	    // ... and the current value in the underlying generator isn't
	    // what we want it to be, then change it.
	    base->set(section, node->newvalue);
    }
    else if (node->mode == NEWNODE)
    {
	// Else if the current change is a NEWNODE change, ...
	if (!base->exists(section))
	    // ... and the current value in the underlying generator doesn't
	    // exist, then create it.
	    base->set(section, WvString::empty);
	// Note: This *is* necessary. We can't ignore this change and have
	// the underlying generator handle it, because it's possible that
	// this NEWNODE was the result of a set() which was later deleted.
    }
    
    // Repeat for each child in the change tree.
    UniConfChangeTree::Iter i(*node);
    for (i.rewind(); i.next();)
	apply_changes(i.ptr(), UniConfKey(section, i->key()));
}

struct my_userdata
{
    UniConfValueTree *node;
    const UniConfKey &key;
};

void UniTransactionGen::deletion_visitor(const UniConfValueTree *node,
					 void *userdata)
{
    my_userdata *data = (my_userdata *)userdata;
    delta(UniConfKey(data->key, node->fullkey(data->node)), WvString::null);
}

// Mirror image of apply_values() that issues all of the callbacks associated
// with discarding a replacement value tree.
void UniTransactionGen::cancel_values(UniConfValueTree *newcontents,
				      const UniConfKey &section)
{
    WvString value(base->get(section));
    if (!newcontents || newcontents->value() != value)
	delta(section, value);
    
    if (newcontents)
    {
	UniConfValueTree::Iter i(*newcontents);
	for (i.rewind(); i.next();)
	{
	    UniConfKey subkey(section, i->key());
	    if (!base->exists(subkey))
	    {
		my_userdata data = { i.ptr(), subkey };
		i->visit(
		    UniConfValueTree::Visitor(
			this, &UniTransactionGen::deletion_visitor),
		    (void *)&data, false, true);
	    }
	}
    }

    UniConfGen::Iter *i = base->iterator(section);
    if (i)
    {
	for (i->rewind(); i->next();)
	    cancel_values(newcontents ?
			  newcontents->findchild(i->key()) : NULL,
			  UniConfKey(section, i->key()));
	delete i;
    }
}

// Mirror image of apply_changes() that issues all of the callbacks associated
// with discarding a change tree.
void UniTransactionGen::cancel_changes(UniConfChangeTree *node,
				       const UniConfKey &section)
{
    if (node->mode == NEWTREE)
    {
	if (!base->exists(section))
	{
	    if (node->newtree != NULL)
	    {
		my_userdata data = { node->newtree, section };
		node->newtree->visit(
		    UniConfValueTree::Visitor(
			this, &UniTransactionGen::deletion_visitor),
		    (void *)&data, false, true);
	    }
	}
	else
	    cancel_values(node->newtree, section);
	return;
    }

    WvString value;
    if (node->mode != BLANK)
	value = base->get(section);

    if (node->mode == NEWVALUE &&
	!value.isnull() &&
	value != node->newvalue)
	delta(section, value);

    UniConfChangeTree::Iter i(*node);
    for (i.rewind(); i.next();)
	cancel_changes(i.ptr(), UniConfKey(section, i->key()));

    if (node->mode != BLANK && value.isnull())
	delta(section, WvString::null);
}

void UniTransactionGen::gencallback(const UniConfKey &key,
				    WvStringParm value,
				    void *userdata)
{
    UniConfChangeTree *node = root;
    for (int seg = 0;; node = node->findchild(key.segment(seg++)))
    {
	if (!node)
	    // If we couldn't find the next node, then we aren't changing
	    // the changed key or any of its children, and so a callback
	    // should be made.
	    break;
	else if (node->mode == NEWTREE)
	    // Else if the next node has mode of NEWTREE, then we're changing
	    // the changed key and all of its children to whatever their
	    // values are in the stored tree, and so the callback should be
	    // ignored.
	    return;
	else if (seg == key.numsegments())
	{
	    // Else if this is the last node, then figure out what we 
	    // should do.
	    if (node->mode == NEWVALUE)
		// If we're replacing this key's value, then we should
		// ignore the callback.
		return;
	    else if (node->mode == NEWNODE)
	    {
		// Else if we want to create this key, then use its
		// was_null_or_empty flag to figure out if we need
		// to issue a callback, and update it if necessary.
		if (node->was_null_or_empty && !value)
		    return;
		node->was_null_or_empty = !value;
		if (value.isnull())
		{
		    delta(key, WvString::empty);
		    return;
		}
		break;
	    }
	    else // i.e. node->mode == BLANK
		// Else if we're doing nothing to this key, then a
		// callback should be made.
		break;
	}
    }
    
    // Make a normal callback.
    delta(key, value);
}

// Create and return a UniConfValueTree containing the value 'value' for
// the key given by the segments of 'key' at and after position 'seg', with
// parent 'parent' and key given by the segment of 'key' at position seg-1
// (which is the "root" key if seg == 0). Issue callbacks as necessary using
// all the segments of 'key'.
UniConfValueTree *UniTransactionGen::create_value(UniConfValueTree *parent,
						  const UniConfKey &key,
						  int seg,
						  WvStringParm value)
{
    UniConfValueTree *tree = 0;
    for (; seg != key.numsegments();)
    {
	// Create any needed intermediate nodes, each with value equal to
	// the empty string.
	parent = new UniConfValueTree(parent,
				      key.segment(seg-1),
				      WvString::empty);
	delta(key.first(seg++), WvString::empty);
	if (!tree)
	    tree = parent;
    }
    // Create the last node with the specified value.
    parent = new UniConfValueTree(parent,
				  key.segment(seg-1),
				  value);
    delta(key, value);
    if (!tree)
	tree = parent;
    return tree;
}

void UniTransactionGen::deletion_simulator(const UniConfKey &key)
{
    UniConfGen::Iter *i = base->iterator(key);
    if (i)
    {
	for (i->rewind(); i->next();)
	    deletion_simulator(UniConfKey(key, i->key()));
	delete i;
    }
    delta(key, WvString::null);
}

// Like create_value(), but make a UniConfChangeTree containing a *change*
// to value 'value'.
UniConfChangeTree *UniTransactionGen::create_change(UniConfChangeTree *parent,
						    const UniConfKey &key,
						    int seg,
						    WvStringParm value)
{
    UniConfChangeTree *tree = 0;
    for (; seg != key.numsegments(); seg++)
    {
	parent = new UniConfChangeTree(parent, key.segment(seg-1));
	if (value.isnull())
	    // We don't do anything for intermediate nodes when deleting, ...
	    parent->mode = BLANK;
	else
	{
	    // ... but when set()'ing a non-null value, we want them to exist.
	    parent->mode = NEWNODE;
	    UniConfKey nodekey(key.first(seg));
	    WvString curr = base->get(nodekey);
	    parent->was_null_or_empty = !curr;
	    if (curr.isnull())
		delta(nodekey, WvString::empty);
	}
	if (!tree)
	    tree = parent;
    }
    parent = new UniConfChangeTree(parent, key.segment(seg-1));
    // Create the last node with the specified change.
    if (value.isnull())
    {
	parent->mode = NEWTREE;
	parent->newtree = 0;
	if (base->exists(key))
	    deletion_simulator(key);
    }
    else
    {
	parent->mode = NEWVALUE;
        fprintf(stderr, "Creating new string create_change\n");
	parent->newvalue = WvString(value);
	if (base->get(key) != value)
	    delta(key, value);
    }
    if (!tree)
	tree = parent;
    return tree;
}

// Modify an existing UniConfValueTree to incorporate the set() of a
// particular value for a particular key. Return a possibly altered
// pointer to the root of the tree. 'seg' and 'key' are used like they
// are in create_value(), and callbacks are made similarly.
UniConfValueTree *UniTransactionGen::set_value(UniConfValueTree *node,
					       const UniConfKey &key,
					       int seg,
					       WvStringParm value)
{
    if (value.isnull())
    {
	// Delete the key if it exists.
	if (node)
	{
	    UniConfValueTree *subnode = node->find(
		key.last(key.numsegments() - seg));
	    if (subnode)
	    {
		hold_delta();
		my_userdata data = { subnode, key };
		subnode->visit(
		    UniConfValueTree::Visitor(
			this, &UniTransactionGen::deletion_visitor),
		    (void *)&data, false, true);
		delete subnode;
		unhold_delta();
		return subnode == node ? NULL : node;
	    }
	    else
		return node;
	}
	else
	    return NULL;
    }
    else
    {
	// Switch to create_value() if we ever can't find the next node.
	if (!node)
	    return create_value(NULL, key, seg, value);
	
	UniConfValueTree *subnode = node;
	for (; seg != key.numsegments();)
	{
	    UniConfKey segment(key.segment(seg++));
	    UniConfValueTree *child = subnode->findchild(segment);
	    // Switch to create_value() if we ever can't find the next node.
	    if (!child)
	    {
		create_value(subnode, key, seg, value);
		return node;
	    }
	    else
		subnode = child;
	}
	// The node already existed and we've found it; set it.
	if (value != subnode->value())
	{
	    subnode->setvalue(value);
	    delta(key, value);
	}
	return node;
    }
}

void UniTransactionGen::deletion_simulator2(const UniConfKey &key)
{
    UniConfGen::Iter *i = this->iterator(key);
    if (i)
    {
	for (i->rewind(); i->next();)
	    deletion_simulator2(UniConfKey(key, i->key()));
	delete i;
    }
    delta(key, WvString::null);
}

// Like set_value(), but, again, for UniConfChangeTrees instead.
UniConfChangeTree *UniTransactionGen::set_change(UniConfChangeTree *node,
						 const UniConfKey &key,
						 int seg,
						 WvStringParm value)
{
    fprintf(stderr, "set_change(%p, %s, %d, %s) called\n", node, key.cstr(), seg, value.cstr());
    // Switch to create_change() if we ever can't find the next node,
    // and switch to set_value() if we ever find a NEWTREE.
    if (!node)
	return create_change(NULL, key, seg, value);
    else if (node->mode == NEWTREE)
    {
	node->newtree = set_value(node->newtree, key, seg, value);
	return node;
    }
    
    UniConfChangeTree *subnode = node;
    for (; seg != key.numsegments();)
    {
	if (subnode->mode == BLANK && !value.isnull())
	{
	    // If we're setting a non-null value and we weren't previously
	    // doing anything to this node, then now we want to create it.
	    subnode->mode = NEWNODE;
	    UniConfKey nodekey(key.first(seg));
	    WvString curr = base->get(nodekey);
	    subnode->was_null_or_empty = !curr;
	    if (curr.isnull())
		delta(nodekey, WvString::empty);
	}
	
	UniConfKey segment(key.segment(seg++));
	UniConfChangeTree *next = subnode->findchild(segment);
	// Switch to create_change() if we ever can't find the next node,
	// and switch to set_value() if we ever find a NEWTREE.
	if (!next)
	{
	    create_change(subnode, key, seg, value);
	    return node;
	}
	else if (next->mode == NEWTREE)
	{
	    next->newtree = set_value(next->newtree,
				      key, seg, value);
	    return node;
	}
	else
	    subnode = next;
    }
    // The node already existed, didn't have mode of NEWTREE, and we've
    // found it; change it.
    if (value.isnull())
    {
	if (subnode->mode != BLANK || base->exists(key))
	    deletion_simulator2(key);
	subnode->zap();
	subnode->mode = NEWTREE;
	subnode->newtree = 0;
    }
    else if (subnode->mode == NEWVALUE)
    {
	if (subnode->newvalue != value)
	{
	    subnode->newvalue = value;
	    delta(key, value);
	}
    }
    else if (subnode->mode == BLANK)
    {
	if (base->get(key) != value)
	    delta(key, value);	    
	subnode->mode = NEWVALUE;
	subnode->newvalue = WvString(value);
    }
    else // i.e. subnode->mode == NEWNODE
    {
	WvString currval(base->get(key));
	if ((!currval != !value) && (currval != value))
	    delta(key, value);
	subnode->mode = NEWVALUE;
	subnode->newvalue = WvString(value);
    }
    return node;
}

// We'll say we're okay whenever the underlying generator is.
bool UniTransactionGen::isok()
{
    return base->isok();
}

void UniTransactionGen::flush_buffers()
{
}
