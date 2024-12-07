#include <iostream>
#include <tracy/Tracy.hpp>
#include "audio.h"
namespace myengine
{
	namespace audio
	{
		void Audio::LoadBnk(const std::string& bnk_path)
		{
			AkBankID bnk_id;
			AK::SoundEngine::LoadBank(bnk_path.c_str(), bnk_id);
		}

		void Audio::PostEvent(AkGameObjectID gameObj, const std::string& eventPath)
		{
			AK::SoundEngine::PostEvent(eventPath.c_str(), gameObj);
		}

		AkGameObjectID Audio::RegisterGameObject(std::string name)
		{
			AK::SoundEngine::RegisterGameObj(++start_id, name.c_str());
			return start_id;
		}

		Audio::Audio()
		{
			m_pLowLevelIO = new CAkFilePackageLowLevelIODeferred();
			AkOSChar error[100];
			GetDefaultSettings();
			bool ret = InitWwise(error, 100);
			std::cout << "init wwise" << ret << std::endl;

		}

		void Audio::GetDefaultSettings()
		{
			AK::MemoryMgr::GetDefaultSettings(m_memSettings);
			AK::StreamMgr::GetDefaultSettings(m_stmSettings);
			AK::StreamMgr::GetDefaultDeviceSettings(m_deviceSettings);

			AK::SoundEngine::GetDefaultInitSettings(m_initSettings);
	#if defined( INTEGRATIONDEMO_ASSERT_HOOK )
			m_initSettings.pfnAssertHook = INTEGRATIONDEMO_ASSERT_HOOK;
	#endif // defined( INTEGRATIONDEMO_ASSERT_HOOK )

			// Set bDebugOutOfRangeCheck to true by default. This can help catch some errors, and it's not expensive!
	#if !defined(AK_OPTIMIZED)
			m_initSettings.bDebugOutOfRangeCheckEnabled = true;
	#endif

			AK::SoundEngine::GetDefaultPlatformInitSettings(m_platformInitSettings);
			AK::MusicEngine::GetDefaultInitSettings(m_musicInit);

			// The default job worker manager provided in the samples will generate more appropriate settings for the internal job manager of the sound engine.
			AK::JobWorkerMgr::GetDefaultInitSettings(m_jobWorkerSettings);
			m_jobWorkerSettings.uNumWorkerThreads = AK_MAX_WORKER_THREADS;
			m_initSettings.settingsJobManager = m_jobWorkerSettings.GetJobMgrSettings();
			for (AkUInt32 uJobType = 0; uJobType < AK_NUM_JOB_TYPES; ++uJobType)
			{
				m_initSettings.settingsJobManager.uMaxActiveWorkers[uJobType] = 2;
			}

	#if !defined AK_OPTIMIZED && !defined INTEGRATIONDEMO_DISABLECOMM
			AK::Comm::GetDefaultInitSettings(m_commSettings);
	#endif
		}

		bool Audio::InitWwise(
			AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
			unsigned int in_unErrorBufferCharCount	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
		)
		{
			//AK::IntegrationDemo::PlatformHooks::Get()->PreInit(this);

			//
			// Create and initialize an instance of the default memory manager. Note
			// that you can override the default memory manager with your own. Refer
			// to the SDK documentation for more information.
			//
			AKRESULT res = AK::MemoryMgr::Init(&m_memSettings);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::MemoryMgr::Init() returned AKRESULT %d"), res);
				return false;
			}

			//
			// Create and initialize an instance of the default streaming manager. Note
			// that you can override the default streaming manager with your own. Refer
			// to the SDK documentation for more information.
			//

			// Customize the Stream Manager settings here.

			if (!AK::StreamMgr::Create(m_stmSettings))
			{
				AKPLATFORM::SafeStrCpy(in_szErrorBuffer, AKTEXT("AK::StreamMgr::Create() failed"), in_unErrorBufferCharCount);
				return false;
			}

			// 
			// Create a streaming device with blocking low-level I/O handshaking.
			// Note that you can override the default low-level I/O module with your own. Refer
			// to the SDK documentation for more information.        
			//

			// CAkFilePackageLowLevelIODeferred::Init() creates a streaming device
			// in the Stream Manager, and registers itself as the File Location Resolver.
			m_deviceSettings.bUseStreamCache = true;
			res = m_pLowLevelIO->Init(m_deviceSettings);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("m_lowLevelIO.Init() returned AKRESULT %d"), res);
				return false;
			}

			//
			// Create and start the job worker threads that the Sound Engine will use
			// Using the default sample implementation supplied in the Wwise SDK samples
			//
			if (m_jobWorkerSettings.uNumWorkerThreads > 0)
			{
				res = AK::JobWorkerMgr::InitWorkers(m_jobWorkerSettings);
				if (res != AK_Success)
				{
					__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::JobWorkerMgr::InitWorkers() returned AKRESULT %d"), res);
					return false;
				}
			}

			//
			// Create the Sound Engine
			// Using default initialization parameters
			//
			res = AK::SoundEngine::Init(&m_initSettings, &m_platformInitSettings);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::SoundEngine::Init() returned AKRESULT %d"), res);
				return false;
			}

			//
			// Initialize the music engine
			// Using default initialization parameters
			//

			res = AK::MusicEngine::Init(&m_musicInit);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::MusicEngine::Init() returned AKRESULT %d"), res);
				return false;
			}

	#if !defined AK_OPTIMIZED && !defined INTEGRATIONDEMO_DISABLECOMM
			//
			// Initialize communications (not in release build!)
			//
			AKPLATFORM::SafeStrCpy(m_commSettings.szAppNetworkName, "Integration Demo", AK_COMM_SETTINGS_MAX_STRING_SIZE);

			res = AK::Comm::Init(m_commSettings);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::Comm::Init() returned AKRESULT %d. Communication between the Wwise authoring application and the game will not be possible."), res);
			}
	#endif // AK_OPTIMIZED	

			AkSpatialAudioInitSettings settings;

			res = AK::SpatialAudio::Init(settings);
			if (res != AK_Success)
			{
				__AK_OSCHAR_SNPRINTF(in_szErrorBuffer, in_unErrorBufferCharCount, AKTEXT("AK::SpatialAudio::Init() returned AKRESULT %d"), res);
				return false;
			}

			AK::SoundEngine::RegisterGameObj(LISTENER_ID, "Listener (Default)");
			AK::SoundEngine::SetDefaultListeners(&LISTENER_ID, 1);

			// For platforms with a read-only bank path, add a writable folder
			// to the list of base paths. IO will fallback on this path
			// when opening a file for writing fails.
	#if defined(WRITABLE_PATH)
			m_pLowLevelIO->AddBasePath(WRITABLE_PATH);
	#endif

			// Set the path to the SoundBank Files last.
			// The last base path is always the first queried for files.
			AkOSChar soundBankPath[] = L"C:\\Program Files (x86)\\Audiokinetic\\Wwise2024.1.0.8669\\SDK\\samples\\IntegrationDemo\\WwiseProject\\GeneratedSoundBanks\\Windows";
			m_pLowLevelIO->SetBasePath(soundBankPath);

			// Set global language. Low-level I/O devices can use this string to find language-specific assets.
			if (AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)")) != AK_Success)
				return false;

			AK::SoundEngine::RegisterResourceMonitorCallback(ResourceMonitorDataCallback);

			AK::Monitor::SetLocalOutput(AK::Monitor::ErrorLevel_All, LocalErrorCallback);

			return true;

		}


		//Menu* m_pMenu;	///< The menu system pointer.
		void Audio::ResourceMonitorDataCallback(const AkResourceMonitorDataSummary* in_pdataSummary)
		{
			static int ResourceMonitorUpdateCooldown = 0;
			if (ResourceMonitorUpdateCooldown <= 0)
			{
				Audio::ResourceDataSummary = *in_pdataSummary;
				ResourceMonitorUpdateCooldown = DATA_SUMMARY_REFRESH_COOLDOWN;
			}
			else
				ResourceMonitorUpdateCooldown--;
		}

		//The callback function called when the AK::Monitor does a localoutput
		static void LocalErrorCallback(
			AK::Monitor::ErrorCode in_eErrorCode,	///< Error code number value
			const AkOSChar* in_pszError,	///< Message or error string to be displayed
			AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
			AkPlayingID in_playingID,   ///< Related Playing ID if applicable, AK_INVALID_PLAYING_ID otherwise
			AkGameObjectID in_gameObjID ///< Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise
		);;
		static AkResourceMonitorDataSummary ResourceDataSummary;


		AkResourceMonitorDataSummary Audio::ResourceDataSummary;


		void Audio::LocalErrorCallback(AK::Monitor::ErrorCode in_eErrorCode, const AkOSChar* in_pszError, AK::Monitor::ErrorLevel in_eErrorLevel, AkPlayingID in_playingID, AkGameObjectID in_gameObjID)
		{
			int i = 0;
			return;
			//char* pszErrorStr;
			//CONVERT_OSCHAR_TO_CHAR(in_pszError, pszErrorStr);

			//if (in_eErrorLevel == AK::Monitor::ErrorLevel_Error)
			//{
			//	// Error handle Plugin Errors to be shown in plugin context.
			//	if (in_eErrorCode == AK::Monitor::ErrorCode_PluginFileNotFound || in_eErrorCode == AK::Monitor::ErrorCode_PluginNotRegistered || in_eErrorCode == AK::Monitor::ErrorCode_PluginFileRegisterFailed)
			//	{
			//		Instance().m_pMenu->SetPluginLog(pszErrorStr);
			//	}
			//	else
			//	{
			//		Page* pPage = Instance().m_pMenu->GetCurrentPage();
			//		if (pPage != nullptr)
			//		{
			//			Instance().m_pMenu->GetCurrentPage()->SetErrorMessage(pszErrorStr);
			//		}
			//	}
			//}
			//AKPLATFORM::OutputDebugMsgV("%s: %s\n", in_eErrorLevel == AK::Monitor::ErrorLevel_Message ? "AK Message" : "AK Error", pszErrorStr);
		}

	}
}
