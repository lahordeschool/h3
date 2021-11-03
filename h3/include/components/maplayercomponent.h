#ifndef _H3_COMPONENTS_MAPCOMPONENT_H_
#define _H3_COMPONENTS_MAPCOMPONENT_H_

#include <h3.h>

#define MAPLAYERCOMPONENT_TYPEID 0x00000004

H3_CAPI_BEGIN_BLOCK
void MapLayerComponent_Terminate(void* properties);
void MapLayerComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties);

void* MapLayerComponent_CreateProperties(H3Handle map, const char* layerName);
H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(MapLayerComponent, H3Handle, Map);
H3_CAPI_END_BLOCK

#define MAPLAYERCOMPONENT_CREATE(MAP, LAYERNAME)                                 \
	(SH3Component) {                                                             \
		.Terminate          = MapLayerComponent_Terminate,                       \
		.Draw               = MapLayerComponent_Draw,                            \
		.isInitialized      = false,                                             \
		.componentType      = MAPLAYERCOMPONENT_TYPEID,                          \
		.properties         = MapLayerComponent_CreateProperties(MAP, LAYERNAME) \
	}

#endif /* _H3_COMPONENTS_MAPCOMPONENT_H_ */
