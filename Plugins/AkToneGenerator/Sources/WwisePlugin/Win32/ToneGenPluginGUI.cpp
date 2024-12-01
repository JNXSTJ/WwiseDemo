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
// ToneGenPlugin.cpp
//
// Tone generator Wwise plugin implementation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToneGenPluginGUI.h"

#include "../resource.h"
#include "../Help/TopicAlias.h"

// Bind non static text UI controls to properties for property view
AK_WWISE_PLUGIN_GUI_WINDOWS_BEGIN_POPULATE_TABLE(ToneGenProp)
	AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(IDC_CHECK_SWEEPFREQ, szSweepFreq)
	AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(IDC_RADIO_FREQSWEEPLIN, szSweepFreqType)
	AK_WWISE_PLUGIN_GUI_WINDOWS_POP_ITEM(IDC_RADIO_FIXLENGTH, szDurMode)
AK_WWISE_PLUGIN_GUI_WINDOWS_END_POPULATE_TABLE()

// Take necessary action on property changes.
// Note: user also has the option of catching appropriate message in WindowProc function.
void ToneGenGUI::NotifyPropertyChanged( const GUID & in_guidPlatform, const char* in_szPropertyName )
{
	if ( !strcmp( in_szPropertyName, szWaveType ) )
	{
		if ( m_hwndPropView )
			EnableFrequencyControls( m_hwndPropView, AK::Wwise::Plugin::SettingsDialog );

		if ( m_hwndObjPane )
			EnableFrequencyControls( m_hwndObjPane, AK::Wwise::Plugin::ContentsEditorDialog );
	}
}

// Determine what dialog just get called and set the property names to UI control binding populated table.
bool ToneGenGUI::GetDialog( AK::Wwise::Plugin::eDialog in_eDialog, UINT & out_uiDialogID, AK::Wwise::Plugin::PopulateTableItem *& out_pTable ) const
{
	switch ( in_eDialog )
	{
	case AK::Wwise::Plugin::SettingsDialog:
		out_uiDialogID = IDD_TONEGENPLUGIN_BIG;
		out_pTable = ToneGenProp;

		return true;

	case AK::Wwise::Plugin::ContentsEditorDialog:
		out_uiDialogID = IDD_TONEGENPLUGIN_SMALL;
		out_pTable = NULL;

		return true;
	}

	return false;
}

// Standard window function, user can intercept what ever message that is of interest to him to implement UI behavior.
bool ToneGenGUI::WindowProc( AK::Wwise::Plugin::eDialog in_eDialog, HWND in_hWnd, UINT in_message, WPARAM in_wParam, LPARAM in_lParam, LRESULT & out_lResult )
{
	switch ( in_message )
	{
	case WM_INITDIALOG:
		{
			if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
				m_hwndPropView = in_hWnd;
			else if ( in_eDialog == AK::Wwise::Plugin::ContentsEditorDialog )
				m_hwndObjPane = in_hWnd;

			EnableFrequencyControls( in_hWnd, in_eDialog );
		}
		break;

	case WM_DESTROY:
		{
			if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
				m_hwndPropView = NULL;
			else if ( in_eDialog == AK::Wwise::Plugin::ContentsEditorDialog )
				m_hwndObjPane = NULL;
		}
		break;

	// Catch window command actions (regardless if it is object pane or property view) to enable/disable controls
	case WM_COMMAND:
		{
			// Notification code
			switch ( HIWORD( in_wParam ) )
			{

			case BN_CLICKED:

				switch ( LOWORD( in_wParam ) )
				{
				case IDC_CHECK_SWEEPFREQ:

					if ( IsDlgButtonChecked( in_hWnd, IDC_CHECK_SWEEPFREQ ) == BST_CHECKED )
					{
						EnableFrequencySweeping( in_hWnd, in_eDialog, true );	// Enable controls
					}
					else if ( IsDlgButtonChecked( in_hWnd, IDC_CHECK_SWEEPFREQ ) == BST_UNCHECKED )
					{
						EnableFrequencySweeping( in_hWnd, in_eDialog, false );	// Disable controls
					}

					break;

				case IDC_RADIO_FIXLENGTH:

					if ( IsDlgButtonChecked( in_hWnd, IDC_RADIO_FIXLENGTH ) == BST_CHECKED )
					{
						EnableEnvelopeControls( in_hWnd, in_eDialog, false ); // Disable envelope controls
					}
					else if ( IsDlgButtonChecked( in_hWnd, IDC_RADIO_FIXLENGTH ) == BST_UNCHECKED )
					{
						EnableEnvelopeControls( in_hWnd, in_eDialog, true ); // Enable controls
					}

					break;

				} // End switch low word

				break;	// End BN_CLICKED notification case

			} // End switch hi word (notification code)

		} // End command window event
		break;

	case WM_ENABLE:
		{
			// Enable/Disable all child controls

			HWND hWnd = ::GetWindow( in_hWnd, GW_CHILD );
			while( hWnd )
			{
				::EnableWindow( hWnd, in_wParam == TRUE );
				hWnd = ::GetWindow( hWnd, GW_HWNDNEXT );
			}

			if ( in_wParam == TRUE )
			{
				EnableFrequencyControls( in_hWnd, in_eDialog );
			}
		}
		return true;
	}

	out_lResult = 0;
	return false;
}

// Implement online help when the user clicks on the "?" icon .
bool ToneGenGUI::Help( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, const char* in_szLanguageCode ) const
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	DWORD dwTopic = ONLINEHELP::Tone_Generator_Properties;
	if ( in_eDialog == AK::Wwise::Plugin::ContentsEditorDialog )
		dwTopic = ONLINEHELP::Sound_Object_TG;

	::SendMessage( in_hWnd, WM_AK_PRIVATE_SHOW_HELP_TOPIC, dwTopic, 0 );

	return true;
}

// Enable/disable frequency sweeping controls
void ToneGenGUI::EnableFrequencySweeping( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, bool in_bEnable )
{
	if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
	{
		// Note: Check the wave type  before activating controls
		auto waveType = m_propertySet.GetInt32( m_host.GetCurrentPlatform(), szWaveType );
		// Only enable for oscillator types, always disabled when asked to however.
		if ( ( waveType == 0 || waveType == 1 || waveType == 2 || waveType == 3 ) || !in_bEnable )
		{
			EnableDlgItem( in_hWnd, IDC_RADIO_FREQSWEEPLIN, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_RADIO_FREQSWEEPLOG, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_STATIC_STOPFREQ, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_RANGE_STOPFREQ, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_STATIC_STOPFREQRANDMIN, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_RANGE_STOPFREQRANDMIN, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_STATIC_STOPFREQRANDMAX, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_RANGE_STOPFREQRANDMAX, in_bEnable );
			EnableDlgItem( in_hWnd, IDC_INTERPOLATION_STATIC, in_bEnable );
		}
	}
}

// Enable/disable envelope controls
void ToneGenGUI::EnableEnvelopeControls( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog, bool in_bEnable )
{
	if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
	{
		EnableDlgItem( in_hWnd, IDC_STATIC_ATTACKTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_ATTACKTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_DECAYTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_DECAYTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_SUSTAINTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_SUSTAINTIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_SUSTAINLEVEL, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_SUSTAINLEVEL, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_RELEASETIME, in_bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_RELEASETIME, in_bEnable );

		EnableDlgItem( in_hWnd, IDC_RANGE_FIXDUR, !in_bEnable );
	}
}

// Enable/disable all frequency controls
void ToneGenGUI::EnableFrequencyControls( HWND in_hWnd, AK::Wwise::Plugin::eDialog in_eDialog )
{
	auto waveType = m_propertySet.GetInt32( m_host.GetCurrentPlatform(), szWaveType );
	bool bEnable = ( waveType == 0 || waveType == 1 || waveType == 2 || waveType == 3 );

	// For both the object pane view and property view
	EnableDlgItem( in_hWnd, IDC_RANGE_STARTFREQ, bEnable );

	// Most controls only exist in property view
	if ( in_eDialog == AK::Wwise::Plugin::SettingsDialog )
	{
		EnableDlgItem( in_hWnd, IDC_BOX_STATIC_FREQUENCY, bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_STARTFREQ, bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_STARTFREQ, bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_STARTFREQRANDMIN, bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_STARTFREQRANDMIN, bEnable );
		EnableDlgItem( in_hWnd, IDC_STATIC_STARTFREQRANDMAX, bEnable );
		EnableDlgItem( in_hWnd, IDC_RANGE_STARTFREQRANDMAX, bEnable );
		EnableDlgItem( in_hWnd, IDC_CHECK_SWEEPFREQ, bEnable );

		// Enable sweep frequency control if necessary
		auto sweepFreq = m_propertySet.GetBool( m_host.GetCurrentPlatform(), szSweepFreq );
		bool bFreqSweep = sweepFreq && bEnable;
		EnableFrequencySweeping( in_hWnd, in_eDialog, bFreqSweep );
	}
}

void ToneGenGUI::EnableDlgItem( HWND in_hwndDialog, UINT in_uiDlgItem, BOOL in_bEnable )
{
	HWND hwndItem = GetDlgItem( in_hwndDialog, in_uiDlgItem );
	AKASSERT( hwndItem );
	::EnableWindow( hwndItem, in_bEnable );
}

AK_ADD_PLUGIN_CLASS_TO_CONTAINER(AkToneGen, ToneGenGUI, AkToneSource);
