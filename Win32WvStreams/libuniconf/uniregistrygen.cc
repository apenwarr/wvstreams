/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 * 
 * A generator that exposes the windows registry.
 */
#include "uniregistrygen.h"
#include "wvmoniker.h"

UniRegistryGen::UniRegistryGen(WvString _moniker) :
    m_log(_moniker), m_hRoot(0)
{
    UniConfKey key = _moniker;
    WvString hive = key.first().printable();
    if (strcmp("HKEY_CLASSES_ROOT", hive) == 0)
    {
	m_hRoot = HKEY_CLASSES_ROOT;
    } 
    else if (strcmp("HKEY_CURRENT_USER", hive) == 0)
    {
	m_hRoot = HKEY_CURRENT_USER;
    }
    else if (strcmp("HKEY_LOCAL_MACHINE", hive) == 0)
    {
	m_hRoot = HKEY_LOCAL_MACHINE;
    }
    else if (strcmp("HKEY_USERS", hive) == 0)
    {
	m_hRoot = HKEY_USERS;
    }

    m_hRoot = follow_path(key.range(1, key.numsegments()), true, NULL);

#if 0
    // FIXME: Notifications don't work for external registry changes.
    //
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    RegNotifyChangeKeyValue(
	m_hRoot,
	TRUE,
	REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
	REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_SECURITY,
	hEvent,
	TRUE
    );
#endif
}

UniRegistryGen::~UniRegistryGen()
{
    if (m_hRoot)
    {
	LONG result = RegCloseKey(m_hRoot);
	m_hRoot = 0;
    }
}

bool UniRegistryGen::isok()
{
    return m_hRoot != 0;
}

// returns a handle to the key specified by key, or, if key specifies a value,
// a handle to the key containing that value (and setting isValue = true)
HKEY UniRegistryGen::follow_path(const UniConfKey &key, bool create, bool *isValue)
{
    const REGSAM samDesired = KEY_READ | KEY_WRITE;
    LONG result;
    HKEY hLastKey = m_hRoot; // DuplicateHandle() does not work with regkeys
    int n = key.numsegments();

    if (isValue) *isValue = false;

    for (int i=0;i<n;i++)
    {
	WvString subkey = key.segment(i).printable();
	HKEY hNextKey;
	
	if (create)
	{
	    result = RegCreateKeyEx(hLastKey, subkey, 0, NULL, 0, samDesired, 
		NULL, &hNextKey, NULL);
	}
	else
	{
	    result = RegOpenKeyEx(hLastKey, subkey, 0, samDesired, &hNextKey);
	}

	if ((result == ERROR_FILE_NOT_FOUND) && (i == n-1))
	{
	    // maybe the last segment is a value name
	    if (RegQueryValueEx(hLastKey, subkey, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
	    {
		// ... it is a value
		if (isValue) *isValue = true;
		break;
	    }
	}
	if (result != ERROR_SUCCESS)
	{
	    return 0;
	}
	
	
	if (i > 0)
	{
	    RegCloseKey(hLastKey);
	}
	hLastKey = hNextKey;
    }

    return hLastKey;
}

WvString UniRegistryGen::get(const UniConfKey &key)
{
    WvString retval = WvString::null;
    bool isvalue;
    DWORD type;
    TCHAR data[1024];
    DWORD size = sizeof(data);
    WvString value = key.last().printable();
    
    HKEY hKey = follow_path(key, false, &isvalue);
    if (!isvalue)
    {
	// allow fetching a key's default value
	value = WvString::null;
    }
    
    LONG result = RegQueryValueEx(
	hKey, 
	value.cstr(), 
	0, 
	&type, 
	(BYTE *) data, 
	&size
    );

    if (result == ERROR_SUCCESS)
    {
	switch (type)
	{
	case REG_DWORD:
	    retval.setsize(11);
	    itoa(*((int *) data), retval.edit(), 10);
	    break;
	case REG_SZ:
	    retval = data;
	    break;
	default:
	    break;
	};
    }

    if (hKey != m_hRoot) RegCloseKey(hKey);
    return retval;
}

void UniRegistryGen::set(const UniConfKey &key, WvStringParm value)
{
    LONG result;
    HKEY hKey = follow_path(key.first( key.numsegments()-1 ), true, NULL);
    if (hKey)
    {
	result = RegSetValueEx(
	    hKey,
	    key.last().printable(),
	    0,
	    REG_SZ,
	    (BYTE *) value.cstr(),
	    strlen(value)+1
	);
	if (result == ERROR_SUCCESS)
	{
	    delta(key, value);
	}
    }
    if (hKey != m_hRoot) RegCloseKey(hKey);
}

bool UniRegistryGen::exists(const UniConfKey &key)
{
    return get(key) == WvString::null;
}

bool UniRegistryGen::haschildren(const UniConfKey &key)
{
    bool isValue;
    HKEY hKey = follow_path(key, false, &isValue);
    bool retval = false;
    if (hKey && !isValue)
    {
	FILETIME dontcare;
	TCHAR data[1024];
	DWORD size = sizeof(data);
	LONG result = RegEnumKeyEx(hKey, 0, data, &size, 0, 
	    NULL, NULL, &dontcare);
	retval = ((result == ERROR_SUCCESS) || (result == ERROR_MORE_DATA));
    }
    
    if (hKey != m_hRoot) RegCloseKey(hKey);

    return retval;
}

UniConfGen::Iter *UniRegistryGen::iterator(const UniConfKey &key)
{
    return new NullIter();
}

static IUniConfGen *creator(WvStringParm s, IObject *, void *)
{
    return new UniRegistryGen(s);
}

#pragma warning(disable : 4073)
#pragma init_seg(lib)
WvMoniker<IUniConfGen> UniRegistryGenMoniker("registry", creator);
