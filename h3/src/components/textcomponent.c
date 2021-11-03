#include <components/textcomponent.h>

#include <stdlib.h>

typedef struct
{
	const char*       Text;
	SH3TextProperties Props;
} TextComponent_Properties;

void TextComponent_Terminate(void* properties)
{
	free(properties);
}

void TextComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties)
{
	TextComponent_Properties* props = (TextComponent_Properties*)properties;
	H3_Font_Printf(h3, props->Props, transform, props->Text);
}


void* TextComponent_CreateProperties(const char* text, SH3TextProperties props)
{
	TextComponent_Properties* properties = malloc(sizeof(TextComponent_Properties));
	properties->Text  = text;
	properties->Props = props;
	return properties;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(TextComponent, const char*, Text);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(TextComponent, SH3TextProperties, Props);