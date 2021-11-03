#include <components/cameracomponent.h>

#include <stdlib.h>

typedef struct
{
	float Width;
	float Height;
} CameraComponent_Properties;

void CameraComponent_Terminate(void* properties)
{
	free(properties);
}

void CameraComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties)
{
	CameraComponent_Properties* props = (CameraComponent_Properties*)properties;
	H3_SetView(h3, transform, props->Width, props->Height);
}

void* CameraComponent_CreateProperties(float width, float height)
{
	CameraComponent_Properties* properties = malloc(sizeof(CameraComponent_Properties));
	H3_ASSERT_CONSOLE(properties, "Failed to allocate properties");

	properties->Width  = width;
	properties->Height = height;

	return properties;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(CameraComponent, float, Width);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(CameraComponent, float, Height);
