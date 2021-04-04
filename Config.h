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

#include <map>
#include <string>
#include <filesystem>

#include "targetver.h"


struct Config
{
    // display name in combo box
    std::wstring display_name;

    // keyboard & button size
    int keyboard_width = 200;
    int keyboard_height = 80;
    int button_width = 40;
    int button_height = 30;
    int button_margin = 5;
    int button_columns = 6;
    int button_amount = 12;

    // button text
    std::map<int, std::wstring> button_texts;
    // button shift text/character
    std::map<int, std::wstring> button_shift_texts;
};

void LoadConfig(Config& config, const char* json);
void LoadConfig(Config& config, const std::filesystem::path &file_path);