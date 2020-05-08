// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <ddraw.h>
#include <d3d.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#ifdef MINGW_HAS_SECURE_API
// Bug workaround to fix compilation
// of comdef.h
#include <stdio.h>
#endif
#include <comdef.h>

#include <string>
#include <sstream>

#include "ComPtr.h"
#include "logger.h"
#include "utils.h"
#include "config.h"