/*
    Copyright (C) 2021 Christian Strauch

    This file is part of ScreenKeyboard.

    ScreenKeyboard is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ScreenKeyboard is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ScreenKeyboard.  If not, see <https://www.gnu.org/licenses/>

*/
#pragma once


#include <winsdkver.h>

#undef _WIN32_WINNT
#undef WINVER
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

#include <SDKDDKVer.h>
