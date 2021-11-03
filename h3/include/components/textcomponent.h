#ifndef _H3_COMPONENTS_TEXTCOMPONENT_H_
#define _H3_COMPONENTS_TEXTCOMPONENT_H_

#include <h3.h>

#define SPRITECOMPONENT_TYPEID 0x00000001

H3_CAPI_BEGIN_BLOCK
void TextComponent_Terminate(void* properties);
void TextComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties);

void* TextComponent_CreateProperties(const char* text, SH3TextProperties props);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(TextComponent, const char*, Text);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(TextComponent, SH3TextProperties, Props);
H3_CAPI_END_BLOCK

#define TEXTCOMPONENT_CREATE(TEXT, PROPS)                                 \
	(SH3Component) {                                                      \
		.Terminate          = TextComponent_Terminate,                    \
		.Draw               = TextComponent_Draw,                         \
		.isInitialized      = false,                                      \
		.componentType      = SPRITECOMPONENT_TYPEID,                     \
		.properties         = TextComponent_CreateProperties(TEXT, PROPS) \
	}

#endif /* _H3_COMPONENTS_TEXTCOMPONENT_H_ */
