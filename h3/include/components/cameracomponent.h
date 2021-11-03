#ifndef _H3_COMPONENTS_CAMERACOMPONENT_H_
#define _H3_COMPONENTS_CAMERACOMPONENT_H_

#include <h3.h>

#define CAMERACOMPONENT_TYPEID 0x00000003

H3_CAPI_BEGIN_BLOCK
void CameraComponent_Terminate(void* properties);
void CameraComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties);

void* CameraComponent_CreateProperties(float width, float height);

H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(CameraComponent, float, Width);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(CameraComponent, float, Height);
H3_CAPI_END_BLOCK

#define CAMERACOMPONENT_CREATE(W, H)                                 \
	(SH3Component) {                                                 \
		.Terminate          = CameraComponent_Terminate,             \
		.Update             = CameraComponent_Update,                \
		.isInitialized      = false,                                 \
		.componentType      = CAMERACOMPONENT_TYPEID,                \
		.properties         = CameraComponent_CreateProperties(W, H) \
	}

#endif /* _H3_COMPONENTS_CAMERACOMPONENT_H_ */
