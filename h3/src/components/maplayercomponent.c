#include <components/maplayercomponent.h>

#include <stdlib.h>

typedef struct
{
	H3Handle Map;
	size_t   LayerID;
} MapLayerComponent_Properties;

void MapLayerComponent_Terminate(void* properties)
{
	free(properties);
}

void MapLayerComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties)
{
	MapLayerComponent_Properties* props = (MapLayerComponent_Properties*)properties;
	H3_Map_DrawLayer(h3, props->Map, props->LayerID);
}

void* MapLayerComponent_CreateProperties(H3Handle map, const char* layerName)
{
	MapLayerComponent_Properties* properties = malloc(sizeof(MapLayerComponent_Properties));

	properties->Map = map;
	properties->LayerID = H3_Map_GetLayerFromName(map, layerName);
	return properties;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(MapLayerComponent, H3Handle, Map);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(MapLayerComponent, size_t,   LayerID);
