#include <iostream>
#include <tracy/Tracy.hpp>
#include <AK/SoundEngine/Common/AkTypes.h>

#include <AK/SoundEngine/Common/AkMemoryMgr.h>		// Memory Manager
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>			// Default memory and stream managers
#include <AK/SoundEngine/Common/IAkStreamMgr.h>		// Streaming Manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>	// Music Engine
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>	// AkStreamMgrModule
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>	// Spatial Audio module
#include <AK/SoundEngine/Common/AkCallback.h>    // Callback
#include "../Common/AkFilePackageLowLevelIO.h"
#include "AkDefaultIOHookDeferred.h"
#include "Platform.h"

typedef CAkFilePackageLowLevelIO<CAkDefaultIOHookDeferred> CAkFilePackageLowLevelIODeferred;
// layout is hard-coded to 1280*720 (16:9) for consistent layout on all platforms
static const AkUInt32 INTEGRATIONDEMO_LAYOUT_WIDTH = 1280;
static const AkUInt32 INTEGRATIONDEMO_LAYOUT_HEIGHT = 720;

static const AkUInt16 DESIRED_FPS = 60;
static const AkReal32 MS_PER_FRAME = (1000 / (AkReal32)DESIRED_FPS);

static const AkGameObjectID LISTENER_ID = 10000;

#define DATA_SUMMARY_REFRESH_COOLDOWN 7; // Refresh cooldown affecting the refresh rate of the resource monitor data summary

// Android devices show poor performance with more than 2 worker threads (and >2 is overkill for NX)
#if defined(AK_SUPPORT_THREADS)
#if defined(AK_ANDROID) || defined(AK_NX)
static const AkUInt32 AK_MAX_WORKER_THREADS = 2;
#else
static const AkUInt32 AK_MAX_WORKER_THREADS = 4;
#endif
#else
static const AkUInt32 AK_MAX_WORKER_THREADS = 0;
#endif

#include "AkJobWorkerMgr.h"

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>	// Communication between Wwise and the game (excluded in release build)
#endif

using namespace std;

namespace myengine
{
	namespace audio
	{
		class Audio
		{
		private:
			AkGameObjectID start_id = 100;
		public:
			static Audio& Instance()
			{
				static Audio audio;
				return audio;
			}
		public:
			void LoadBnk(const std::string& bnk_path);

			void PostEvent(AkGameObjectID gameObj, const std::string& eventPath);

			AkGameObjectID RegisterGameObject(std::string name);
		private:
			Audio();

			void Audio::GetDefaultSettings();

			/// Initializes the Wwise sound engine only, not the IntegrationDemo app.  Also used to change critical settings in DemoOption page.
			bool InitWwise(
				AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
				unsigned int in_unErrorBufferCharCount	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
			);

		private:
			
			int m_bnk_id;

		private:
			/// We're using the default Low-Level I/O implementation that's part
			/// of the SDK's sample code, with the file package extension
			CAkFilePackageLowLevelIODeferred* m_pLowLevelIO;

			//Settings kept for reinitialization in DemoOptions page.
			AkMemSettings			m_memSettings;
			AkStreamMgrSettings		m_stmSettings;
			AkDeviceSettings		m_deviceSettings;
			AK::JobWorkerMgr::InitSettings m_jobWorkerSettings;
			AkInitSettings			m_initSettings;
			AkPlatformInitSettings	m_platformInitSettings;
			AkMusicSettings			m_musicInit;

		#ifndef AK_OPTIMIZED
			AkCommSettings			m_commSettings;
			//AkXMLErrorMessageTranslator m_xmlMsgTranslator;
		#endif

		public:
			//Menu* m_pMenu;	///< The menu system pointer.
			static void ResourceMonitorDataCallback(const AkResourceMonitorDataSummary* in_pdataSummary);
			//The callback function called when the AK::Monitor does a localoutput
			static void LocalErrorCallback(
				AK::Monitor::ErrorCode in_eErrorCode,	///< Error code number value
				const AkOSChar* in_pszError,	///< Message or error string to be displayed
				AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
				AkPlayingID in_playingID,   ///< Related Playing ID if applicable, AK_INVALID_PLAYING_ID otherwise
				AkGameObjectID in_gameObjID ///< Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise
			);;
			static AkResourceMonitorDataSummary ResourceDataSummary;

		};



	}
}

