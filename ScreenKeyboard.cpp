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


#pragma comment(lib, "ComCtl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <windowsx.h>
#include <CommCtrl.h>
#include <Shlobj.h>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <stdint.h>

#include "targetver.h"
#include "Resource.h"
#include "Config.h"
#include "Keyboard.h"

using namespace std;



HINSTANCE instance = NULL;
vector<Config> configs;


bool ReadUserConfigFiles()
{
    bool loaded_any_custom_layouts = false;
    wchar_t* documents_path_c = NULL;


    HRESULT res = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documents_path_c);
    wstring documents_path_str(documents_path_c);
    CoTaskMemFree((void*)documents_path_c);

    try
    {
        filesystem::path config_path(documents_path_str);
        config_path /= "ScreenKeyboard";

        error_code err;
        if (!filesystem::is_directory(config_path, err))
            return false;

        filesystem::directory_iterator dir_iter(config_path);
        for (auto& path_iter : dir_iter)
        {
            filesystem::path path = path_iter.path();
            try
            {
                if (path_iter.is_regular_file() or path_iter.is_symlink())
                {
                    if (path.extension() == ".json")
                    {
                        Config cfg;
                        LoadConfig(cfg, path);
                        configs.push_back(cfg);
                        loaded_any_custom_layouts = true;
                    }
                }
            }
            catch (exception& e)
            {
                stringstream text;
                text << "The layout file '" << path << "' could not be loaded: " << e.what() << "\n\nPress OK to continue.";
                MessageBoxA(NULL, text.str().c_str(), "One of the user layout files could not be loaded", MB_OK | MB_ICONWARNING);
            }
        }
    }
    catch (exception& e)
    {
        stringstream text;
        text << "Could not load user layouts: " << e.what() << "\n";
        MessageBoxA(NULL, text.str().c_str(), "Error while loading user layout files", MB_OK | MB_ICONERROR);
    }

    return loaded_any_custom_layouts;
}




bool LoadFileInResource(int name, int type, DWORD& size, const char*& data)
{
    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
    if (rc == 0)
        return false;
    HGLOBAL rcData = LoadResource(handle, rc);
    if (rcData == 0)
        return false;
    size = SizeofResource(handle, rc);
    data = static_cast<const char*>(LockResource(rcData));
    return true;
}


void TryLoadEmbeddedConfigString(int name, int type)
{
    // load embedded default layout
    DWORD size = 0;
    const char* data = 0;
    if (!LoadFileInResource(name, type, size, data))
    {
        MessageBoxA(NULL, "Error loading embedded default config!", "FATAL ERROR", MB_OK | MB_ICONERROR);
        return;
    }
    char* resstr = new char[size + 1L];
    memcpy(resstr, data, size);
    resstr[size] = 0;
    string str = resstr;
    delete[] resstr;

    try
    {
        Config cfg;
        LoadConfig(cfg, str.c_str());
        configs.push_back(cfg);
    }
    catch (exception& e)
    {
        stringstream text;
        text << "Exception while loading embedded config: " << e.what() << "\n";
        MessageBoxA(NULL, text.str().c_str(), "FATAL ERROR", MB_OK | MB_ICONERROR);
    }
}


int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    instance=hInst;  

    configs.reserve(2);
    TryLoadEmbeddedConfigString(IDR_JSON_DEFAULT_ENGLISH, RESTYPE_JSON_FILE);
    TryLoadEmbeddedConfigString(IDR_JSON_OLD_ENGLISH, RESTYPE_JSON_FILE);

    ReadUserConfigFiles();

    InitCommonControls();

    if (configs.size() < 1)
        configs.emplace_back();
    int ret = Keyboard::CreateKeyboard(instance, configs, 0);

    return ret;
}
