#include <components/audiosourcecomponent.h>

#include <stdlib.h>

typedef struct
{
	H3Handle Source;
} AudioSourceComponent_Properties;

void AudioSourceComponent_Terminate(void* properties)
{
	free(properties);
}

void AudioSourceComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties)
{
	AudioSourceComponent_Properties* props = (AudioSourceComponent_Properties*)properties;

	float x, y;
	H3_Transform_GetPosition(transform, &x, &y);
	H3_Sound_SetPosition(props->Source, x, y);
}

void* AudioSourceComponent_CreateProperties(H3Handle source)
{
	AudioSourceComponent_Properties* properties = malloc(sizeof(AudioSourceComponent_Properties));

	properties->Source = source;
	return properties;
}

void AudioSourceComponent_Play(H3Handle handle, float volume, bool loop)
{
	SH3Transform* xform = H3_Object_GetTransform(handle);

	SH3Component* cb = H3_Object_GetComponent(handle, AUDIOSOURCECOMPONENT_TYPEID);

	if (cb)
	{
		AudioSourceComponent_Properties* props = (AudioSourceComponent_Properties*)(cb->properties);

		float x, y;
		H3_Transform_GetPosition(xform, &x, &y);
		H3_Sound_PlaySpatialized(props->Source, volume, x, y, loop);
	}
	else
		H3_ASSERT_CONSOLE(false, "This object has no such component");
}

void AudioSourceComponent_Stop(H3Handle handle)
{
	SH3Component* cb = H3_Object_GetComponent(handle, AUDIOSOURCECOMPONENT_TYPEID);

	if (cb)
	{
		AudioSourceComponent_Properties* props = (AudioSourceComponent_Properties*)(cb->properties);
		H3_Sound_Stop(handle);
	}
	else
		H3_ASSERT_CONSOLE(false, "This object has no such component");
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(AudioSourceComponent, H3Handle, Source);
