
#include "gameobject.h"

namespace myengine
{
namespace audio
{
AudioGameObject::AudioGameObject(AkGameObjectID id): m_id(id)
{
}

bool AudioGameObject::PostEvent(std::string eventPath)
{
	AkPlayingID playing_id = AK::SoundEngine::PostEvent(eventPath.c_str(), m_id);
	return true;
}

}
}
