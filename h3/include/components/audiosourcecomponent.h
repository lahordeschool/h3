#ifndef _H3_COMPONENTS_AUDIOSOURCECOMPONENT_H_
#define _H3_COMPONENTS_AUDIOSOURCECOMPONENT_H_

#include <h3.h>

#define AUDIOSOURCECOMPONENT_TYPEID 0x00000005

H3_CAPI_BEGIN_BLOCK
void AudioSourceComponent_Terminate(void* properties);
void AudioSourceComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties);

void* AudioSourceComponent_CreateProperties(H3Handle source);

void AudioSourceComponent_Play(H3Handle handle, float volume, bool loop);
void AudioSourceComponent_Stop(H3Handle handle);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(AudioSourceComponent, H3Handle, Source);
H3_CAPI_END_BLOCK

#define AUDIOSOURCECOMPONENT_CREATE(SOURCE)                                 \
	(SH3Component) {                                                        \
		.Terminate          = AudioSourceComponent_Terminate,               \
		.Update             = AudioSourceComponent_Update,                  \
		.isInitialized      = false,                                        \
		.componentType      = AUDIOSOURCECOMPONENT_TYPEID,                  \
		.properties         = AudioSourceComponent_CreateProperties(SOURCE) \
	}

#endif /* _H3_COMPONENTS_AUDIOSOURCECOMPONENT_H_ */
