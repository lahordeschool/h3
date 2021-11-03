#ifndef _H3_COMPONENTS_SPRITECOMPONENT_H_
#define _H3_COMPONENTS_SPRITECOMPONENT_H_

#include <h3.h>

#define SPRITECOMPONENT_TYPEID 0x00000001

H3_CAPI_BEGIN_BLOCK
void SpriteComponent_Terminate(void* properties);
void SpriteComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties);

void* SpriteComponent_CreateProperties(const char* texturePath, uint8_t anchor);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(SpriteComponent, H3Handle, Texture);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(SpriteComponent, EH3Anchor, Anchor);
H3_CAPI_END_BLOCK

#define SPRITECOMPONENT_CREATE(TEXTURE, ANCHOR)                                          \
	(SH3Component) {                                                            \
		.Init               = NULL,                                                      \
		.Terminate          = SpriteComponent_Terminate,                                 \
		.Update             = NULL,                                                      \
		.Draw               = SpriteComponent_Draw,                                      \
		.isInitialized      = false,                                                     \
		.componentType      = SPRITECOMPONENT_TYPEID,                                    \
		.properties         = SpriteComponent_CreateProperties(TEXTURE, (uint8_t)ANCHOR) \
	}

#endif /* _H3_COMPONENTS_SPRITECOMPONENT_H_ */
