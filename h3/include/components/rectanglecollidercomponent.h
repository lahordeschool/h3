#ifndef _H3_COMPONENTS_RECTANGLECOLLIDERCOMPONENT_H_
#define _H3_COMPONENTS_RECTANGLECOLLIDERCOMPONENT_H_

#include <h3.h>

#define RECTANGLECOLLIDERCOMPONENT_TYPEID (H3_COLLIDER_COMPONENT_FLAG | 0x00000001)

H3_CAPI_BEGIN_BLOCK
void RectangleColliderComponent_Terminate(void* properties);
void RectangleColliderComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties);

void* RectangleColliderComponent_CreateProperties(float width, float height);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(RectangleColliderComponent, float, Width);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(RectangleColliderComponent, float, Height);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(RectangleColliderComponent, SH3Transform*, GlobalTransform);
H3_CAPI_END_BLOCK

#define AUDIOSOURCECOMPONENT_CREATE(W, H)                                       \
	(SH3Component) {                                                            \
		.Terminate          = RectangleColliderComponent_Terminate,             \
		.Update             = RectangleColliderComponent_Update,                \
		.isInitialized      = false,                                            \
		.componentType      = RECTANGLECOLLIDERCOMPONENT_TYPEID,                \
		.properties         = RectangleColliderComponent_CreateProperties(W, H) \
	}

#endif /* _H3_COMPONENTS_RECTANGLECOLLIDERCOMPONENT_H_ */
