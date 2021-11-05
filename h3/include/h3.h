#ifndef _H3_H_
#define _H3_H_

#include <h3/common.h>

#define H3_COLLIDER_COMPONENT_FLAG        0x10000000
#define H3_CUSTOM_BEHAVIOR_COMPONENT_FLAG 0x20000000

H3_CAPI_BEGIN_BLOCK
typedef enum
{
	K_Left,
	K_Right,
	K_Up,
	K_Down,
	K_Space,
	K_Enter,
	K_Tab,
	K_Backspace,
	K_Escape,
	K_Shift,
	K_Ctrl,

	K_F2,

	EH3Key_Count
} EH3Key;

typedef enum
{
	MB_Left,
	MB_Middle,
	MB_Right,

	EH3MouseButton_Count
} EH3MouseButton;

typedef enum
{
	A_Left   = 0x01,
	A_Center = 0x02,
	A_Right  = 0x04,

	A_Top    = 0x10,
	A_Middle = 0x20,
	A_Bottom = 0x40
} EH3Anchor;

typedef enum
{
	CST_Circle,
	CST_Box
} EH3ColliderShapeType;


typedef enum
{
	CDT_Static,
	CDT_Kinematic,
	CDT_Dynamic
} EH3ColliderDynamicsType;

typedef struct
{
	uint32_t    width;
	uint32_t    height;
	bool        fullscreen;

	const char* windowTitle;
} SH3InitParams;

typedef void SH3Transform;

typedef struct
{
	EH3ColliderShapeType    shapeType;
	EH3ColliderDynamicsType dynamicsType;
	union
	{
		struct { float radius; }                         circle;
		struct { float width, height; uint32_t anchor; } box;
	} shapeData;

	struct { float x, y; } offset;

	struct
	{
		float mass;
	} dynamicsData;

	bool isTrigger;
} SH3ColliderDesc;

#define H3_CIRCLE_COLLIDER(DT, R, TRIG) ((SH3ColliderDesc) { \
	.dynamicsType = DT, \
	.shapeType = CST_Circle, \
	.shapeData = { .circle = { .radius = R }}, \
	.isTrigger = TRIG \
})

#define H3_BOX_COLLIDER(DT, W, H, A, TRIG) ((SH3ColliderDesc) { \
	.dynamicsType = DT, \
	.shapeType = CST_Box, \
	.shapeData = { .box = { .width = W, .height = H, .anchor = A }}, \
	.isTrigger = TRIG \
})

#define H3_MAX_COLLISION_CONTACT_POINTS 8

typedef struct
{
	float x, y;
} SH3ContactPoint;

typedef struct
{
	H3Handle other;

	struct { float x, y; } normal;
	uint32_t        numContactPoints;
	SH3ContactPoint contactPoints[8];
} SH3Collision;

typedef struct
{
	// Default component callbacks
	void  (*Init)(SH3Transform*, void**);
	void  (*Terminate)(void*);
	void  (*Update)(H3Handle, H3Handle, SH3Transform*, float /* t */, float /* dt */, void*);
	void  (*PreUpdate)(H3Handle, H3Handle, SH3Transform*, float /* t */, float /* dt */, void*);
	void  (*Draw)(H3Handle, SH3Transform*, void*);

	void (*OnCollisionEnter)(H3Handle, SH3Collision);
	void (*OnCollisionLeave)(H3Handle, H3Handle);
	void (*OnTriggerEnter)(H3Handle, SH3Collision);
	void (*OnTriggerLeave)(H3Handle, H3Handle);

	bool     isInitialized;
	uint32_t componentType;
	void*    properties;
} SH3Component;

typedef struct
{
	H3Handle font;
	uint32_t size;
	struct { uint8_t r, g, b, a; } fillColor;

	bool hasOutline;
	struct { uint8_t r, g, b, a; } outlineColor;
	float outlineThickness;

	uint32_t anchor;

	bool isBold;
	bool isItalic;
	bool isUnderlined;

	bool isViewLocal;
} SH3TextProperties;

H3Handle               H3_Init(SH3InitParams params);
void                   H3_Terminate(H3Handle* h3);

float                  H3_GetTime();
float                  H3_GetDeltaTime();

bool                   H3_Input_IsKeyDown(EH3Key k);
bool                   H3_Input_IsKeyPressed(EH3Key k);
void                   H3_Input_GetMousePos(H3Handle h3, int* x, int* y);
bool                   H3_Input_IsMouseBtnDown(EH3MouseButton btn);
bool                   H3_Input_IsMouseBtnPressed(EH3MouseButton btn);

H3Handle               H3_Scene_Create(H3Handle h3, bool physicsLocksRotation);
void                   H3_Scene_Destroy(H3Handle* scene);
void                   H3_Scene_SetGravity(H3Handle scene, float x, float y);

H3Handle               H3_Object_Create(H3Handle scene, const char* objName, H3Handle parent);
void                   H3_Object_Destroy(H3Handle* object, bool recursive);
H3Handle               H3_Object_Get(H3Handle scene, const char* objName);
void                   H3_Object_SetEnabled(H3Handle object, bool enabled);
bool                   H3_Object_GetEnabled(H3Handle object);
const char*            H3_Object_GetName(H3Handle object);
int32_t                H3_Object_GetRenderOrder(H3Handle object);
void                   H3_Object_SetRenderOrder(H3Handle object, int32_t renderOrder);
SH3Transform*          H3_Object_GetTransform(H3Handle object);
void                   H3_Object_ResetTransform(H3Handle object);
void                   H3_Object_Translate(H3Handle object, float x, float y);
void                   H3_Object_Rotate(H3Handle object, float angle);
void                   H3_Object_Scale(H3Handle object, float s);
void                   H3_Object_SetTranslation(H3Handle object, float x, float y);
void                   H3_Object_SetRotation(H3Handle object, float angle);
void                   H3_Object_SetScale(H3Handle object, float s);
void                   H3_Object_ScaleNonUniform(H3Handle object, float x, float y);
void                   H3_Object_AddComponent(H3Handle object, SH3Component component);
SH3Component*          H3_Object_GetComponent(H3Handle object, uint32_t componentId);
bool                   H3_Object_HasComponent(H3Handle object, uint32_t componentId);
void                   H3_Object_EnablePhysics(H3Handle object, SH3ColliderDesc desc);
void                   H3_Object_EnablePhysicsEx(H3Handle object, SH3ColliderDesc* descList, uint32_t numShapes);
void                   H3_Object_DisablePhysics(H3Handle object);
void                   H3_Object_GetVelocity(H3Handle object, float* vx, float* vy);
void                   H3_Object_SetVelocity(H3Handle object, float vx, float vy);
void                   H3_Object_AddVelocity(H3Handle object, float vx, float vy);


void                   H3_Transform_GetPosition(SH3Transform* transform, float* x, float* y);

H3Handle               H3_Texture_Load(const char* path, uint32_t* width, uint32_t* height);
void                   H3_Texture_Destroy(H3Handle* handle);
void                   H3_Texture_Draw(H3Handle h3, float px, float py, H3Handle texture, EH3Anchor anchor);
void                   H3_Texture_DrawEx(H3Handle h3, SH3Transform* transform, H3Handle texture, EH3Anchor anchor);
void                   H3_Texture_DrawRectEx(H3Handle h3, SH3Transform* transform, H3Handle texture, float x0, float y0, float x1, float y1, EH3Anchor anchor);

H3Handle               H3_Map_Load(const char* path);
void                   H3_Map_Destroy(H3Handle* handle);
void                   H3_Map_RegisterObjectLayerForPhysicsInScene(H3Handle scene, H3Handle map, const char* layerName);
size_t                 H3_Map_GetLayerFromName(H3Handle map, const char* layerName);
void                   H3_Map_Draw(H3Handle h3, H3Handle map);
void                   H3_Map_DrawLayer(H3Handle h3, H3Handle map, size_t layerId);

H3Handle               H3_Sound_Load(const char* path);
void                   H3_Sound_Destroy(H3Handle* snd);
void                   H3_Sound_Play(H3Handle snd, float volume, bool loop);
void                   H3_Sound_PlaySpatialized(H3Handle snd, float volume, float px, float py, bool loop);
void                   H3_Sound_Stop(H3Handle snd);
void                   H3_Sound_SetPosition(H3Handle snd, float px, float py);
void                   H3_Listener_SetLocation(float px, float py, float angle);
void                   H3_Listener_SetPosition(float px, float py);
void                   H3_Listener_SetRotation(float angle);

H3Handle               H3_Music_Load(const char* path);
void                   H3_Music_Destroy(H3Handle* music);
void                   H3_Music_Play(H3Handle music, float volume, bool loop);
void                   H3_Music_Stop(H3Handle music);

H3Handle               H3_Font_Load(const char* path);
void                   H3_Font_Destroy(H3Handle* font);
void                   H3_Font_Printf(H3Handle h3, SH3TextProperties properties, SH3Transform* transform, const char* format, ...);

void                   H3_GetView(H3Handle h3, float* x, float* y, float* w, float* h, float* vpw, float* vph);
void                   H3_SetView(H3Handle h3, SH3Transform* transform, float w, float h);
void                   H3_SetView2(H3Handle h3, float x, float y, float w, float h);
bool                   H3_DoFrame(H3Handle h3, H3Handle scene);

H3_CAPI_END_BLOCK

#define H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW(C, T, N) \
	T C##_Get##N(void* properties); \
	void C##_Set##N(void* properties, T value);

#define H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO(C, T, N) \
	T C##_Get##N(void* properties);

#define H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(C, T, N) \
	T C##_Get##N(void* properties) { return ((C##_Properties*)properties)->N; } \
	void C##_Set##N(void* properties, T value) { ((C##_Properties*)properties)->N = value; }

#define H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(C, T, N) \
	T C##_Get##N(void* properties) { return ((C##_Properties*)properties)->N; }

#endif /* _H3_H_ */
