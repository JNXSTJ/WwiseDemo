
#include <string>
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

namespace myengine
{
	namespace audio
	{
		class AudioGameObject
		{
		public:
			AudioGameObject(AkGameObjectID id);

			bool PostEvent(std::string event_name);

		private:
			AkGameObjectID m_id;
		};

	}
}