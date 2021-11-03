#include <components/spritecomponent.h>

#include <stdlib.h>

typedef struct
{
	H3Handle         Texture;
	EH3Anchor Anchor;
} SpriteComponent_Properties;

void SpriteComponent_Terminate(void* properties)
{
	SpriteComponent_Properties* props = (SpriteComponent_Properties*)properties;

	if (props->Texture)
		H3_Texture_Destroy(&props->Texture);

	free(properties);
}

void SpriteComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties)
{
	SpriteComponent_Properties* props = (SpriteComponent_Properties*)properties;
	H3_Texture_DrawEx(h3, transform, props->Texture, props->Anchor);
}

void* SpriteComponent_CreateProperties(const char* texturePath, uint8_t anchor)
{
	SpriteComponent_Properties* properties = malloc(sizeof(SpriteComponent_Properties));
	H3_ASSERT_CONSOLE(properties, "Failed to allocate properties");

	uint32_t w, h;
	properties->Texture = H3_Texture_Load(texturePath, &w, &h);
	properties->Anchor = (EH3Anchor)anchor;
	return properties;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(SpriteComponent, H3Handle, Texture);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(SpriteComponent, EH3Anchor, Anchor);
