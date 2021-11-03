#include <components/rectanglecollidercomponent.h>

#include <stdlib.h>

typedef struct
{
	float Width;
	float Height;

	SH3Transform* GlobalTransform;
} RectangleColliderComponent_Properties;

void RectangleColliderComponent_Terminate(void* properties)
{
	free(properties);
}

void RectangleColliderComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties)
{
	RectangleColliderComponent_Properties* props = (RectangleColliderComponent_Properties*)properties;
	props->GlobalTransform = transform;
}

void* RectangleColliderComponent_CreateProperties(float width, float height)
{
	RectangleColliderComponent_Properties* properties = malloc(sizeof(RectangleColliderComponent_Properties));
	
	properties->Width  = width;
	properties->Height = height;

	return properties;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(RectangleColliderComponent, float, Width);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(RectangleColliderComponent, float, Height);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(RectangleColliderComponent, SH3Transform*, GlobalTransform);