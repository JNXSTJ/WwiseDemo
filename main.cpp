#include <iostream>
#include <tracy/Tracy.hpp>
#include "audio.h"



int main()
{
	ZoneScoped;
	myengine::audio::Audio& instance = myengine::audio::Audio::Instance();
	instance.LoadBnk("Init.bnk");
	bool flag = false;
	while (true)
	{
		::Sleep(1000);
		if (flag == false)
		{
			flag = true;
			instance.LoadBnk("Reflect.bnk");
			auto id = instance.RegisterGameObject("taojian");
			instance.PostEvent(id, "Play_Reflect_Emitter");
			AK::SoundEngine::RenderAudio();
		}
	}

	return 0;
}