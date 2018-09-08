#pragma once

#include <memory>
#include <string>
#include <list>

#include "AE_Macros.h"
#include "AE_EffectCBSuites.h"
#include "AEGP_SuiteHandler.h"

#define SAFE_FREE(m)		do { if (m != NULL) suites.MemorySuite1()->AEGP_FreeMemHandle(m); m = NULL; } while(0)
#define CONSOLE(str)		ERR(suites.UtilitySuite6()->AEGP_WriteToOSConsole(str));

static SPBasicSuite*		s_pica_basic;
static AEGP_PluginID		s_plugin_id;

A_Err
AEUT_Init(AEGP_PluginID plugin_id, SPBasicSuite* pica_basic)
{
	s_plugin_id = plugin_id;
	s_pica_basic = pica_basic;

	return A_Err_NONE;
}

A_Err
AEUT_ReportInfo(std::string message)
{
    A_Err					err = A_Err_NONE;
    AEGP_SuiteHandler		suites(s_pica_basic);

    ERR(suites.UtilitySuite6()->AEGP_ReportInfo(s_plugin_id, message.c_str()));

    return err;
}

A_Err
AEUT_WriteToConsole(const std::string& text)
{
    A_Err					err = A_Err_NONE;
    AEGP_SuiteHandler		suites(s_pica_basic);

    ERR(suites.UtilitySuite6()->AEGP_WriteToOSConsole(text.c_str()));

    return err;
}

A_Err
AEUT_EnableCommand(AEGP_Command command, A_Boolean enable)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);
	
	if (enable)
	{
		ERR(suites.CommandSuite1()->AEGP_EnableCommand(command));
	}
	else
	{
		ERR(suites.CommandSuite1()->AEGP_DisableCommand(command));
	}

	return err;
}

A_Err
AEUT_GetActiveCompH(AEGP_CompH* compH)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	AEGP_ItemH				active_itemH;
	AEGP_ItemType			item_type;

	ERR(suites.ItemSuite8()->AEGP_GetActiveItem(&active_itemH));
	if (active_itemH == NULL) return err;

	ERR(suites.ItemSuite8()->AEGP_GetItemType(active_itemH, &item_type));
	if (item_type != AEGP_ItemType_COMP) return err;

	ERR(suites.CompSuite10()->AEGP_GetCompFromItem(active_itemH, compH));

	return err;
}

A_Err
AEUT_GetSelectionLayers(std::list<AEGP_LayerH> &selection, A_Boolean sorted)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	A_u_long				num_items;
	AEGP_CompH				active_compH = NULL;
	AEGP_Collection2H		collection = NULL;
	AEGP_CollectionItemV2	collection_item;

	selection.clear();

	ERR(AEUT_GetActiveCompH(&active_compH));
	if (active_compH == NULL) return err;

	ERR(suites.CompSuite10()->AEGP_GetNewCollectionFromCompSelection(s_plugin_id, active_compH, &collection));
	if (collection == NULL) return err;

	ERR(suites.CollectionSuite2()->AEGP_GetCollectionNumItems(collection, &num_items));

	for (A_u_long i = 0; i < num_items; i++)
	{
		ERR(suites.CollectionSuite2()->AEGP_GetCollectionItemByIndex(collection, i, &collection_item));
		if (collection_item.type != AEGP_CollectionItemType_LAYER) continue;

		selection.push_back(collection_item.u.layer.layerH);
	}

	if (sorted)
	{
		selection.sort([&err, &suites](const AEGP_LayerH& a, const AEGP_LayerH& b) {
			A_long ia, ib;

			ERR(suites.LayerSuite8()->AEGP_GetLayerIndex(a, &ia));
			ERR(suites.LayerSuite8()->AEGP_GetLayerIndex(b, &ib));

			return ia < ib;
		});
	}

	return err;
}

A_Err
AEUT_StringFromMemHandle(AEGP_MemHandle source, std::string &dest)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	A_UTF16Char*			utf16;

	ERR(suites.MemorySuite1()->AEGP_LockMemHandle(source, (void**)&utf16));
	if (err != A_Err_NONE) return err;

	auto size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)utf16, -1, NULL, 0, NULL, NULL);
	auto cstr = std::unique_ptr<A_char[]>(new A_char[size]);

	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)utf16, -1, cstr.get(), size, NULL, NULL);
	dest = cstr.get();

	ERR(suites.MemorySuite1()->AEGP_UnlockMemHandle(source));

	return err;
}

A_Err
AEUT_GetLayerName(AEGP_LayerH layerH, std::string& name)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	AEGP_MemHandle			layer_name = NULL;
	AEGP_MemHandle			source_name = NULL;

	ERR(suites.LayerSuite8()->AEGP_GetLayerName(s_plugin_id, layerH, &layer_name, &source_name));
	ERR(AEUT_StringFromMemHandle(layer_name, name));
	if (name.length() < 1)
	{
		ERR(AEUT_StringFromMemHandle(source_name, name));
	}

	SAFE_FREE(layer_name);
	SAFE_FREE(source_name);

	return err;
}

A_Err
AEUT_GetLayerMarkerComment(AEGP_StreamRefH marker_stream, A_long index, std::string& comment)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	AEGP_MemHandle			marker_comment;
	AEGP_StreamValue2		valueP;

	ERR(suites.KeyframeSuite4()->AEGP_GetNewKeyframeValue(s_plugin_id, marker_stream, index, &valueP));
	ERR(suites.MarkerSuite2()->AEGP_GetMarkerString(s_plugin_id, valueP.val.markerP, AEGP_MarkerString_COMMENT, &marker_comment));

	ERR(AEUT_StringFromMemHandle(marker_comment, comment));

	ERR(suites.MemorySuite1()->AEGP_FreeMemHandle(marker_comment));
	ERR(suites.StreamSuite4()->AEGP_DisposeStreamValue(&valueP));

	return err;
}

A_Err
AEUT_GetLayerMarkerComments(AEGP_LayerH layerH, std::list<std::string> &comments)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	AEGP_StreamRefH			marker_stream;
	A_long					num_keyframes;
	std::string				comment;

	comments.clear();

	ERR(suites.StreamSuite4()->AEGP_GetNewLayerStream(s_plugin_id, layerH, AEGP_LayerStream_MARKER, &marker_stream));

	ERR(suites.KeyframeSuite4()->AEGP_GetStreamNumKFs(marker_stream, &num_keyframes));
	for (A_long i = 0; i < num_keyframes; i++)
	{
		ERR(AEUT_GetLayerMarkerComment(marker_stream, i, comment));
		comments.push_back(comment);
	}

	ERR(suites.StreamSuite4()->AEGP_DisposeStream(marker_stream));

	return err;
}

A_Err
AEUT_SetClipboardText(const std::string& text)
{
	A_Err					err = A_Err_NONE;
	AEGP_SuiteHandler		suites(s_pica_basic);

	HWND					hWnd;
	const A_char*			cstr = text.c_str();
	HGLOBAL					hGlobal = NULL;
	LPSTR					lpBuffer = NULL;
	bool					open = false;

	try
	{
		hGlobal = GlobalAlloc(GHND, text.length() + 1);
		if (hGlobal == NULL) throw A_Err_ALLOC;

		auto lpBuffer = (LPSTR)GlobalLock(hGlobal);
		if (lpBuffer == NULL) throw A_Err_GENERIC;

		auto copyed = strcpy_s((LPSTR)lpBuffer, text.length() + 1, text.c_str());
		if (copyed != 0) throw A_Err_GENERIC;

		GlobalUnlock(hGlobal);
		if (GetLastError() != NO_ERROR) throw A_Err_GENERIC;

		ERR(suites.UtilitySuite6()->AEGP_GetMainHWND(&hWnd));
		if (err != A_Err_NONE) throw err;

		open = OpenClipboard(hWnd);
		if (open == FALSE) throw A_Err_GENERIC;

		EmptyClipboard();

		auto hText = SetClipboardData(CF_TEXT, hGlobal);
		if (hText == NULL) throw A_Err_GENERIC;

		CloseClipboard();
	}
	catch (A_Err _err)
	{
		if (open) CloseClipboard();
		if (hGlobal) GlobalFree(hGlobal);

		err = _err;
	}

	return err;
}

A_Err
AEUT_ExecuteScript(const std::string& script, std::string& returnValue, std::string& errorMessage)
{
    A_Err					err = A_Err_NONE;
    AEGP_SuiteHandler		suites(s_pica_basic);

    AEGP_MemHandle          resultH = NULL;
    AEGP_MemHandle          errorH = NULL;
    A_char*                 resultBuf = NULL;
    A_char*                 errorBuf = NULL;

    ERR(suites.UtilitySuite6()->AEGP_ExecuteScript(s_plugin_id, script.c_str(), true, &resultH, &errorH));

    ERR(suites.MemorySuite1()->AEGP_LockMemHandle(resultH, (void**)&resultBuf));
    ERR(suites.MemorySuite1()->AEGP_LockMemHandle(errorH, (void**)&errorBuf));

    returnValue = resultBuf;
    errorMessage = errorBuf;

    SAFE_FREE(errorH);
    SAFE_FREE(resultH);

    return err;
}
