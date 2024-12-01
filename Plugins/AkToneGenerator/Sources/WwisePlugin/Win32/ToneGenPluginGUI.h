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
// ToneGenPlugin.h
//
// Tone generator Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../ToneGenPlugin.h"

class ToneGenGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
	, public AK::Wwise::Plugin::RequestHost
	, public AK::Wwise::Plugin::RequestPropertySet
{
public:
	ToneGenGUI() {}

	virtual void NotifyPropertyChanged( const GUID & in_guidPlatform, const char* in_szPropertyName ) override;
	virtual bool GetDialog( AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable ) const override;
	virtual bool WindowProc( AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult ) override;
	virtual bool Help( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode ) const override;

private:
	// Window action controls
	// Enable/disable frequency sweeping controls
	void EnableFrequencySweeping( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, bool in_bEnable );
	// Enable/disable envelope controls
	void EnableEnvelopeControls( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, bool in_bEnable );
	// Enable/disable all frequency controls
	void EnableFrequencyControls( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog );

	// Enable a single dialog item
	static void EnableDlgItem( HWND in_hwndDialog, UINT in_uiDlgItem, BOOL in_bEnable );

	HWND m_hwndPropView = nullptr;
	HWND m_hwndObjPane = nullptr;
};
