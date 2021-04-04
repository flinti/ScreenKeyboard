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
#include "Config.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <Shlobj.h>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "thirdparty/json.hpp"


using namespace std;
using json = nlohmann::json;

static void ConvertUTF8_UTF16(const std::string input, wstring& output)
{
    const char* input_cstr = input.c_str();
    wchar_t* widechar_str = 0;
    int size = MultiByteToWideChar(CP_UTF8, 0, input_cstr, -1, widechar_str, 0);
    if (size == 0)
        throw std::runtime_error("Error while converting UTF-8 to UTF-16. Check keyboard json file.");
    widechar_str = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, input_cstr, -1, widechar_str, size);
    output = widechar_str;
    delete[] widechar_str;
}

static void LoadConfig(Config& config, json& j, wstring default_name = L"default")
{
    vector<string> button_text_array;
    vector<string> button_text_shift_array;

    try
    {
        std::string display_name = j.value("display_name", "");
        if (display_name != "")
            ConvertUTF8_UTF16(display_name, config.display_name);
        else
            config.display_name = default_name;
        config.button_width = max(j.value("button_width", 1), 1);
        config.button_height = max(j.value("button_height", 1), 1);
        config.button_margin = max(j.value("button_margin", 1), 1);
        config.button_columns = max(j.value("button_columns", 1), 1);
        button_text_array = j.value("buttons", button_text_array);
        button_text_shift_array = j.value("buttons_shift", button_text_shift_array);
        config.button_amount = (int) button_text_array.size();
        if (config.button_amount % config.button_columns)
            config.button_amount += (config.button_columns - config.button_amount % config.button_columns);



         // convert all the UTF-8 strings in button_text_array
         // to UTF-16 and add them to config.button_texts
        for (int i = 0; i < (int) button_text_array.size(); ++i)
            ConvertUTF8_UTF16(button_text_array[i], config.button_texts[i]);
        for (int i = 0; i < (int) button_text_shift_array.size(); ++i)
            ConvertUTF8_UTF16(button_text_shift_array[i], config.button_shift_texts[i]);


        config.keyboard_width = (config.button_width + config.button_margin) * (config.button_columns + 1);
        config.keyboard_height = (config.button_height + config.button_margin) * (config.button_amount / config.button_columns + 1) + 10;
    }
    catch (exception& e)
    {
        stringstream ss;
        ss << "Config error: \n";
        ss << e.what();
        MessageBoxA(0, ss.str().c_str(), "JSON config error", MB_OK);
    }
}

void LoadConfig(Config& config, const char* json_text)
{
    json j;
    stringstream cfg;
    cfg << json_text;
    cfg >> j;
    LoadConfig(config, j);
}

void LoadConfig(Config& config, const std::filesystem::path& file_path)
{
    json j;
    ifstream f;
    f.open(file_path, ios::in);
    if (!f)
        return;
    f >> j;
    LoadConfig(config, j, file_path.c_str());
}
