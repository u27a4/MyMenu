#include "MyMenu.h"
#include <string>
#include <stack>
#include <unordered_map>
#include "AEUT.hpp"

#include "Lib/json11/json11.hpp"

static HWND					s_hWnd = 0L;
static AEGP_Command			s_command = NULL;
static struct {
    bool                    handled = true;
    POINT                   pt;
    std::list<AEGP_LayerH>  selection;
}                           s_event;

HMENU
BuildPopupMenu(std::unordered_map<UINT_PTR, std::string>& commands)
{
    std::string             returnValue = "";
    std::string             error = "";
    UINT_PTR                id = 1;

    auto build = [&commands, &id](auto Self, std::vector<json11::Json> items) -> HMENU
    {
        HMENU hMenu = CreatePopupMenu();
        for (auto &item : items)
        {
            auto name = item["name"].string_value();
            auto items = item["items"].array_items();
            auto uFlags = item["enabled"].bool_value() ? MF_ENABLED : MF_GRAYED;
            auto show = item["show"].bool_value();

            if (show == false)
            {
                continue;
            }

            if (items.empty())
            {
                AppendMenu(hMenu, uFlags | (name.length() > 0 ? MF_STRING : MF_SEPARATOR), id, name.c_str());
                commands[id] = "(" + item["func"].string_value() + ")();";
                id++;
            }
            else
            {
                AppendMenu(hMenu, uFlags | MF_POPUP, (UINT_PTR)Self(Self, items), name.c_str());
            }
        }
        return hMenu;
    };

    commands.clear();

    AEUT_ExecuteScript(BuildMenuScript, returnValue, error);
    return build(build, json11::Json::parse(returnValue, error).array_items());
}

LRESULT CALLBACK
MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0) return CallNextHookEx(NULL, code, wParam, lParam);

	auto pmh = (MOUSEHOOKSTRUCT*)lParam;
	auto swapped = GetSystemMetrics(SM_SWAPBUTTON);
	auto primary = GetAsyncKeyState(swapped ? VK_RBUTTON : VK_LBUTTON);
	auto secondary = GetAsyncKeyState(swapped ? VK_LBUTTON : VK_RBUTTON);

	if (primary & 0x8000 && secondary & 0x0001)
	{
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

        s_event.handled = false;
        s_event.pt = pmh->pt;
        AEUT_GetSelectionLayers(s_event.selection, true);

		return -1;
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

A_Err
IdleHook(AEGP_GlobalRefcon plugin_refconP, AEGP_IdleRefcon refconP, A_long *max_sleepPL)
{
    A_Err                   err = A_Err_NONE;

    if (s_event.handled == false)
    {
        std::unordered_map<UINT_PTR, std::string> commands;
        std::string         returnValue;
        std::string         error;

        s_event.handled = true;
        auto hMenu = BuildPopupMenu(commands);
        auto key = (UINT_PTR)TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, s_event.pt.x, s_event.pt.y, 0, s_hWnd, NULL);
        DestroyMenu(hMenu);

        if (commands.count(key) > 0)
        {
            ERR(AEUT_ExecuteScript(commands[key], returnValue, error));
        }

        if (error.length() > 0)
        {
            ERR(AEUT_ReportInfo(error));
        }
    }

	return A_Err_NONE;
}

A_Err
EntryPointFunc(
	struct SPBasicSuite		*pica_basicP,			/* >> */
	A_long				 	major_versionL,			/* >> */		
	A_long					minor_versionL,			/* >> */		
	AEGP_PluginID			aegp_plugin_id,			/* >> */
	AEGP_GlobalRefcon		*global_refconP)		/* << */
{
	A_Err                   err = A_Err_NONE;
	AEGP_SuiteHandler       suites(pica_basicP);
    std::string             returnValue, errorMessage;

    ERR(AEUT_Init(aegp_plugin_id, pica_basicP));

    ERR(suites.RegisterSuite5()->AEGP_RegisterIdleHook(aegp_plugin_id, IdleHook, NULL));
    ERR(suites.UtilitySuite6()->AEGP_GetMainHWND(&s_hWnd));

    if (err == A_Err_NONE)
    {
        auto hInst = (HINSTANCE)GetWindowLongPtr(s_hWnd, GWLP_HINSTANCE);
        SetWindowsHookEx(WH_MOUSE, MouseProc, hInst, GetCurrentThreadId());
    }
    else
    {
        AEUT_ReportInfo("An error has occurred at enabling QuickMenu Plug-in");
    }

	return err;
}
