/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002-2003, Pierre Phaneuf
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
 *
 * As a special exception, you may use this file as part of a free
 * software library without restriction.  Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU Lesser General Public
 * License.  This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Lesser
 * General Public License.
 */

#include <stdlib.h>
#include <string.h>
#include <xplc/uuid.h>

const UUID UuidFromString(const char* str) {
  UUID rv;
  char tmp[3];
  char* end;
  bool format1 = false;
  bool ok = false;

  do {
    if(*str == '{') {
      format1 = true;
      ++str;
    }

    rv.Data1 = strtoul(str, &end, 16);
    if(end != str + 8)
      break;
    str = end;

    if(*str != '-')
      break;
    ++str;

    rv.Data2 = static_cast<unsigned short>(strtoul(str, &end, 16));
    if(end != str + 4)
      break;
    str = end;

    if(*str != '-')
      break;
    ++str;

    rv.Data3 = static_cast<unsigned short>(strtoul(str, &end, 16));
    if(end != str + 4)
      break;
    str = end;

    if(*str != '-')
      break;
    ++str;

    tmp[2] = 0;

    strncpy(tmp, str, 2);
    rv.Data4[0] = static_cast<unsigned char>(strtoul(tmp, &end, 16));
    if(end != tmp + 2)
      break;
    str += 2;

    strncpy(tmp, str, 2);
    rv.Data4[1] = static_cast<unsigned char>(strtoul(tmp, &end, 16));
    if(end != tmp + 2)
      break;
    str += 2;

    if(*str != '-')
      break;
    ++str;

    for(int i = 2; i < 8; ++i) {
      strncpy(tmp, str, 2);
      rv.Data4[i] = static_cast<unsigned char>(strtoul(tmp, &end, 16));
      if(end != tmp + 2)
	break;
      str += 2;
    }

    if(format1) {
      if(*str != '}')
	break;
      ++str;
    }

    if(*str != 0)
      break;

    ok = true;
  } while(0);

  if(!ok)
    rv = UUID_null;

  return rv;
}

