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
#include "Keyboard.h"

#include <string>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <CommCtrl.h>
#include <shellscalingapi.h>

#include "resource.h"

using namespace std;

int Keyboard::dpiX = 0, Keyboard::dpiY = 0;
Keyboard::DialogData Keyboard::dialog_data;
vector<Config> Keyboard::configs;
Config *Keyboard::config = 0;
unsigned Keyboard::config_index = 0;
HINSTANCE Keyboard::instance = 0;
HWND Keyboard::dialog = 0;
HWND Keyboard::combo_box = 0;
int Keyboard::combo_box_y = 0;
bool Keyboard::created = false;


DWORD Keyboard::CreateKeyboard(HINSTANCE inst, const vector<Config> &new_configs, unsigned config_idx)
{
    if (created)
        return 1;
    if (new_configs.size() < 1)
        return 1;

    configs = new_configs;
    config = &configs[config_idx];
    config_index = config_idx;
    instance = inst;


    // For interface scaling
    // load Shcore.dll dynamically so that this program also runs on Win7 where Shcore.dll is not available
    typedef HRESULT(*DpiAwarenessSetter)(PROCESS_DPI_AWARENESS);
    SetDllDirectory(L""); // do not allow loading dll from working dir
    HMODULE lib = LoadLibraryW(L"Shcore.dll");
    if (lib)
    {
        DpiAwarenessSetter SetProcessDpiAwarenessDyn = (DpiAwarenessSetter) GetProcAddress(lib, "SetProcessDpiAwareness");
        if (SetProcessDpiAwarenessDyn)
            SetProcessDpiAwarenessDyn(PROCESS_PER_MONITOR_DPI_AWARE);
    }

    SetProcessDPIAware();
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }

    DWORD ret = (DWORD) DialogBoxW(instance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
    created = true;
    return ret;
}

void Keyboard::ReloadKeyboard(int index)
{
    if (index >= configs.size() || index < 0)
        return;

    DestroyButtons();

    config_index = index;
    config = &configs[index];

    InitButtons();
    RepositionComboBox();
    ResizeKeyboard();
    // prevent highlight of one of the buttons by focussing a dummy button
    SetFocus(dialog_data.dummy_button);
}

void Keyboard::RepositionComboBox()
{
    int x = config->button_margin;
    int y = combo_box_y;
    int wdt = config->button_width * config->button_columns + config->button_margin * (config->button_columns - 1);
    int hgt = config->button_height;
    SetWindowPos(combo_box, HWND_TOP, x * dpiX / 96.f, y * dpiY / 96.f, wdt * dpiX / 96.f, hgt * dpiY / 96.f, SWP_NOACTIVATE | SWP_NOZORDER);
    InvalidateRect(combo_box, 0, TRUE);
    UpdateWindow(combo_box);
}

bool Keyboard::CreateComboBox()
{
    combo_box = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
        1, 1, 1, 1, dialog, NULL, instance, NULL);
    SetWindowLongW(combo_box, GWL_EXSTYLE, WS_EX_NOACTIVATE);

    LRESULT font = SendMessage(dialog, WM_GETFONT, 0, 0);
    SendMessage(combo_box, WM_SETFONT, font, TRUE);
    for (auto& config : configs)
    {
        SendMessage(combo_box, (UINT) CB_ADDSTRING, 0, (LPARAM)config.display_name.c_str());
    }
    SendMessage(combo_box, CB_SETCURSEL, (WPARAM)config_index, (LPARAM)0);

    RepositionComboBox();

    return true;
}

void Keyboard::SendInputWChar(wchar_t wchar)
{
    INPUT input[2];
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = wchar;
    input[0].ki.time = 0;
    input[0].ki.dwFlags = KEYEVENTF_UNICODE;
    input[0].ki.dwExtraInfo = 0;// GetMessageExtraInfo();

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 0;
    input[1].ki.wScan = wchar;
    input[1].ki.time = 0;
    input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    input[1].ki.dwExtraInfo = NULL;

    SendInput(2, input, sizeof(INPUT));
}




void Keyboard::ButtonClicked(int idx, HWND button)
{
    bool shift_pressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
    bool no_shift = true;

    std::map<int, std::wstring>::const_iterator text_iter;

    if (shift_pressed)
    {
        text_iter = config->button_shift_texts.find(idx);
        if (text_iter != config->button_shift_texts.end())
        {
            if(text_iter->second != L"")
                no_shift = false;
        }
    }
    
    if (no_shift)
    {
        text_iter = config->button_texts.find(idx);
        if (text_iter == config->button_texts.end())
            return;
    }
    

    wstring send_str = text_iter->second;

    for (size_t i = 0; i < send_str.length(); ++i)
    {
        wchar_t wchar = send_str[i];
        SendInputWChar(wchar);
    }
}

void Keyboard::AddButton(int x, int y, int idx)
{
    wstring text;
    auto button_text_iter = config->button_texts.find(idx);
    if (button_text_iter != config->button_texts.end())
        text = button_text_iter->second;

    RECT keyboardRect;
    GetWindowRect(dialog, &keyboardRect);
    int keyboard_width = keyboardRect.right - keyboardRect.left;

    // TODO: center buttons in window
    HWND hwndButton = CreateWindow(
        L"BUTTON",
        text.c_str(),
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_TEXT,
        x * dpiX / 96.f,
        y * dpiY / 96.f, 
        config->button_width * dpiX / 96.f,
        config->button_height * dpiY / 96.f,
        dialog,
        (HMENU)idx, 
        instance,
        NULL);

    dialog_data.button_handle[idx] = hwndButton;

    if (text.empty())
        EnableWindow(hwndButton, FALSE);
}

bool CALLBACK Keyboard::SetButtonFont(HWND child, LPARAM font)
{
    SendMessage(child, WM_SETFONT, font, true);
    return true;
}

void Keyboard::InitButtons()
{
    int x = config->button_margin;
    int y = config->button_margin;

    dialog_data.dummy_button = CreateWindow(L"BUTTON", L"", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, dialog, NULL, instance, NULL);

    for (int i = 1; i <= config->button_amount; ++i)
    {

        AddButton(x, y, i - 1);
        x += (config->button_width + config->button_margin);
        if (i % config->button_columns == 0)
        {
            x = config->button_margin;
            y += (config->button_height + config->button_margin);
        }
    }
    combo_box_y = y + config->button_margin;
    // The buttons get the same font as the main dialog
    LRESULT font = SendMessage(dialog, WM_GETFONT, 0, 0);
    EnumChildWindows(dialog, (WNDENUMPROC)SetButtonFont, (LPARAM)font);
    // prevent highlight of one of the buttons by focussing a dummy button
    SetFocus(dialog_data.dummy_button);
}

void Keyboard::DestroyButtons()
{
    DestroyWindow(dialog_data.dummy_button);
    dialog_data.dummy_button = 0;
    for (auto& p : dialog_data.button_handle)
    {
        DestroyWindow(p.second);
    }
    dialog_data.button_handle.clear();
}

void Keyboard::ResizeKeyboard(RECT* suggested_rect)
{
    bool has_combo = combo_box != 0;
    int hgt = has_combo ? config->keyboard_height + config->button_margin * 2 + config->button_height : config->keyboard_height;

    if (!suggested_rect)
    {
        SetWindowPos(dialog, HWND_TOPMOST, 0, 0, config->keyboard_width * dpiX / 96.f, hgt * dpiY / 96.f, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
    }
    else
    {
        SetWindowPos(dialog, HWND_TOPMOST, suggested_rect->left, suggested_rect->top, config->keyboard_width * dpiX / 96.f, hgt * dpiY / 96.f, SWP_NOACTIVATE | SWP_NOZORDER);
    }
}

BOOL CALLBACK Keyboard::DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        dialog = hwndDlg;

        InitButtons();
        bool has_combo = CreateComboBox();
        ResizeKeyboard();
        
    }
    return TRUE;

    case WM_CLOSE:
    {
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    // TODO: scale font
    case WM_DPICHANGED:
    {
        WORD new_dpi_x = LOWORD(wParam);
        WORD new_dpi_y = HIWORD(wParam);
        RECT* recommended_pos = (RECT*)lParam;
        dpiX = new_dpi_x;
        dpiY = new_dpi_y;
        ReloadKeyboard(config_index);
        ResizeKeyboard(recommended_pos);
        return TRUE;
    }


    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
        {
            int index = (int)LOWORD(wParam);
            HWND button = (HWND)lParam;
            ButtonClicked(index, button);
            break;
        }
        case CBN_SELCHANGE:
        {
            // TODO: Fix ScreenKeyboard getting focus after layout change
            int index = (int) SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
            ReloadKeyboard(index);
            break;
        }
        }
    }
    return TRUE;
    }
    return FALSE;
}

