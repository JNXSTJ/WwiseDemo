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
// AudioInputPlugin.cpp
//
// Audio Input Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioInputPluginGUI.h"

#include "../resource.h"
#include "../Help/TopicAlias.h"

// Determine what dialog just get called and set the property names to UI control binding populated table.
bool AudioInputGUI::GetDialog( AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable ) const
{
	switch ( in_eDialog )
	{
	case AK::Wwise::Plugin::SettingsDialog:
		out_uiDialogID = IDD_AUDIOINPUTPLUGIN_BIG;
		out_pTable = NULL;
		return true;

	case AK::Wwise::Plugin::ContentsEditorDialog:
		out_uiDialogID = IDD_AUDIOINPUTPLUGIN_SMALL;
		out_pTable = NULL;
		return true;
	}

	return false;
}

// Implement online help when the user clicks on the "?" icon .
bool AudioInputGUI::Help( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode ) const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() ) ;

	DWORD dwTopic = ONLINEHELP::Audio_Input_Property;
	if ( in_eDialog == AK::Wwise::Plugin::ContentsEditorDialog )
		dwTopic = ONLINEHELP::Audio_Input_Contents;

	::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, dwTopic, 0 );

	return true;
}

AK_ADD_PLUGIN_CLASS_TO_CONTAINER(AkAudioInput, AudioInputGUI, AkMicrophoneSource);
