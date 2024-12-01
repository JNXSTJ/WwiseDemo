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

#pragma once

#include "../DelayPlugin.h"

class DelayGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
	, public AK::Wwise::Plugin::RequestHost
	, public AK::Wwise::Plugin::RequestPropertySet
{
public:
	DelayGUI() {}
	virtual void NotifyPropertyChanged( const GUID & in_guidPlatform, const char* in_szPropertyName ) override;

	virtual bool GetDialog( AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable ) const override;
	virtual bool WindowProc( AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult ) override;
	virtual bool Help( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode ) const override;

private:
	void EnableFeedback( );

	HWND m_hwndPropView = nullptr;
};
