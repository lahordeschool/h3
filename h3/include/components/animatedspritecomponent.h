#ifndef _H3_COMPONENTS_ANIMATEDSPRITECOMPONENT_H_
#define _H3_COMPONENTS_ANIMATEDSPRITECOMPONENT_H_

#include <h3.h>

#define ANIMATEDSPRITECOMPONENT_TYPEID 0x00000002

H3_CAPI_BEGIN_BLOCK
void AnimatedSpriteComponent_Terminate(void* properties);
void AnimatedSpriteComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties);
void AnimatedSpriteComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties);

void* AnimatedSpriteComponent_CreateProperties(const char* texturePath, uint8_t anchor, uint32_t numFrames, float frameDuration, bool loop);

void AnimatedSpriteComponent_ResetAnimation(H3Handle object);
bool AnimatedSpriteComponent_IsAnimOver(H3Handle object);

H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, H3Handle,         Texture);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, EH3Anchor,        Anchor);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, uint32_t,         NumFrames);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, float,            FrameDuration);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, bool,             FlipX);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, bool,             FlipY);
H3_CAPI_END_BLOCK

#define ANIMATEDSPRITECOMPONENT_CREATE(TEXTURE, ANCHOR, NFRAMES, FRAMEDURATION, LOOP)                                          \
	(SH3Component) {                                                                                                           \
		.Terminate          = AnimatedSpriteComponent_Terminate,                                                               \
		.Update             = AnimatedSpriteComponent_Update,                                                                  \
		.Draw               = AnimatedSpriteComponent_Draw,                                                                    \
		.isInitialized      = false,                                                                                           \
		.componentType      = ANIMATEDSPRITECOMPONENT_TYPEID,                                                                  \
		.properties         = AnimatedSpriteComponent_CreateProperties(TEXTURE, (uint8_t)ANCHOR, NFRAMES, FRAMEDURATION, LOOP) \
	}

#endif /* _H3_COMPONENTS_ANIMATEDSPRITECOMPONENT_H_ */
