/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002, Net Integration Technologies, Inc.
 * Copyright (C) 2002-2004, Pierre Phaneuf
 * Copyright (C) 2002-2004, Stéphane Lajoie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "loader.h"

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_MACH_O_DYLD_H
#include <mach-o/dyld.h>
#endif

#if defined(WITH_DLOPEN) && defined(ENABLE_LOADER)
const char* loaderOpen(const char* aFilename,
		       void** aHandle) {
  const char* rv = 0;

  /* clear out dl error */
  static_cast<void>(dlerror());

  *aHandle = dlopen(aFilename, RTLD_NOW);

  if(!*aHandle)
    rv = dlerror();

  return rv;
}

const char* loaderSymbol(void* aHandle,
			 const char* aSymbol,
			 void** aPointer) {
  /* clear out dl error */
  static_cast<void>(dlerror());

  *aPointer = dlsym(aHandle, aSymbol);

  return dlerror();
}

bool loaderClose(void*& aHandle) {
  bool rv;

  rv = dlclose(aHandle) == 0;
  aHandle = 0;

  return rv;
}

#elif defined(WITH_DYLD) && defined(ENABLE_LOADER)

const char* loaderOpen(const char* aFilename,
                       void** aHandle) {
  NSObjectFileImage ofi = 0;
  NSObjectFileImageReturnCode ofirc;

  ofirc = NSCreateObjectFileImageFromFile(aFilename, &ofi);
  switch(ofirc) {
  case NSObjectFileImageSuccess:
    *aHandle = NSLinkModule(ofi, aFilename,
                            NSLINKMODULE_OPTION_RETURN_ON_ERROR
                            | NSLINKMODULE_OPTION_PRIVATE
                            | NSLINKMODULE_OPTION_BINDNOW);
    NSDestroyObjectFileImage(ofi);
    break;
  case NSObjectFileImageInappropriateFile:
    *aHandle =
      const_cast<void*>(reinterpret_cast<const void*>(NSAddImage(aFilename, NSADDIMAGE_OPTION_RETURN_ON_ERROR)));
    break;
  default:
    return "could not open dynamic library";
    break;
  }

  return 0;
}

const char* loaderSymbol(void* aHandle,
                         const char* aSymbol,
                         void** aPointer) {
  int len = strlen(aSymbol);
  char* sym = static_cast<char*>(malloc(len + 2));
  NSSymbol* nssym = 0;

  snprintf(sym, len + 2, "_%s", aSymbol);

  /* Check for both possible magic numbers depending on x86/ppc byte order */
  if ((((struct mach_header *)aHandle)->magic == MH_MAGIC) ||
      (((struct mach_header *)aHandle)->magic == MH_CIGAM)) {
    if (NSIsSymbolNameDefinedInImage((struct mach_header *)aHandle, sym)) {
      nssym = (NSModule *)NSLookupSymbolInImage((struct mach_header *)aHandle,
                                    sym,
                                    NSLOOKUPSYMBOLINIMAGE_OPTION_BIND
                                    | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
    }
  } else {
    nssym = (NSModule *)NSLookupSymbolInModule(aHandle, sym);
  }

  free(sym);

  if(!nssym) {
    *aPointer = 0;
    return "symbol not found";
  }

  return 0;
}

bool loaderClose(void*& aHandle) {
  aHandle = 0;
  return false;
}

#elif defined(WIN32)

#include <windows.h>

const char* getErrorMessage() {
  static char error[1024];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, error, sizeof(error), 0);
  return error;
}

const char* loaderOpen(const char* aFilename,
		       void** aHandle) {
  const char* rv = 0;

  UINT oldErrorMode = SetErrorMode(0);
  SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS);
  *aHandle = LoadLibrary(aFilename);
  SetErrorMode(oldErrorMode);

  if(!*aHandle)
    rv = getErrorMessage();

  return rv;
}

const char* loaderSymbol(void* aHandle,
			 const char* aSymbol,
			 void** aPointer) {
  const char* rv = 0;

  *aPointer = GetProcAddress(static_cast<HMODULE>(aHandle), aSymbol);

  if(!aPointer)
    rv = getErrorMessage();

  return rv;
}

bool loaderClose(void*& aHandle) {
  bool rv;

  rv = FreeLibrary(static_cast<HMODULE>(aHandle)) != 0;
  aHandle = 0;

  return rv;
}

#else

const char* loaderOpen(const char* aFilename,
                       void** aHandle) {
  *aHandle = 0;
  return "dynamic loading not supported on this platform";
}

const char* loaderSymbol(void* aHandle,
                         const char* aSymbol,
                         void** aPointer) {
  *aPointer = 0;
  return "dynamic loading not supported on this platform";
}

bool loaderClose(void*& aHandle) {
  aHandle = 0;
  return false;
}

#endif
