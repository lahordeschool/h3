/** \file */

/**
 * Hydrogen3 - The 2D game engine for La Horde
 */

#ifndef _H3_H_
#define _H3_H_

#include <h3/common.h>

#define H3_COLLIDER_COMPONENT_FLAG        0x10000000
#define H3_CUSTOM_BEHAVIOR_COMPONENT_FLAG 0x20000000

H3_CAPI_BEGIN_BLOCK

/**
 * Keyboard keys
 */
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

/**
 * Mouse buttons
 */
typedef enum
{
	MB_Left,
	MB_Middle,
	MB_Right,

	EH3MouseButton_Count
} EH3MouseButton;

/**
 * Anchor flag.
 * A full anchor is a bitwise OR of a horizontal and a vertical anchor flag, ie. (A_Center | A_Bottom)
 */
typedef enum
{
	A_Left   = 0x01,
	A_Center = 0x02,
	A_Right  = 0x04,

	A_Top    = 0x10,
	A_Middle = 0x20,
	A_Bottom = 0x40
} EH3Anchor;

/**
 * Collider shapes.
 * Only supporting circles and boxes (rectangles) for now.
 */
typedef enum
{
	CST_Circle,
	CST_Box
} EH3ColliderShapeType;

/**
 * Collider dynamics type.
 * This influences how the physics engine considers the collider.
 */
typedef enum
{
	CDT_Static,    /**< Static collider (will never move) */
	CDT_Kinematic, /**< Kinematic collider (can be moved by code) */
	CDT_Dynamic    /**< Dynamic collider (can be moved by code or by the physics engine) */
} EH3ColliderDynamicsType;

/**
 * Physics joint type
 */
typedef enum
{
	JT_Distance,  //!< Linear distance (spring-like)
	JT_Revolute,  //!< Revolute (rotating)
	JT_Prismatic, //!< Prismatic (One-axis rotation, like a piston)
	JT_Wheel,     //!< Wheel. Designed for vehicles, includes suspension.

	EH3JointType_Count
} EH3JointType;

/**
 * Descriptor of a physics joint. Fill only the union part you need.
 */
typedef struct
{
	EH3JointType type;                              //!< The joint type

	H3Handle     body1;                             //!< First body to attach
	H3Handle     body2;                             //!< Second body to attach

	struct { float x; float y; } localAnchor1;      //!< Anchor relative to body1's origin
	struct { float x; float y; } localAnchor2;      //!< Anchor relative to body2's origin

	union {
		struct {
			float restLength;                       //!< Rest length of this joint
			float minLength;                        //!< Minimum length
			float maxLength;                        //!< Maximum length
			float stiffness;                        //!< Linear stiffness in N/m
			float damping;                          //!< Linear damping in N.s/m
		} distance;

		struct {
			float referenceAngle;                   //!< The (body2 - body1) angle in the reference state
			bool  enableLimits;                     //!< Enable lower/upper limits ?
			float lowerAngle;                       //!< Lower angle limit
			float upperAngle;                       //!< Upper angle limit
		} revolute;

		struct {
			struct { float x; float y; } localAxis; //!< Translation axis relative to body1
			float referenceAngle;                   //!< The (body2 - body1) angle in the reference state

			bool  enableLimits;                     //!< Enable lower/upper limits ?
			float lowerTranslation;                 //!< lower translation limit
			float upperTranslation;                 //!< upper translation limit
		} prismatic;

		struct {
			struct { float x; float y; } localAxis; //!< Translation axis relative to body1

			bool  enableLimits;                     //!< Enable lower/upper limits ?
			float lowerTranslation;                 //!< lower translation limit
			float upperTranslation;                 //!< upper translation limit
		
			float stiffness;                        //!< suspension stiffness in N/m
			float damping;                          //!< suspension damping in N.s/m
		} wheel;
	} data;
} SH3JointDesc;

/**
 * H3 initialization parameters, passed to H3_Init
 */
typedef struct
{
	uint32_t    width;       //!< The window width, in pixels
	uint32_t    height;      //!< The window height, in pixels
	bool        fullscreen;  //!< true to make the window fullscreen, false otherwise

	const char* windowTitle; //!< The window title. Can be anything and is displayed in the window's title bar.
} SH3InitParams;

typedef void SH3Transform;

/**
 * A collider descriptor.
 * Do not use directly, use the H3_XXX_COLLIDER macros instead.
 */
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
	float rotation;

	struct
	{
		float mass;
	} dynamicsData;

	bool isTrigger;
} SH3ColliderDesc;

/**
 * Circle collider macro
 * \param DT   The dynamics type as a EColliderDynamicsType
 * \param R    The circle's radius in world units
 * \param TRIG true if this collider should be a trigger, false otherwise
 */
#define H3_CIRCLE_COLLIDER(DT, R, TRIG) ((SH3ColliderDesc) { \
	.dynamicsType = DT, \
	.shapeType = CST_Circle, \
	.shapeData = { .circle = { .radius = R }}, \
	.isTrigger = TRIG \
})

 /**
  * Circle collider macro
  * \param DT   The dynamics type as a EColliderDynamicsType
  * \param W    The box's width in world units
  * \param H    The box's height in world units
  * \param A    The box's anchor point (see EH3Anchor)
  * \param TRIG true if this collider should be a trigger, false otherwise
  */
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

/**
 * The defining structure of a component.
 * 
 * Hydrogen3 follows an object/component paradigm where objects are basically empty boxes
 * that the developer can add features to using components.
 * 
 * A component is a set of functions called throughout the whole object's lifetime, associated with a memory blob containing its specific properties.
 * Functions are stored as function pointers ("callbacks") and the properties are stored as an opaque pointer to force the component's developer to properly
 * manage their own memory.
 */
typedef struct
{
	// Default component callbacks
	void  (*Init)(SH3Transform* /* transform */, void** /* propertiesPtr */);                                                                           //!< Called when the object is first used
	void  (*Terminate)(void* /* properties */);                                                                                                         //!< Called when the object is destroyed
	void  (*Update)(H3Handle /* h3 */, H3Handle /* object */, SH3Transform* /* transform */, float /* t */, float /* dt */, void* /* properties */);    //!< Called on each frame, after the physics update
	void  (*PreUpdate)(H3Handle /* h3 */, H3Handle /* object */, SH3Transform* /* transform */, float /* t */, float /* dt */, void* /* properties */); //!< Called on each frame, before the physics update
	void  (*Draw)(H3Handle /* h3 */, SH3Transform* /* transform */, void* /* properties */);                                                            //!< Called on each frame, after all is updated. Used to actually draw to screen.

	// Physics callbacks are called BETWEEN PreUpdate and Update.

	void (*OnCollisionEnter)(H3Handle /* self */, SH3Collision /* collision */);                                                                        //!< Called when this object starts to interact with another.
	void (*OnCollisionLeave)(H3Handle /* self */, H3Handle /* other */);                                                                                //!< Called when this object ceases to interact with another.
	void (*OnTriggerEnter)(H3Handle /* self */, SH3Collision /* collision */);                                                                          //!< Called when this object starts to interact with another (that is a trigger).
	void (*OnTriggerLeave)(H3Handle /* self */, H3Handle /* other */);                                                                                  //!< Called when this object ceases to interact with another (that is a trigger).

	bool     isInitialized;                                                                                                                             //!< Whether this component's Init() function has been called
	uint32_t componentType;                                                                                                                             //!< This component's type (see "..._TYPEID")
	void*    properties;                                                                                                                                //!< This component's properties as an opaque pointer
} SH3Component;

/**
 * Text rendering properties.
 */
typedef struct
{
	H3Handle font;                               //!< A handle to the font (loaded via H3_Font_Load)
	uint32_t size;                               //!< The font size in pixels
	struct { uint8_t r, g, b, a; } fillColor;    //!< The text color, RGBA[0,255]

	bool hasOutline;                             //!< Use outline?
	struct { uint8_t r, g, b, a; } outlineColor; //!< Outline color, RGBA [0,255]
	float outlineThickness;                      //!< Outline thickness, in pixels

	uint32_t anchor;                             //!< An EH3Anchor to use as a reference position for this text

	bool isBold;                                 //!< Use bold font?
	bool isItalic;                               //!< Use italic font?
	bool isUnderlined;                           //!< Underline text?

	bool isViewLocal;                            //!< Is this text local to the view or to the scene?
} SH3TextProperties;

// ============================================================================

/**
 * \brief Initializes H3 and opens a window
 * \param params The initialization parameters
 * \return The main handle to H3
 */
H3Handle H3_Init(SH3InitParams params);

/**
 * \brief Closes the window and deinitializes H3
 * \param h3 Pointer to the main handle as returned by H3_Init
 */
void H3_Terminate(H3Handle* h3);

/**
 * \return the time in seconds since H3_Init was called
 */
float H3_GetTime();

/**
 * \return the time in seconds since the previous frame started
 */
float H3_GetDeltaTime();

// ============================================================================

/**
 * \param k the keyboard key to test
 * \return true if the key is down, false otherwise
 */
bool H3_Input_IsKeyDown(EH3Key k);

/**
 * \param k the keyboard key to test
 * \return true on the first frame the key is down, false otherwise
 */
bool H3_Input_IsKeyPressed(EH3Key k);

/**
 * \brief Returns true on the first frame the key is down, false otherwise
 * \param h3 The window handle to test against
 * \param x  A pointer to the horizontal coordinate, in pixels
 * \param y  A pointer to the vertical coordinate, in pixels
 */
void H3_Input_GetMousePos(H3Handle h3, int* x, int* y);

/**
 * \brief Returns true if the mouse button is down, false otherwise
 * \param k the mouse button to test
 */
bool H3_Input_IsMouseBtnDown(EH3MouseButton btn);

/**
 * \brief Returns true on the first frame the mouse button is down, false otherwise
 * \param k the mouse button to test
 */
bool H3_Input_IsMouseBtnPressed(EH3MouseButton btn);

// ============================================================================

/**
 * \brief Creates a new scene.
 * \param h3                   The main H3 handle
 * \param physicsLocksRotation If true, objects colliding in this scene won't get any angular velocity from their collisions
 * \return The new scene
 */
H3Handle H3_Scene_Create(H3Handle h3, bool physicsLocksRotation);

/**
 * \brief Destroys a scene
 * \param scene The scene to destroy
 */
void H3_Scene_Destroy(H3Handle scene);

/**
 * \brief Sets a scene's gravity, which can be in any direction and of any strength
 * \param scene The scene to set the gravity into
 * \param x the horizontal value of gravity
 * \param y the vertical value of gravity
 */
void H3_Scene_SetGravity(H3Handle scene, float x, float y);

// ============================================================================

/**
 * \brief Creates a new object.
 * \param scene   The scene to create the object into
 * \param objName The name of the object to create
 * \param parent  The parent of the new object, or NULL to make it a root object
 * \return The new object
 */
H3Handle H3_Object_Create(H3Handle scene, const char* objName, H3Handle parent);

/**
 * \brief Destroy an object.
 * \param object    The object to destroy
 * \param recursive True to destroy all children objects as well, false otherwise
 */
void H3_Object_Destroy(H3Handle object, bool recursive);

/**
 * \brief Find an object in a scene
 * \param scene   The scene to look for the object in
 * \param objName The name of the object to look for
 * \return An object, or a NULL handle if none is found
 */
H3Handle H3_Object_Get(H3Handle scene, const char* objName);

/**
 * \brief Enables or disables an object. A disabled object is neither shown nor updated.
 * \param object  The object to enable/disable
 * \param enabled true to enable, false to disable
 */
void  H3_Object_SetEnabled(H3Handle object, bool enabled);

/**
 * \brief Gets the Enabled state of an object. A disabled object is neither shown nor updated.
 * \param object  The object to enable/disable
 * \return true if enabled, false otherwise
 */
bool H3_Object_GetEnabled(H3Handle object);

/**
 * \brief Gets an object's name
 * \param object  The object to get the name of
 * \return The object's name
 */
const char* H3_Object_GetName(H3Handle object);

/**
 * \brief Gets an object's render order. Objects are rendered in ascending render order, so the higher the render order is, the later (and the more above the others) it is drawn.
 * \param object  The object to get the render order of
 * \return The object's render order
 */
int32_t H3_Object_GetRenderOrder(H3Handle object);

/**
 * \brief Sets an object's render order. Objects are rendered in ascending render order, so the higher the render order is, the later (and the more above the others) it is drawn.
 * \param object      The object to set the render order to
 * \param renderOrder The render order to set to this object
 */
void H3_Object_SetRenderOrder(H3Handle object, int32_t renderOrder);

/**
 * \brief Gets an object's transform (Translation, rotation, scale)
 * \param object  The object to get the transform of
 * \return The object's transform as an opaque pointer
 */
SH3Transform* H3_Object_GetTransform(H3Handle object);

/**
 * \brief Resets an object's transform (no translation, no rotation, scale 1:1).
 * \param object      The object to reset the transform of
 */
void H3_Object_ResetTransform(H3Handle object);

/**
 * \brief Translates an object.
 * \param object The object to translate
 * \param x      The horizontal component of the translation
 * \param y      The vertical component of the translation
 */
void H3_Object_Translate(H3Handle object, float x, float y);

/**
 * \brief Rotates an object.
 * \param object The object to rotate
 * \param angle  The rotation angle
 */
void H3_Object_Rotate(H3Handle object, float angle);

/**
 * \brief Scales an object.
 * \param object The object to scale
 * \param x      The horizontal component of the scale
 * \param y      The vertical component of the scale
 */
void H3_Object_Scale(H3Handle object, float s);

/**
 * \brief Sets an object's translation.
 * \param object The object to set the translation of
 * \param x      The horizontal component of the translation
 * \param y      The vertical component of the translation
 */
void H3_Object_SetTranslation(H3Handle object, float x, float y);

/**
 * \brief Sets an object's rotation.
 * \param object The object to set the rotation of
 * \param angle  The rotation angle
 */
void H3_Object_SetRotation(H3Handle object, float angle);

/**
 * \brief Sets an object's scale.
 * \param object The object to set the scale of
 * \param s      A uniform scale value
 */
void H3_Object_SetScale(H3Handle object, float s);

/**
 * \brief Sets an object's scale.
 * \param object The object to set the scale of
 * \param x      The horizontal component of the scale
 * \param y      The vertical component of the scale
 */
void H3_Object_ScaleNonUniform(H3Handle object, float x, float y);

/**
 * \brief Adds a component to the object.
 * \param object The object to add a component to
 * \param component The component to add.
 * \sa    SH3Component
 */
void H3_Object_AddComponent(H3Handle object, SH3Component component);

/**
 * \brief Removes a component from the object.
 * \param componentId The component TypeID to look for.
 * \return true if a component was removed, false otherwise
 */
bool H3_Object_RemoveComponent(H3Handle object, uint32_t componentId);

/**
 * \brief Gets an object's component.
 * \param object      The object to get a component from
 * \param componentId The component TypeID to look for.
 * \return A pointer to the specified component or NULL if the object does not have a component of this type ID
 */
SH3Component* H3_Object_GetComponent(H3Handle object, uint32_t componentId);

/**
 * \brief Gets whether an object has a component.
 * \param object      The object to look for a component into
 * \param componentId The component TypeID to look for.
 * \return True if the object has a component with this ID, false otherwise.
 */
bool H3_Object_HasComponent(H3Handle object, uint32_t componentId);

/**
 * \brief Enables physics for this component. Once called, this object will be affected by gravity and will receive OnCollisionEnter/OnCollisionLeave (and Trigger variants) events.
 * \param object      The object to enable physics for
 * \param desc        The collider for this object
 * \sa SH3ColliderDesc
 */
void H3_Object_EnablePhysics(H3Handle object, SH3ColliderDesc desc);

/**
 * \brief Enables physics for this component using multiple shapes to make a more complex collider. Once called, this object will be affected by gravity and will receive OnCollisionEnter/OnCollisionLeave (and Trigger variants) events.
 * \param object      The object to enable physics for
 * \param descList    An array of colliders for this object
 * \param numShapes   The number of colliders in the array
 * \sa SH3ColliderDesc
 */
void H3_Object_EnablePhysicsEx(H3Handle object, SH3ColliderDesc* descList, uint32_t numShapes);

/**
 * \brief Disables physics for this component. Once called, this object will no longer be affected by gravity and will no longer receive OnCollisionEnter/OnCollisionLeave (or Trigger variants) events.
 * \param object      The object to disable physics for
 * \sa SH3ColliderDesc
 */
void H3_Object_DisablePhysics(H3Handle object);

/**
 * \brief Retrieves an object's linear damping (how much it slows down). Only valid if this object has physics enabled.
 * \param object The object to get the linear damping from
 * \return The damping
 */
float H3_Object_GetLinearDamping(H3Handle object);

/**
 * \brief Sets an object's linear damping (how much it slows down). Only valid if this object has physics enabled.
 * \param object The object to set the linear damping of
 * \param v      the damping
 */
void H3_Object_SetLinearDamping(H3Handle object, float v);

/**
 * \brief Retrieves an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to get the linear velocity from
 * \param vx     Pointer to the horizontal component of the velocity
 * \param vy     Pointer to the vertical component of the velocity
 */
void H3_Object_GetVelocity(H3Handle object, float* vx, float* vy);

/**
 * \brief Sets an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to set the linear velocity to
 * \param vx     The horizontal component of the velocity
 * \param vy     The vertical component of the velocity
 */
void H3_Object_SetVelocity(H3Handle object, float vx, float vy);

/**
 * \brief Add to an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to add the linear velocity to
 * \param vx     The horizontal component of the velocity to add
 * \param vy     The vertical component of the velocity to add
 */
void H3_Object_AddVelocity(H3Handle object, float vx, float vy);

/**
 * \brief Retrieves an object's angular damping (how much its rotation slows down). Only valid if this object has physics enabled.
 * \param object The object to get the linear damping from
 * \return The damping
 */
float H3_Object_GetAngularDamping(H3Handle object);

/**
 * \brief Sets an object's angular damping (how much its rotation slows down). Only valid if this object has physics enabled.
 * \param object The object to set the linear damping of
 * \param v      the damping
 */
void H3_Object_SetAngularDamping(H3Handle object, float v);

/**
 * \brief Retrieves an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to get the linear velocity from
 * \return The angular velocity
 */
float H3_Object_GetAngularVelocity(H3Handle object);

/**
 * \brief Sets an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to set the linear velocity to
 * \param vx     The velocity
 */
void H3_Object_SetAngularVelocity(H3Handle object, float v);

/**
 * \brief Add to an object's linear velocity. Only valid if this object has physics enabled.
 * \param object The object to add the linear velocity to
 * \param v      The horizontal component of the velocity to add
 */
void H3_Object_AddAngularVelocity(H3Handle object, float v);

// ============================================================================

/**
 * \brief Creates a joint (a constraint used to hold two or more bodies together)
 * \param scene The scene to create the joint into
 * \param joint The joint descriptor
 * \return The newly created joint
 */
H3Handle H3_Joint_Create(H3Handle scene, SH3JointDesc jointDesc);

/**
 * \brief Destroys a joint
 * \param joint The joint to destroy
 */
void     H3_Joint_Destroy(H3Handle joint);

// ============================================================================

/**
 * \brief Gets a position from a SH3Transform pointer
 * \param transform Pointer to the transform
 * \param x         Pointer to write the horizontal position to
 * \param y         Pointer to write the vertical position to
 */
void H3_Transform_GetPosition(SH3Transform* transform, float* x, float* y);

// ============================================================================

/**
 * \brief Loads a texture
 * \param path   The path to the image file
 * \param width  A pointer to write the image's width to
 * \param height A pointer to write the image's height to
 * \return A handle to the loaded texture, or a NULL handle if loading failed (in which case width and height are not written to)
 */
H3Handle H3_Texture_Load(const char* path, uint32_t* width, uint32_t* height);

/**
 * \brief Destroys a texture
 * \param handle The texture to destroy
 */
void H3_Texture_Destroy(H3Handle handle);

/**
 * \brief Draws a texture to screen
 * \param h3      Main H3 handle
 * \param px      Horizontal position in world units
 * \param py      Vertical position in world units
 * \param texture The texture to draw
 * \param anchor  The anchor to use as a reference. px and py will define the position of the anchor
 */
void H3_Texture_Draw(H3Handle h3, float px, float py, H3Handle texture, EH3Anchor anchor);

/**
 * \brief Draws a texture to screen
 * \param h3        Main H3 handle
 * \param transform Pointer to the transform to use
 * \param texture   The texture to draw
 * \param anchor    The anchor to use as a reference. transform will define the position of the anchor
 */
void H3_Texture_DrawEx(H3Handle h3, SH3Transform* transform, H3Handle texture, EH3Anchor anchor);

/**
 * \brief Draws part of a texture to screen
 * \param h3        Main H3 handle
 * \param transform Pointer to the transform to use
 * \param texture   The texture to draw
 * \param x0        Upper-left corner to use inside the texture
 * \param y0        Upper-left corner to use inside the texture
 * \param x1        Lower-right corner to use inside the texture
 * \param y1        Lower-right corner to use inside the texture
 * \param anchor    The anchor to use as a reference. transform will define the position of the anchor
 */
void H3_Texture_DrawRectEx(H3Handle h3, SH3Transform* transform, H3Handle texture, float x0, float y0, float x1, float y1, EH3Anchor anchor);

// ============================================================================

/**
 * \brief Loads a map
 * \param path The path to the TMX map file
 * \return Handle to the loaded map
 * \remark As of November 2021, "Collection of images" maps are not supported.
 */
H3Handle H3_Map_Load(const char* path);

/**
 * \brief Destroys a map
 * \param handle The map to destroy
 */
void H3_Map_Destroy(H3Handle* handle);

/**
 * \brief Register all shapes in a specified object layer as static colliders in a scene
 * \param scene     The scene in which to add the colliders to
 * \param map       The map to get the colliders from
 * \param layerName The name of the map layer to get the colliders from
 * \remark As of November 2021, these shapes do not support rotation and supported shapes are boxes and circles (aka. ellipses with width = height)
 */
void H3_Map_RegisterObjectLayerForPhysicsInScene(H3Handle scene, H3Handle map, const char* layerName);

/**
 * \brief Gets the ID of a layer from its name
 * \param map       The map to get the layer ID from
 * \param layerName The name of the map layer to get the ID of
 * \return The ID of the specified layer, or (size_t)-1 if not found
 */
size_t H3_Map_GetLayerFromName(H3Handle map, const char* layerName);

/**
 * \brief Draws a full map, using (0, 0) as origin
 * \param h3  The main H3 handle
 * \param map The map to draw
 */
void H3_Map_Draw(H3Handle h3, H3Handle map);

/**
 * \brief Draws a single map layer, using (0, 0) as origin
 * \param h3      The main H3 handle
 * \param map     The map to draw a layer from
 * \param layerId The layer ID to draw
 */
void H3_Map_DrawLayer(H3Handle h3, H3Handle map, size_t layerId);

// ============================================================================

/**
 * \brief Loads a sound
 * \param path The path to the sound file to load
 * \return Handle to the loaded sound
 * \remark As of November 2021, wav, flac and ogg (but not mp3) files are supported.
 */
H3Handle H3_Sound_Load(const char* path);

/**
 * \brief Destroys a sound
 * \param handle The sound to destroy
 */
void H3_Sound_Destroy(H3Handle handle);

/**
 * \brief Plays a sound (without spatialization)
 * \param snd    The sound to play
 * \param volume The volume to play the sound at, between 0.0f and 1.0f
 * \param loop   True to play the sound as an infinite loop, false to only play once
 */
void H3_Sound_Play(H3Handle snd, float volume, bool loop);

/**
 * \brief Plays a sound (spatialized)
 * \param snd    The sound to play
 * \param volume The volume to play the sound at, between 0.0f and 1.0f
 * \param px     The position to play the sound from (horizontal component)
 * \param py     The position to play the sound from (vertical component)
 * \param loop   True to play the sound as an infinite loop, false to only play once
 * \remark Only mono sounds can be spatialized. Stereo sounds will be played non-spatialized.
 */
void H3_Sound_PlaySpatialized(H3Handle snd, float volume, float px, float py, bool loop);

/**
 * \brief Stops a sound
 * \param snd The sound to stop
 */
void H3_Sound_Stop(H3Handle snd);

/**
 * \brief Sets a spatialized sound position
 * \param snd The sound to set the position of
 * \param px  The position to set to the sound (horizontal component)
 * \param py  The position to set to the sound (vertical component)
 */
void H3_Sound_SetPosition(H3Handle snd, float px, float py);

/**
 * \brief Sets the sound listener's location. All spatialized sounds will use this listener as a reference.
 * \param px    The position to set to the listener
 * \param py    The position to set to the listener
 * \param angle The rotation angle to set to the listener
 */
void H3_Listener_SetLocation(float px, float py, float angle);

/**
 * \brief Sets the sound listener's position. All spatialized sounds will use this listener as a reference.
 * \param px    The position to set to the listener
 * \param py    The position to set to the listener
 */
void H3_Listener_SetPosition(float px, float py);

/**
 * \brief Sets the sound listener's rotation angle. All spatialized sounds will use this listener as a reference.
 * \param angle The rotation angle to set to the listener
 */
void H3_Listener_SetRotation(float angle);

// ============================================================================

/**
 * \brief Loads a music
 * \param path The path to the music file to load
 * \return Handle to the loaded music
 * \remark As of November 2021, wav, flac and ogg (but not mp3) files are supported. Moreover, while sounds are entirely loaded to memory, musics are streamed for disk, so this is preferable for long-duration audio files.
 */
H3Handle H3_Music_Load(const char* path);

/**
 * \brief Destroys a music
 * \param handle The music to destroy
 */
void H3_Music_Destroy(H3Handle handle);

/**
 * \brief Plays a music
 * \param music  The music to play
 * \param volume The volume to play the music at, between 0.0f and 1.0f
 * \param loop   True to play the music as an infinite loop, false to only play once
 */
void H3_Music_Play(H3Handle music, float volume, bool loop);

/**
 * \brief Stops a music
 * \param music The music to stop playing
 */
void H3_Music_Stop(H3Handle music);

// ============================================================================

/**
 * \brief Loads a font
 * \param path The path to the font file to load
 * \return Handle to the loaded font
 * \remark As of November 2021, only TrueType (.ttf) fonts are supported.
 */
H3Handle H3_Font_Load(const char* path);

/**
 * \brief Destroys a font
 * \param handle The font to destroy
 */
void H3_Font_Destroy(H3Handle handle);

/**
 * \brief Draws a text to the window using the specified properties
 * \param h3         The main H3 handle
 * \param properties The text properties to use to draw this text
 * \param transform  The transform to use as a reference to draw this text
 * \param format     The text to draw, printf-style
 */
void H3_Font_Printf(H3Handle h3, SH3TextProperties properties, SH3Transform* transform, const char* format, ...);

// ============================================================================

/**
 * \brief Gets the current view properties
 * \param h3  The main H3 handle
 * \param x   pointer to write the view center's X coordinate to
 * \param y   pointer to write the view center's Y coordinate to
 * \param w   pointer to write the view's width to
 * \param h   pointer to write the view's height to
 * \param vpw pointer to write the window's width (in pixels) to
 * \param vph pointer to write the windows's height (in pixels) to
 */
void H3_GetView(H3Handle h3, float* x, float* y, float* w, float* h, float* vpw, float* vph);

/**
 * \brief Sets the current view properties
 * \param h3        The main H3 handle
 * \param transform The transform to use as a reference position
 * \param w         The view's width to
 * \param h         The view's height to
 */
void H3_SetView(H3Handle h3, SH3Transform* transform, float w, float h);

/**
 * \brief Sets the current view properties
 * \param h3 The main H3 handle
 * \param x  The view center's X coordinate to
 * \param y  The view center's Y coordinate to
 * \param w  The view's width to
 * \param h  The view's height to
 */
void H3_SetView2(H3Handle h3, float x, float y, float w, float h);

/**
 * \brief Processes a full frame (init, preupdate, physics step, update, draw)
 * \param h3    The main H3 handle
 * \param scene The scene to draw this frame
 * \return true if the window hasn't been closed (and thus we should keep going), false otherwise
 */
bool H3_DoFrame(H3Handle h3, H3Handle scene);

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

#define H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RW_EX(C, T, N) \
	T C##_Get##N##Ex(H3Handle object); \
	void C##_Set##N##Ex(H3Handle object, T value);

#define H3_DECLARE_COMPONENT_PROPERTY_ACCESSORS_RO_EX(C, T, N) \
	T C##_Get##N##Ex(H3Handle object);

#define H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW_EX(C, ID, T, N) \
	T C##_Get##N##Ex(H3Handle object) { return ((C##_Properties*)(H3_Object_GetComponent(object, ID)->properties))->N; } \
	void C##_Set##N##Ex(H3Handle object, T value) { ((C##_Properties*)(H3_Object_GetComponent(object, ID)->properties))->N = value; }

#define H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO_EX(C, ID, T, N) \
	T C##_Get##N##Ex(H3Handle object) { return ((C##_Properties*)(H3_Object_GetComponent(object, ID)->properties))->N; }

#endif /* _H3_H_ */
