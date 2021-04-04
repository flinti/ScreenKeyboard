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

#include <vector>

#include <Windows.h>

#include "targetver.h"
#include "Config.h"



struct Keyboard
{
    static DWORD CreateKeyboard(HINSTANCE instance, const std::vector<Config>& configs, unsigned default_config_idx);
    static void ReloadKeyboard(int index);
private:
    static void ButtonClicked(int idx, HWND button);
    static BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static bool CALLBACK SetButtonFont(HWND child, LPARAM font);
    static void RepositionComboBox();
    static bool CreateComboBox();
    static void InitButtons();
    static void AddButton(int x, int y, int idx);
    static void DestroyButtons();
    static void SendInputWChar(wchar_t wchar);
    static void ResizeKeyboard(RECT *suggested_rect = 0);

    struct DialogData
    {
        HWND dummy_button = NULL;
        std::map<int, HWND> button_handle;
    };

    static int dpiX, dpiY;
    static DialogData dialog_data;
    static std::vector<Config> configs;
    static Config *config;
    static unsigned config_index;
    static HINSTANCE instance;
    static HWND dialog;
    static HWND combo_box;
    static int combo_box_y;
    static bool created;
};