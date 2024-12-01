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

#include "stdafx.h"
#include "DelayPluginGUI.h"

#include "../resource.h"
#include "../Help/TopicAlias.h"

// Bind non static text UI controls to properties for property view
AK_WWISE_PLUGIN_GUI_WINDOWS_BEGIN_POPULATE_TABLE(DelayProp)
	AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(IDC_CHECK_FEEDBACKENABLED, szFeedbackEnabled)
	AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(IDC_CHECK_PROCESSLFE, szProcessLFE)
AK_WWISE_PLUGIN_GUI_WINDOWS_END_POPULATE_TABLE()

// Take necessary action on property changes.
// Note: user also has the option of catching appropriate message in WindowProc function.
void DelayGUI::NotifyPropertyChanged( const GUID & in_guidPlatform, const char* in_szPropertyName )
{
	if ( !strcmp( in_szPropertyName, szFeedbackEnabled ) )
	{
		if ( m_hwndPropView )
			EnableFeedback( );
	}
}

// Set the property names to UI control binding populated table.
bool DelayGUI::GetDialog( AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable ) const
{
	AKASSERT( in_eDialog == AK::Wwise::Plugin::SettingsDialog );

	out_uiDialogID = IDD_DELAY_BIG;
	out_pTable = DelayProp;

	return true;
}

// Standard window function, user can intercept what ever message that is of interest to him to implement UI behavior.
bool DelayGUI::WindowProc( AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
{
	switch ( in_message )
	{
	case WM_INITDIALOG:
		m_hwndPropView = in_hWnd;
		EnableFeedback( );
		break;
	case WM_DESTROY:
		m_hwndPropView = NULL;
		break;
	}

	out_lResult = 0;
	return false;
}

// Implement online help when the user clicks on the "?" icon .
bool DelayGUI::Help( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode ) const
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() ) ;

	if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
		::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, ONLINEHELP::Delay_Properties, 0 );
	else
		return false;

	return true;
}

// Enable/disable feedback slider based on feedback enable checkbox
void DelayGUI::EnableFeedback( )
{
	auto feedbackEnabled = m_propertySet.GetBool( m_host.GetCurrentPlatform(), szFeedbackEnabled );
	HWND hwndItem = GetDlgItem( m_hwndPropView, IDC_RANGE_FEEDBACK );
	AKASSERT( hwndItem );
	::EnableWindow( hwndItem, MKBOOL(feedbackEnabled) );
	hwndItem = GetDlgItem( m_hwndPropView, IDC_STATIC_FEEDBACK );
	AKASSERT( hwndItem );
	::EnableWindow( hwndItem, MKBOOL(feedbackEnabled) );
}

AK_ADD_PLUGIN_CLASS_TO_CONTAINER(AkDelay, DelayGUI, AkDelayFX);		// Add our GUI class to the PluginContainer
