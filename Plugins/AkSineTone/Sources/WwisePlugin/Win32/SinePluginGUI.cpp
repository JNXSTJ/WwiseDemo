/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/
//////////////////////////////////////////////////////////////////////
//
// SinePluginGUI.cpp
//
// Sine generator Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SinePluginGUI.h"

#include "../resource.h"
#include "../Help/TopicAlias.h"

// Determine what dialog just get called and set the property names to UI control binding populated table.
bool SinePluginGUI::GetDialog(AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable) const
{
	switch ( in_eDialog )
	{
	case AK::Wwise::Plugin::SettingsDialog:
		out_uiDialogID = IDD_SINEPLUGIN_BIG;
		out_pTable = NULL;
		return true;

	case AK::Wwise::Plugin::ContentsEditorDialog:
		out_uiDialogID = IDD_SINEPLUGIN_SMALL;
		out_pTable = NULL;
		return true;
	}

	return false;
}

bool SinePluginGUI::WindowProc(AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult)
{
	out_lResult = 0;
	return false;
}

// Implement online help when the user clicks on the "?" icon .
bool SinePluginGUI::Help(HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode) const
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	DWORD dwTopic = ONLINEHELP::Sine_Properties;
	if (in_eDialog == AK::Wwise::Plugin::ContentsEditorDialog)
		dwTopic = ONLINEHELP::Sound_Object_Sine;

	::SendMessage(in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, dwTopic, 0);

	return true;
}

AK_ADD_PLUGIN_CLASS_TO_CONTAINER(AkSineTone, SinePluginGUI, SineSource);
