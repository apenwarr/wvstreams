/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 * 
 * A generator that exposes Windows protected storage.
 */
#include "unipstoregen.h"
#include "wvmoniker.h"
#include <string>

static const int MAX = 1024;

using namespace PSTORECLib;

typedef HRESULT (WINAPI *PStoreCreateInstancePtr)(IPStore **, DWORD, DWORD, DWORD);

HRESULT UniPStoreGen::create_types(WvString type_name, WvString subtype_name)
{
    HRESULT hRes;
    
    _PST_TYPEINFO myTypeInfo;
    myTypeInfo.cbSize = strlen(type_name.cstr()) + 1;
    myTypeInfo.szDisplayName = new wchar_t[myTypeInfo.cbSize];
    mbstowcs(myTypeInfo.szDisplayName, type_name.cstr(), myTypeInfo.cbSize);
    
    _PST_TYPEINFO mySubTypeInfo;
    mySubTypeInfo.cbSize = strlen(subtype_name.cstr()) + 1;
    mySubTypeInfo.szDisplayName = new wchar_t[mySubTypeInfo.cbSize];
    mbstowcs(mySubTypeInfo.szDisplayName, subtype_name.cstr(), mySubTypeInfo.cbSize);

    _PST_ACCESSRULESET myRuleSet;
    myRuleSet.cbSize = sizeof(myRuleSet);
    myRuleSet.cRules = 0;
    myRuleSet.rgRules = 0;

    hRes = m_spPStore->CreateType( m_key, &m_type, &myTypeInfo, 0);
    
    if ((hRes != PST_E_OK) && (hRes != PST_E_TYPE_EXISTS))
    {
	m_log("CreateSubtype() returned: %s\n", hRes);
	goto done;
    }

    hRes = m_spPStore->CreateSubtype( m_key, &m_type, &m_subtype, &mySubTypeInfo, &myRuleSet, 0);
    if ((hRes != PST_E_OK) && (hRes != PST_E_TYPE_EXISTS))
    {
	m_log("CreateSubtype() returned: %s\n", hRes);
	goto done;
    }

done:
    delete[] myTypeInfo.szDisplayName;
    delete[] mySubTypeInfo.szDisplayName; 
    return hRes;
}

// moniker is
// PST_KEY_CURRENT_USER:TYPENAME:TYPEGUID:SUBTYPE:SUBTYPEGUID
UniPStoreGen::UniPStoreGen(WvString _moniker) :
    m_log(_moniker), m_key(-1)
{
    // load the library and get an entry point function pointer
    m_hPstoreDLL = LoadLibrary("pstorec.dll");
    assert(m_hPstoreDLL);

    PStoreCreateInstancePtr pPStoreCreateInstance = 
	(PStoreCreateInstancePtr) GetProcAddress(m_hPstoreDLL, "PStoreCreateInstance");
    assert(pPStoreCreateInstance);

    HRESULT hr = pPStoreCreateInstance(&m_spPStore, 0, 0, 0);
    assert(SUCCEEDED(hr));

    // parse the moniker
    char *moniker = _moniker.edit();
    const char *seps = ":";
    WvString _key = strtok(moniker, seps);
    WvString type_name = strtok(NULL, seps);
    WvString _type_guid = strtok(NULL, seps);
    WvString subtype_name = strtok(NULL, seps);
    WvString _subtype_guid = strtok(NULL, seps);
    
    if (!!_key && strcmp(_key, "PST_KEY_CURRENT_USER") == 0)
    {
	m_key = PST_KEY_CURRENT_USER;
    }
    else if (!!_key && strcmp(_key, "PST_KEY_LOCAL_MACHINE") == 0)
    {
	m_key = PST_KEY_LOCAL_MACHINE;
    }

    if ((m_key >= 0) && !!type_name && !!_type_guid && !!subtype_name && !!_subtype_guid)
    {
	HRESULT hr;
	hr = UuidFromString((unsigned char*)_type_guid.edit(), &m_type);
	hr = UuidFromString((unsigned char*)_subtype_guid.edit(), &m_subtype);
	int result = create_types(type_name, subtype_name);
        assert(SUCCEEDED( result ) || (result == PST_E_TYPE_EXISTS));
    }
}

UniPStoreGen::~UniPStoreGen()
{
    m_spPStore = 0;
    if (m_hPstoreDLL)
    {
	FreeLibrary(m_hPstoreDLL);
	m_hPstoreDLL = 0;
    }
}

bool UniPStoreGen::isok()
{
    return m_key >= 0;
}


WvString UniPStoreGen::get(const UniConfKey &key)
{
    HRESULT hRes;
    WvString value = WvString::null;

    unsigned char *data;
    unsigned long cbdata;

    WvString _name = key.last().printable();
    WCHAR name[MAX];
    mbstowcs(name, _name.cstr(), MAX);

    hRes = m_spPStore->ReadItem(
	m_key,
	&m_type,
	&m_subtype,
	name,
	&cbdata,
	&data,
	NULL,
	0
    );

    if (hRes == PST_E_OK)
    {
	value.setsize(MAX);
	wcstombs(value.edit(), (wchar_t*)data, MAX);
	CoTaskMemFree(data);
    }

    return value;
}

void UniPStoreGen::set(const UniConfKey &key, WvStringParm value)
{
    HRESULT hRes;
    unsigned char *data = (unsigned char *) value.cstr();
    
    WvString _name = key.last().printable();
    WCHAR name[MAX];
    mbstowcs(name, _name.cstr(), MAX);
    
    DWORD cbdata = DWORD((strlen(value.cstr()) + 1) * sizeof(char));
   
    hRes = m_spPStore->WriteItem(
	m_key, 
	&m_type, 
	&m_subtype, 
	name, 
	cbdata, 
	data, 
	NULL, 
	PST_CF_NONE, 
	0
    );

    if (hRes == PST_E_OK)
    {
	delta(key, value);
    }
}

bool UniPStoreGen::exists(const UniConfKey &key)
{
    return false;
}

bool UniPStoreGen::haschildren(const UniConfKey &key)
{
    return false;
}

UniConfGen::Iter *UniPStoreGen::iterator(const UniConfKey &key)
{
    return new NullIter();
}

static IUniConfGen *creator(WvStringParm s, IObject *, void *)
{
    return new UniPStoreGen(s);
}

#pragma warning(disable : 4073)
#pragma init_seg(lib)
WvMoniker<IUniConfGen> UniPStoreGenMoniker("pstore", creator);
