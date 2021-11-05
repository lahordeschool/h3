#include <h3.h>
#include <h3/internal.h>
#include <h3/tmxlayer.h>

#include <components/rectanglecollidercomponent.h>

#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>

#include <box2d/box2d.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")

#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "openal32.lib")
#pragma comment(lib, "flac.lib")
#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbis.lib")
#pragma comment(lib, "vorbisfile.lib")

#ifndef NDEBUG
#pragma comment(lib, "sfml-graphics-s-d.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-window-s-d.lib")
#pragma comment(lib, "sfml-audio-s-d.lib")
#pragma comment(lib, "box2d-s-d.lib")
#else // NDEBUG
#pragma comment(lib, "sfml-graphics-s.lib")
#pragma comment(lib, "sfml-system-s.lib")
#pragma comment(lib, "sfml-window-s.lib")
#pragma comment(lib, "sfml-audio-s.lib")
#pragma comment(lib, "box2d-s.lib")
#endif // NDEBUG

// #define H3_DEBUG_DRAW_PHYSICS

// ============================================================================

#ifdef H3_DEBUG_DRAW_PHYSICS
class DebugDrawPhysics
	: public b2Draw
{
	sf::RenderWindow* m_window;

public:
	DebugDrawPhysics(sf::RenderWindow* wnd)
		: m_window(wnd)
	{
		this->SetFlags(e_shapeBit | e_aabbBit);
	}

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		for (int32 i = 0; i < vertexCount; ++i)
		{
			int32 i1 = i;
			int32 i2 = (i + 1) % vertexCount;

			DrawSegment(vertices[i1], vertices[i2], color);
		}
	}

	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		DrawPolygon(vertices, vertexCount, color);
	}

	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		sf::CircleShape cs(radius * 100.0f);
		cs.setPosition(sf::Vector2f(center.x - radius, center.y - radius) * 100.0f);
		m_window->draw(cs);
	}

	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		DrawCircle(center, radius, color);
	}

	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		sf::Vertex line[2] = {
				sf::Vertex(sf::Vector2f(p1.x * 100.0f, p1.y * 100.0f), sf::Color(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, color.a * 255.0f)),
				sf::Vertex(sf::Vector2f(p2.x * 100.0f, p2.y * 100.0f), sf::Color(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, color.a * 255.0f))\
		};

		m_window->draw(line, 2, sf::Lines);
	}

	void DrawTransform(const b2Transform& xf)
	{
	}

    void DrawPoint(const b2Vec2& p, float size, const b2Color& color)
	{
	}
};
#endif

// ============================================================================

enum class EH3TypeInternal
{
	SceneObject = 1,
	TmxMap,
	Scene,
	Sound
};

struct SH3ObjectBase_
{
	EH3TypeInternal type;
};

struct SH3Scene_;

struct SH3SceneObject_
{
	EH3TypeInternal type = EH3TypeInternal::SceneObject;

	bool                               enabled;

	int32_t                            renderOrder;

	SH3Scene_*                         scene;
	SH3SceneObject_*                   parent;
	std::vector<SH3SceneObject_*>      children;

	std::vector<SH3Component>          components;

	sf::Transform                      transform;
	sf::Transform                      globalTransform;

	bool                               physicsEnabled;
	b2Body*                            physicsBody;
};

struct SH3Scene_
{
	EH3TypeInternal type = EH3TypeInternal::Scene;
	bool                               processing;

	std::unordered_map<std::string, SH3SceneObject_*> deferredAdditions;
	std::vector<std::pair<SH3SceneObject_*, bool>>    deferredDeletions;

	std::unordered_map<std::string, SH3SceneObject_*> allObjects;
	std::vector<SH3SceneObject_*>                     rootObjects;
	std::unordered_map<SH3SceneObject_*, std::string> objectNames;

	std::map<int32_t, std::vector<SH3SceneObject_*>>  objectsByRenderOrder;

	b2World* physicsWorld;
	bool     physicsLocksRotation;
};

struct SH3TmxMap_
{
	EH3TypeInternal type = EH3TypeInternal::TmxMap;

	tmx::Map map;
	std::vector<CH3TmxLayer*> layers;
};

struct SH3Sound_
{
	EH3TypeInternal type = EH3TypeInternal::Sound;

	sf::Sound       sound;
	sf::SoundBuffer soundBuffer;
};

// ============================================================================

class CollisionHandler
	: public b2ContactListener
{
public:
	virtual void BeginContact(b2Contact* contact)
	{
		SH3SceneObject_* oA = (SH3SceneObject_*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		SH3SceneObject_* oB = (SH3SceneObject_*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;
		
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		SH3Collision collision;
		collision.other = oB;
		collision.normal.x = worldManifold.normal.x;
		collision.normal.y = worldManifold.normal.y;
		collision.numContactPoints = 2;
		collision.contactPoints[0].x = worldManifold.points[0].x;
		collision.contactPoints[0].y = worldManifold.points[0].y;
		collision.contactPoints[1].x = worldManifold.points[1].x;
		collision.contactPoints[1].y = worldManifold.points[1].y;

		if (oA)
		{
			for (auto& c : oA->components)
			{
				if (contact->GetFixtureA()->IsSensor())
				{
					if (c.OnTriggerEnter)
						c.OnTriggerEnter(oA, collision);
				}
				else
				{
					if (c.OnCollisionEnter)
						c.OnCollisionEnter(oA, collision);
				}
			}
		}

		collision.other = oA;

		if (oB)
		{
			for (auto& c : oB->components)
			{
				if (contact->GetFixtureB()->IsSensor())
				{
					if (c.OnTriggerEnter)
						c.OnTriggerEnter(oB, collision);
				}
				else
				{
					if (c.OnCollisionEnter)
						c.OnCollisionEnter(oB, collision);
				}
			}
		}
	}

	virtual void EndContact(b2Contact* contact)
	{
		SH3SceneObject_* oA = (SH3SceneObject_*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
		SH3SceneObject_* oB = (SH3SceneObject_*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

		if (oA)
		{
			for (auto& c : oA->components)
			{
				if (contact->GetFixtureA()->IsSensor())
				{
					if (c.OnTriggerLeave)
						c.OnTriggerLeave(oA, oB);
				}
				else
				{
					if (c.OnCollisionLeave)
						c.OnCollisionLeave(oA, oB);
				}
			}
		}

		if (oB)
		{
			for (auto& c : oB->components)
			{
				if (contact->GetFixtureB()->IsSensor())
				{
					if (c.OnTriggerLeave)
						c.OnTriggerLeave(oB, oA);
				}
				else
				{
					if (c.OnCollisionLeave)
						c.OnCollisionLeave(oB, oA);
				}
			}
		}
	}
};

// ============================================================================

sf::Clock gClock;
sf::Clock gDeltaClock;

float gClockValue      = 0.0f;
float gDeltaClockValue = 0.0f;

bool      gCurrentKeyStates[EH3Key_Count]  = { false };
bool      gPreviousKeyStates[EH3Key_Count] = { false };

bool      gCurrentMouseButtonStates[EH3MouseButton_Count]  = { false };
bool      gPreviousMouseButtonStates[EH3MouseButton_Count] = { false };

// ============================================================================

void H3Internal_AddObjectToScene(SH3SceneObject_* object, const std::string& sObjName, SH3Scene_* scene);
void H3Internal_PreUpdateObjectRecursive(H3Handle h3, SH3SceneObject_* object, float t, float dt, sf::Transform* globalTransform, uint32_t componentTypeFlag);
void H3Internal_UpdateObjectRecursive(H3Handle h3, SH3SceneObject_* object, float t, float dt, sf::Transform* globalTransform, uint32_t componentTypeFlag);
void H3Internal_RenderObjectRecursive(H3Handle h3, SH3SceneObject_* object, sf::Transform* globalTransform);
void H3Internal_RenderObjectRecursiveWithRenderOrder(H3Handle h3, SH3SceneObject_* object, sf::Transform* globalTransform, int32_t renderOrder);
void H3Internal_DestroyObject(SH3SceneObject_* object, bool recursive);
bool H3Internal_SceneOwnsObject(H3Handle scene, H3Handle object);
bool H3Internal_ObjectOwnsObject(H3Handle needle, H3Handle haystack);

void H3Internal_DecomposeTransform(sf::Transform* transform, float& px, float& py, float& r, float& sx, float& sy);

b2Shape* H3Internal_MakePhysicsShape(const SH3ColliderDesc& desc);
b2Body*  H3Internal_CreateAndAddPhysicsBody(b2World* world, float px, float py, SH3ColliderDesc* descList, uint32_t numShapes, bool rotationLocked, void* userData);

// ============================================================================

H3_CAPI H3Handle H3_Init(SH3InitParams params)
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow* window = new sf::RenderWindow(
		sf::VideoMode(params.width, params.height),
		params.windowTitle,
		params.fullscreen
			? sf::Style::Fullscreen
			: sf::Style::Default,
		settings
	);

	if (window)
	{
		window->setVerticalSyncEnabled(true);

		ImGui::SFML::Init(*window);
		ImGui::SFML::Update(*window, gDeltaClock.restart());
	}

	return window;
}

H3_CAPI void H3_Terminate(H3Handle* h3)
{
	H3_ASSERT(h3, "h3 must not be NULL");
	H3_ASSERT(*h3, "Object pointed by h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)(*h3);

	ImGui::SFML::Shutdown();

 	delete window;
	*h3 = nullptr;
}

H3_CAPI float H3_GetTime()
{
	return gClockValue;
}

H3_CAPI float H3_GetDeltaTime()
{
	return gDeltaClockValue;
}

H3_CAPI bool H3_Input_IsKeyDown(EH3Key k)
{
	return gCurrentKeyStates[k];
}

H3_CAPI bool H3_Input_IsKeyPressed(EH3Key k)
{
	return (gCurrentKeyStates[k] && !gPreviousKeyStates[k]);
}

H3_CAPI void H3_Input_GetMousePos(H3Handle h3, int* x, int* y)
{
	H3_ASSERT(h3, "h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	sf::Vector2i pos = sf::Mouse::getPosition(*window);

	*x = pos.x;
	*y = pos.y;
}

H3_CAPI bool H3_Input_IsMouseBtnDown(EH3MouseButton btn)
{
	return gCurrentMouseButtonStates[btn];
}

H3_CAPI bool H3_Input_IsMouseBtnPressed(EH3MouseButton btn)
{
	return (gCurrentMouseButtonStates[btn] && !gPreviousMouseButtonStates[btn]);
}

H3_CAPI H3Handle H3_Scene_Create(H3Handle h3, bool physicsLocksRotation)
{
	static CollisionHandler s_gCollisionHandler;

	SH3Scene_* scn = new SH3Scene_;
	scn->physicsWorld = new b2World(b2Vec2(0.0f, 0.0f));
	scn->physicsLocksRotation = physicsLocksRotation;
	scn->processing = false;

	scn->physicsWorld->SetAllowSleeping(true);
	scn->physicsWorld->SetContinuousPhysics(true);
	scn->physicsWorld->SetContactListener(&s_gCollisionHandler);

#ifdef H3_DEBUG_DRAW_PHYSICS
	scn->physicsWorld->SetDebugDraw(new DebugDrawPhysics(reinterpret_cast<sf::RenderWindow*>(h3)));
#endif /* H3_DEBUG_DRAW_PHYSICS */

	return scn;
}

H3_CAPI void H3_Scene_Destroy(H3Handle* scene)
{
	SH3Scene_* scn = (SH3Scene_*)(*scene);
	H3_ASSERT(scn->type == EH3TypeInternal::Scene, "Handle type mismatch");
	
	for (auto& [_, a] : scn->allObjects)
		for (auto& c : a->components)
			if (c.Terminate)
				c.Terminate(c.properties);

	delete scn->physicsWorld;

	delete scn;
	scene = nullptr;
}

H3_CAPI void H3_Scene_SetGravity(H3Handle scene, float x, float y)
{
	H3_ASSERT(scene, "scene cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");
	SH3Scene_* scn = (SH3Scene_*)scene;

	scn->physicsWorld->SetGravity(b2Vec2(x, y));
}

H3_CAPI H3Handle H3_Object_Create(H3Handle scene, const char* objName, H3Handle parent)
{
	std::string sObjName(objName);

	H3_ASSERT(scene, "scene cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");
	SH3Scene_* scn = (SH3Scene_*)scene;

	H3_ASSERT(!scn->allObjects.count(sObjName), "An object with this name already exists");

	SH3SceneObject_* obj = new SH3SceneObject_;
	obj->scene = scn;
	obj->parent = nullptr;
	obj->renderOrder = 0;
	obj->physicsBody = nullptr;
	obj->enabled = true;
	obj->physicsEnabled = false;

	obj->transform = sf::Transform::Identity;

	if (!scn->processing)
		H3Internal_AddObjectToScene(obj, sObjName, scn);
	else
		scn->deferredAdditions[sObjName] = obj;
	return obj;
}

H3_CAPI void H3_Object_Destroy(H3Handle* object, bool recursive)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(*object, "Object pointed by object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)(*object))->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)(*object);

	if (!sObject->scene->processing)
		H3Internal_DestroyObject(sObject, recursive);
	else
		sObject->scene->deferredDeletions.push_back(std::make_pair(sObject, recursive));
	
	*object = nullptr;
}

H3_CAPI H3Handle H3_Object_Get(H3Handle scene, const char* objName)
{
	H3_ASSERT(scene, "scene must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");
	std::string sObjName(objName);
	H3_ASSERT(!sObjName.empty(), "Object name must not be empty");

	SH3Scene_* scn = (SH3Scene_*)scene;

	if (scn->allObjects.count(sObjName))
		return scn->allObjects[sObjName];

	return nullptr;
}

H3_CAPI void H3_Object_SetEnabled(H3Handle object, bool enabled)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	obj->enabled = enabled;

	if (!enabled)
	{
		if (obj->physicsBody)
		{
			obj->physicsEnabled = obj->physicsBody->IsEnabled();
			if (obj->physicsBody->IsEnabled())
				H3_Object_DisablePhysics(object);
		}
	}
	else
	{
		if (obj->physicsEnabled)
		{
			H3_Object_EnablePhysicsEx(object, nullptr, 0);
		}
	}
}

H3_CAPI bool H3_Object_GetEnabled(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	return obj->enabled;
}

H3_CAPI const char* H3_Object_GetName(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	return obj->scene->objectNames[obj].c_str();
}

H3_CAPI int32_t H3_Object_GetRenderOrder(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");

	return ((SH3SceneObject_*)object)->renderOrder;
}

H3_CAPI void H3_Object_SetRenderOrder(H3Handle object, int32_t renderOrder)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	if (obj->renderOrder != renderOrder)
	{
		auto& renderList = obj->scene->objectsByRenderOrder[obj->renderOrder];
		auto it = std::find(renderList.begin(), renderList.end(), obj);
		renderList.erase(it);
		obj->renderOrder = renderOrder;
		obj->scene->objectsByRenderOrder[renderOrder].push_back(obj);
	}
}

H3_CAPI SH3Transform* H3_Object_GetTransform(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	return &obj->transform;
}

H3_CAPI void H3_Object_ResetTransform(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	if (obj->physicsBody)
		obj->physicsBody->SetTransform(b2Vec2_zero, 0.0f);
	else
		obj->transform = sf::Transform::Identity;
}

H3_CAPI void H3_Object_Translate(H3Handle object, float x, float y)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	if (obj->physicsBody)
		obj->physicsBody->SetTransform(obj->physicsBody->GetPosition() + b2Vec2(x * 0.01f, y * 0.01f), obj->physicsBody->GetAngle());
	else
		obj->transform.translate(sf::Vector2f(x, y));
}

H3_CAPI void H3_Object_Rotate(H3Handle object, float angle)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	if (obj->physicsBody)
		obj->physicsBody->SetTransform(obj->physicsBody->GetPosition(), obj->physicsBody->GetAngle() + angle * H3_DEG2RAD);
	else
		obj->transform.rotate(angle);
}

H3_CAPI void H3_Object_Scale(H3Handle object, float s)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	H3_ASSERT(!obj->physicsBody, "This object has physics enabled. Expect strange behavior.");

	obj->transform.scale(sf::Vector2f(s, s));
}

H3_CAPI void H3_Object_SetTranslation(H3Handle object, float x, float y)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	float px, py, rot, sx, sy;
	H3Internal_DecomposeTransform(&obj->transform, px, py, rot, sx, sy);
	
	H3_Object_ResetTransform(object);
	H3_Object_Translate(object, x, y);
	H3_Object_Rotate(object, rot);
	//H3_Object_Scale(object, sx);
}

H3_CAPI void H3_Object_SetRotation(H3Handle object, float angle)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	float px, py, rot, sx, sy;
	H3Internal_DecomposeTransform(&obj->transform, px, py, rot, sx, sy);

	H3_Object_ResetTransform(object);
	H3_Object_Translate(object, px, py);
	H3_Object_Rotate(object, angle);
	//H3_Object_Scale(object, sx);
}

H3_CAPI void H3_Object_SetScale(H3Handle object, float s)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	float px, py, rot, sx, sy;
	H3Internal_DecomposeTransform(&obj->transform, px, py, rot, sx, sy);

	H3_Object_ResetTransform(object);
	H3_Object_Translate(object, px, py);
	H3_Object_Rotate(object, rot);
	//H3_Object_Scale(object, s);
}

H3_CAPI void H3_Object_ScaleNonUniform(H3Handle object, float x, float y)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* obj = ((SH3SceneObject_*)object);

	H3_ASSERT(!obj->physicsBody, "This object has physics enabled. Expect strange behavior.");

	obj->transform.scale(sf::Vector2f(x, y));
}

H3_CAPI void H3_Object_AddComponent(H3Handle object, SH3Component component)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	bool alreadyExists = false;

	for (auto& c : sObject->components)
	{
		if (c.componentType == component.componentType)
		{
			alreadyExists = true;
			break;
		}
	}

	if (alreadyExists)
		H3_ASSERT(false, "This object already has a component of this type.");
	else
		sObject->components.push_back(component);
 }

H3_CAPI SH3Component* H3_Object_GetComponent(H3Handle object, uint32_t componentId)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	for (auto& c : sObject->components)
		if (c.componentType == componentId)
			return &c;

	return nullptr;
}

H3_CAPI bool H3_Object_HasComponent(H3Handle object, uint32_t componentId)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	for (auto& c : sObject->components)
		if (c.componentType == componentId)
			return true;

	return false;
}

H3_CAPI void H3_Object_EnablePhysics(H3Handle object, SH3ColliderDesc desc)
{
	H3_Object_EnablePhysicsEx(object, &desc, 1);
}

H3_CAPI void H3_Object_EnablePhysicsEx(H3Handle object, SH3ColliderDesc* descList, uint32_t numShapes)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	H3_ASSERT(!sObject->parent, "Cannot enable physics on a non-root object");

	if (sObject->physicsBody)
		sObject->physicsBody->SetEnabled(true);
	else
		sObject->physicsBody = H3Internal_CreateAndAddPhysicsBody(sObject->scene->physicsWorld, 0.0f, 0.0f, descList, numShapes, sObject->scene->physicsLocksRotation, sObject);

	sObject->physicsEnabled = true;
}

H3_CAPI void H3_Object_DisablePhysics(H3Handle object)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	H3_ASSERT(sObject->physicsBody, "This object does not have physics enabled");
	sObject->physicsBody->SetEnabled(false);
}

H3_CAPI void H3_Object_GetVelocity(H3Handle object, float* vx, float* vy)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	H3_ASSERT(sObject->physicsBody, "This object does not have physics enabled");

	b2Vec2 v = sObject->physicsBody->GetLinearVelocity();
	*vx = v.x * 100.0f;
	*vy = v.y * 100.0f;
}

H3_CAPI void H3_Object_SetVelocity(H3Handle object, float vx, float vy)
{
	H3_ASSERT(object, "object must not be NULL");
	H3_ASSERT(((SH3ObjectBase_*)object)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");
	SH3SceneObject_* sObject = (SH3SceneObject_*)object;

	H3_ASSERT(sObject->physicsBody, "This object does not have physics enabled");
	sObject->physicsBody->SetLinearVelocity(b2Vec2(vx * 0.01f, vy * 0.01f));
}

H3_CAPI void H3_Object_AddVelocity(H3Handle object, float vx, float vy)
{
	float bx, by;
	H3_Object_GetVelocity(object, &bx, &by);
	H3_Object_SetVelocity(object, bx + vx, by + vy);
}

H3_CAPI void H3_Transform_GetPosition(SH3Transform* transform, float* x, float* y)
{
	H3_ASSERT_CONSOLE(transform, "transform must not be NULL");
	sf::Vector2f p = reinterpret_cast<sf::Transform*>(transform)->transformPoint(sf::Vector2f(0.0f, 0.0f));
	*x = p.x;
	*y = p.y;
}

H3_CAPI H3Handle H3_Texture_Load(const char* path, uint32_t* width, uint32_t* height)
{
	sf::Texture* tex = new sf::Texture();
	tex->loadFromFile(std::string(path));

	auto size = tex->getSize();
	*width  = size.x;
	*height = size.y;

	return tex;
}

H3_CAPI void H3_Texture_Destroy(H3Handle* handle)
{
	H3_ASSERT_CONSOLE(handle, "handle must not be NULL");
	H3_ASSERT_CONSOLE(*handle, "object pointed by handle must not be NULL");

	sf::Texture* tex = (sf::Texture*)(*handle);
	delete tex;
	*handle = nullptr;
}

H3_CAPI void H3_Texture_Draw(H3Handle h3, float px, float py, H3Handle texture, EH3Anchor anchor)
{
	H3_ASSERT(h3,      "h3 must not be NULL");
	H3_ASSERT(texture, "texture must not be NULL");

	sf::Texture* tex     = (sf::Texture*)texture;
	sf::Vector2f texSize = sf::Vector2f(float(tex->getSize().x), float(tex->getSize().y));

	sf::Vector2f fvAnchor(0.0f, 0.0f);

	switch (((uint8_t)anchor) | 0x0f)
	{
	case 2: fvAnchor.x = texSize.x * 0.5f; break;
	case 4: fvAnchor.x = texSize.x;        break;
	default:
		break;
	}

	switch (((uint8_t)anchor) | 0xf0)
	{
	case 2: fvAnchor.y = texSize.y * 0.5f; break;
	case 4: fvAnchor.y = texSize.y;        break;
	default:
		break;
	}

	sf::RectangleShape r(texSize);

	r.setOrigin(fvAnchor);
	r.setTexture(tex);
	r.setPosition(sf::Vector2f(px, py));
	((sf::RenderWindow*)h3)->draw(r);
}

H3_CAPI void H3_Texture_DrawEx(H3Handle h3, SH3Transform* transform, H3Handle texture, EH3Anchor anchor)
{
	H3_Texture_DrawRectEx(h3, transform, texture, 0.0f, 0.0f, 1.0f, 1.0f, anchor);
}

void H3_Texture_DrawRectEx(H3Handle h3, SH3Transform* transform, H3Handle texture, float x0, float y0, float x1, float y1, EH3Anchor anchor)
{
	H3_ASSERT(h3, "h3 must not be NULL");
	H3_ASSERT(texture, "texture must not be NULL");

	sf::Transform* xform = reinterpret_cast<sf::Transform*>(transform);
	float px, py, rot, sx, sy;
	H3Internal_DecomposeTransform(xform, px, py, rot, sx, sy);

	sf::Texture* tex = (sf::Texture*)texture;
	sf::Vector2f texSize = sf::Vector2f(float(tex->getSize().x), float(tex->getSize().y));

	sf::Vector2f fvAnchor(0.0f, 0.0f);

	sf::IntRect texRect(
		int(texSize.x * x0), int(texSize.y * y0),
		int(texSize.x * (x1 - x0)), int(texSize.y * (y1 - y0))
	);

	switch (anchor & 0x0f)
	{
	case 2: fvAnchor.x = float(texRect.width) * 0.5f; break;
	case 4: fvAnchor.x = float(texRect.width);        break;
	default:
		break;
	}

	switch ((anchor & 0xf0) >> 4)
	{
	case 2: fvAnchor.y = float(texRect.height) * 0.5f; break;
	case 4: fvAnchor.y = float(texRect.height);        break;
	default:
		break;
	}

	sf::RectangleShape r(sf::Vector2f(texRect.width, texRect.height));
	r.setTextureRect(texRect);
	r.setOrigin(fvAnchor);
	r.setTexture(tex);
	r.setPosition(sf::Vector2f(px, py));
	r.setRotation(rot);
	r.setScale(sf::Vector2f(sx, sy));
	((sf::RenderWindow*)h3)->draw(r);
}

H3_CAPI H3Handle H3_Map_Load(const char* path)
{
	SH3TmxMap_* map = new SH3TmxMap_;
	map->map.load(std::string(path));

	for (size_t i = 0; i < map->map.getLayers().size(); ++i)
		map->layers.push_back(new CH3TmxLayer(map->map, i));

	return map;
}

H3_CAPI void H3_Map_Destroy(H3Handle* handle)
{
	H3_ASSERT_CONSOLE(handle, "handle must not be NULL");
	H3_ASSERT_CONSOLE(*handle, "object pointed by handle must not be NULL");

	H3_ASSERT(((SH3ObjectBase_*)(*handle))->type == EH3TypeInternal::TmxMap, "Handle type mismatch");
	SH3TmxMap_* tmxMap = (SH3TmxMap_*)(*handle);

	for (auto layer : tmxMap->layers)
		delete layer;
	
	delete tmxMap;
	*handle = nullptr;
}

H3_CAPI void H3_Map_RegisterObjectLayerForPhysicsInScene(H3Handle scene, H3Handle map, const char* layerName)
{

	H3_ASSERT(map, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)map)->type == EH3TypeInternal::TmxMap, "Handle type mismatch");


	H3_ASSERT(scene, "scene cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");
	SH3Scene_* scn = (SH3Scene_*)scene;

	SH3TmxMap_* tmxMap = (SH3TmxMap_*)map;
	std::string sLayerName(layerName);

	tmx::Layer* layerPtr = nullptr;

	for (auto& l : tmxMap->map.getLayers())
	{
		if (l->getName() == sLayerName)
		{
			layerPtr = l.get();
			break;
		}
	}

	if (layerPtr)
	{
		H3_ASSERT(layerPtr->getType() == tmx::Layer::Type::Object, "Trying to find collider in a non-Object layer, ignoring.");

		std::vector<SH3ColliderDesc> colliders;

		const auto& objectLayer = layerPtr->getLayerAs<tmx::ObjectGroup>();
		const auto& objects = objectLayer.getObjects();
		for (const auto& object : objects)
		{
			if (!object.visible())
				continue;

			SH3ColliderDesc newDesc;
			newDesc.offset.x = object.getPosition().x;
			newDesc.offset.y = object.getPosition().y;

			switch (object.getShape())
			{
			case tmx::Object::Shape::Ellipse:
				newDesc.shapeType = CST_Circle;
				newDesc.shapeData.circle.radius = fminf(object.getAABB().width, object.getAABB().height) * 0.5f;
				colliders.push_back(newDesc);
				break;

			case tmx::Object::Shape::Rectangle:
				newDesc.shapeType = CST_Box;
				newDesc.shapeData.box.width  = object.getAABB().width;
				newDesc.shapeData.box.height = object.getAABB().height;
				newDesc.offset.x += 0.5f * newDesc.shapeData.box.width;
				newDesc.offset.y += 0.5f * newDesc.shapeData.box.height;
				colliders.push_back(newDesc);
				break;

			default:
				break;
			}
		}

		if (colliders.size() > 0)
			H3Internal_CreateAndAddPhysicsBody(scn->physicsWorld, 0.0f, 0.0f, &colliders[0], colliders.size(), true, nullptr);
	}
}

H3_CAPI size_t H3_Map_GetLayerFromName(H3Handle map, const char* layerName)
{
	H3_ASSERT(map, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)map)->type == EH3TypeInternal::TmxMap, "Handle type mismatch");

	SH3TmxMap_* tmxMap = (SH3TmxMap_*)map;
	std::string sLayerName(layerName);

	for (size_t i = 0; i < tmxMap->layers.size(); ++i)
		if (tmxMap->map.getLayers()[i]->getName() == sLayerName)
			return i;

	return size_t(-1);
}

H3_CAPI void H3_Map_Draw(H3Handle h3, H3Handle map)
{
	H3_ASSERT(map, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)map)->type == EH3TypeInternal::TmxMap, "Handle type mismatch");

	SH3TmxMap_* tmxMap = (SH3TmxMap_*)map;

	for (size_t i = 0; i < tmxMap->layers.size(); ++i)
		H3_Map_DrawLayer(h3, map, i);
}

H3_CAPI void H3_Map_DrawLayer(H3Handle h3, H3Handle map, size_t layerId)
{
	H3_ASSERT_CONSOLE(h3, "h3 cannot be NULL");
	H3_ASSERT(map, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)map)->type == EH3TypeInternal::TmxMap, "Handle type mismatch");


	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	SH3TmxMap_* tmxMap = (SH3TmxMap_*)map;

	H3_ASSERT_CONSOLE(layerId < tmxMap->layers.size(), "LayerID out of range");
	window->draw(*(tmxMap->layers[layerId]));
}


H3_CAPI H3Handle H3_Sound_Load(const char* path)
{
	SH3Sound_* sound = new SH3Sound_;

	sf::SoundBuffer sb;
	sound->soundBuffer.loadFromFile(std::string(path));
	sound->sound.setBuffer(sound->soundBuffer);

	return sound;
}

H3_CAPI void H3_Sound_Destroy(H3Handle* snd)
{
	H3_ASSERT(snd, "snd cannot be NULL");
	H3_ASSERT(*snd, "object pointed by snd cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)(*snd))->type == EH3TypeInternal::Sound, "Handle type mismatch");

	SH3Sound_* sound = (SH3Sound_*)(*snd);

	delete sound;
	snd = nullptr;
}

H3_CAPI void H3_Sound_Play(H3Handle snd, float volume, bool loop)
{
	H3_ASSERT(snd, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)snd)->type == EH3TypeInternal::Sound, "Handle type mismatch");

	SH3Sound_* sound = (SH3Sound_*)snd;
	sound->sound.setVolume(volume * 100.0f);
	sound->sound.setLoop(loop);
	sound->sound.setRelativeToListener(false);
	sound->sound.play();
}

H3_CAPI void H3_Sound_PlaySpatialized(H3Handle snd, float volume, float px, float py, bool loop)
{
	H3_ASSERT(snd, "snd cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)snd)->type == EH3TypeInternal::Sound, "Handle type mismatch");

	SH3Sound_* sound = (SH3Sound_*)snd;
	sound->sound.setVolume(volume * 100.0f);
	sound->sound.setLoop(loop);
	sound->sound.setRelativeToListener(true);
	sound->sound.setPosition(sf::Vector3f(px, 0.0f, py));
	sound->sound.play();
}

H3_CAPI void H3_Sound_SetPosition(H3Handle snd, float px, float py)
{
	H3_ASSERT(snd, "snd cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)snd)->type == EH3TypeInternal::Sound, "Handle type mismatch");

	SH3Sound_* sound = (SH3Sound_*)snd;
	sound->sound.setPosition(sf::Vector3f(px, 0.0f, py));
}

H3_CAPI void H3_Sound_Stop(H3Handle snd)
{
	H3_ASSERT(snd, "map cannot be NULL");
	H3_ASSERT(((SH3ObjectBase_*)snd)->type == EH3TypeInternal::Sound, "Handle type mismatch");

	SH3Sound_* sound = (SH3Sound_*)snd;
	sound->sound.stop();
}

H3_CAPI void H3_Listener_SetLocation(float px, float py, float angle)
{
	H3_Listener_SetPosition(px, py);
	H3_Listener_SetRotation(angle);
}

H3_CAPI void H3_Listener_SetPosition(float px, float py)
{
	sf::Listener::setPosition(sf::Vector3f(px, 0.0f, py));
}

H3_CAPI void H3_Listener_SetRotation(float angle)
{
	float ca = cosf(angle);
	float sa = sinf(angle);

	sf::Listener::setDirection(sf::Vector3f(ca, 0.0f, sa));
	sf::Listener::setUpVector(sf::Vector3f(0.0f, 1.0f, 0.0f));
	sf::Listener::setGlobalVolume(1.0f);
}

H3_CAPI H3Handle H3_Music_Load(const char* path)
{
	sf::Music* music = new sf::Music();
	music->openFromFile(path);

	return music;
}

H3_CAPI void H3_Music_Destroy(H3Handle* music)
{
	H3_ASSERT(music, "music cannot be NULL");
	H3_ASSERT(*music, "object pointed by music cannot be NULL");

	H3Handle hnd = *music;

	H3_Music_Stop(hnd);

	sf::Music* msc = (sf::Music*)(*music);
	delete msc;
	*music = nullptr;
}

H3_CAPI void H3_Music_Play(H3Handle music, float volume, bool loop)
{
	H3_ASSERT(music, "music cannot be NULL");
	sf::Music* msc = (sf::Music*)music;

	msc->setVolume(volume * 100.0f);
	msc->setLoop(loop);
	msc->setRelativeToListener(false);
	msc->play();
}

H3_CAPI void H3_Music_Stop(H3Handle music)
{
	H3_ASSERT(music, "music cannot be NULL");
	sf::Music* msc = (sf::Music*)music;

	if (msc->getStatus() != sf::Music::Stopped)
		msc->stop();
}

H3_CAPI H3Handle H3_Font_Load(const char* path)
{
	sf::Font* font = new sf::Font;
	font->loadFromFile(std::string(path));

	return font;
}

H3_CAPI void H3_Font_Destroy(H3Handle* font)
{
	sf::Font* f = (sf::Font*)(*font);
	delete f;
	*font = nullptr;
}

H3_CAPI void H3_Font_Printf(H3Handle h3, SH3TextProperties properties, SH3Transform* transform, const char* format, ...)
{
	H3_ASSERT_CONSOLE(h3, "h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	char buffer[256] = { 0 };

	va_list argp;
	va_start(argp, format);
	vsnprintf_s(buffer, 256, format, argp);
	va_end(argp);

	sf::Text text;
	text.setString(std::string(buffer));
	text.setFont(*((sf::Font*)(properties.font)));
	text.setCharacterSize(properties.size);
	text.setFillColor(sf::Color(properties.fillColor.r, properties.fillColor.g, properties.fillColor.b, properties.fillColor.a));

	if (properties.hasOutline)
	{
		text.setOutlineThickness(properties.outlineThickness);
		text.setOutlineColor(sf::Color(properties.outlineColor.r, properties.outlineColor.g, properties.outlineColor.b, properties.outlineColor.a));
	}

	uint32_t style = 0;
	if (properties.isBold)
		style = style | sf::Text::Bold;
	if (properties.isUnderlined)
		style = style | sf::Text::Underlined;
	if (properties.isItalic)
		style = style | sf::Text::Italic;
	text.setStyle(style);

	float x, y, rotation, sx, sy;
	H3Internal_DecomposeTransform(reinterpret_cast<sf::Transform*>(transform), x, y, rotation, sx, sy);

	if (properties.isViewLocal)
	{
		sf::Vector2f viewCenter = window->getView().getCenter();
		sf::Vector2f viewExtent = window->getView().getSize();

		x = viewCenter.x - (viewExtent.x * 0.5f) + (x - viewCenter.x);
		y = viewCenter.y - (viewExtent.y * 0.5f) + (y - viewCenter.y);
	}

	text.setPosition(sf::Vector2f(x, y));
	text.setRotation(rotation);
	text.setScale(sf::Vector2f(sx, sy));

	sf::Vector2f fvAnchor(0.0f, 0.0f);

	sf::FloatRect bounds = text.getLocalBounds();

	switch (properties.anchor & 0x0f)
	{
	case 2: fvAnchor.x = float(bounds.width) * 0.5f; break;
	case 4: fvAnchor.x = float(bounds.width);        break;
	default:
		break;
	}

	switch ((properties.anchor & 0xf0) >> 4)
	{
	case 2: fvAnchor.y = float(bounds.height) * 0.5f; break;
	case 4: fvAnchor.y = float(bounds.height);        break;
	default:
		break;
	}
	text.setOrigin(fvAnchor);

	window->draw(text);
}

H3_CAPI void H3_GetView(H3Handle h3, float* x, float* y, float* w, float* h, float* vpw, float* vph)
{
	H3_ASSERT_CONSOLE(h3, "h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	auto view = window->getView();
	*x = view.getCenter().x;
	*y = view.getCenter().y;
	*w = view.getSize().x;
	*h = view.getSize().y;

	auto vp = window->getSize();
	*vpw = vp.x;
	*vph = vp.y;
}

H3_CAPI void H3_SetView(H3Handle h3, SH3Transform* transform, float w, float h)
{
	H3_ASSERT_CONSOLE(h3, "h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	float x, y, rotation, sx, sy;
	H3Internal_DecomposeTransform(reinterpret_cast<sf::Transform*>(transform), x, y, rotation, sx, sy);

	sf::View view;

	view.setCenter(sf::Vector2f(x, y));
	view.setSize(sf::Vector2f(w * sx, h * sy));
	view.setRotation(rotation);

	H3_Listener_SetPosition(x, y);

 	window->setView(view);
}

H3_CAPI void H3_SetView2(H3Handle h3, float x, float y, float w, float h)
{
	H3_ASSERT_CONSOLE(h3, "h3 must not be NULL");
	sf::RenderWindow* window = (sf::RenderWindow*)h3;

	sf::View view;

	view.setCenter(sf::Vector2f(x, y));
	view.setSize(sf::Vector2f(w, h));

	H3_Listener_SetPosition(x, y);

	window->setView(view);
}

H3_CAPI bool H3_DoFrame(H3Handle h3, H3Handle scene)
{
	H3_ASSERT_CONSOLE(h3, "h3 must not be NULL");
	H3_ASSERT(scene, "scene must not be NULL");

	auto t  = gClock.getElapsedTime();
	auto dt = gDeltaClock.restart();

	gClockValue      = t.asSeconds();
	gDeltaClockValue = dt.asSeconds();

	sf::Transform noTransform = sf::Transform::Identity;

	if (scene)
	{
		H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");

		SH3Scene_* scn = (SH3Scene_*)scene;
		scn->processing = true;

		for (auto [_, o] : scn->allObjects)
		{ // Init components in object at first use
			if (!o->enabled)
				continue;

			for (auto& c : o->components)
			{
				if (!c.isInitialized)
				{
					if (c.Init)
						c.Init(&o->transform, &c.properties);

					c.isInitialized = true;
				}
			}
		}
		
		{ // Update physics and transforms

			for (auto o : scn->rootObjects)
				H3Internal_PreUpdateObjectRecursive(h3, o, t.asSeconds(), dt.asSeconds(), &noTransform, 0);

			if (scn->physicsWorld)
				scn->physicsWorld->Step(dt.asSeconds(), 6, 2);

			std::function<void(SH3SceneObject_*, sf::Transform)> UpdateGlobalTransformRecursive;
			UpdateGlobalTransformRecursive = [&](SH3SceneObject_* o, sf::Transform parentTransform)
			{
				if (!o->enabled)
					return;

				if (o->physicsBody)
				{
					o->transform = sf::Transform::Identity;

					const b2Vec2& physPosition = o->physicsBody->GetPosition();
					o->transform.translate(sf::Vector2f(physPosition.x * 100.0f, physPosition.y * 100.0f)).rotate(o->physicsBody->GetAngle() * H3_RAD2DEG);
				}

				o->globalTransform = parentTransform * o->transform;

				for (auto c : o->children)
					UpdateGlobalTransformRecursive(c, o->globalTransform);
			};

			for (auto o : scn->rootObjects)
			{
				if (!o->enabled)
					continue;

				UpdateGlobalTransformRecursive(o, noTransform);
			}

			for (auto o : scn->rootObjects)
				H3Internal_UpdateObjectRecursive(h3, o, t.asSeconds(), dt.asSeconds(), &noTransform, 0);
		}

	}

	sf::RenderWindow* window = (sf::RenderWindow*)h3;
	window->clear();

	if (scene)
	{ // Render the entire scene
		H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");

		SH3Scene_* scn = (SH3Scene_*)scene;

		for (auto& k : scn->objectsByRenderOrder)
		{
			for (SH3SceneObject_* obj : k.second)
			{
				if (!obj->enabled)
					continue;

				bool recurseChainCancelsRender = false;

				SH3SceneObject_* parent = obj->parent;
				while (parent)
				{
					if (!parent->enabled)
					{
						recurseChainCancelsRender = true;
						break;
					}

					parent = parent->parent;
				}

				if (recurseChainCancelsRender)
					continue;

				for (auto& c : obj->components)
					if (c.Draw)
						c.Draw(h3, &obj->globalTransform, c.properties);
			}
		}

#ifdef H3_DEBUG_DRAW_PHYSICS
		scn->physicsWorld->DebugDraw();
#endif  /* H3_DEBUG_DRAW_PHYSICS */

		scn->processing = false;
	}

	H3Internal_HandleAssertPopup();

	{ // Update and render UI
		ImGui::SFML::Render(*window);
		ImGui::SFML::Update(*window, dt);
	}

	{ // Swap buffers
		window->display();
	}

	{ // Handle deferred object additions and deletions
		if (scene)
		{
			SH3Scene_* scn = (SH3Scene_*)scene;
			for (auto& [sObjName, obj] : scn->deferredAdditions)
				H3Internal_AddObjectToScene(obj, sObjName, scn);

			scn->deferredAdditions.clear();
		
			for (auto& [toDel, recursive] : scn->deferredDeletions)
				H3Internal_DestroyObject(toDel, recursive);

			scn->deferredDeletions.clear();
		}
	}
	
	{ // Poll inputs
		for (int k = 0; k < EH3Key_Count; ++k)
		{
			gPreviousKeyStates[k] = gCurrentKeyStates[k];
			gCurrentKeyStates[k] = ([k]() -> bool {
				switch (k)
				{
				case K_Left:      return sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
				case K_Right:     return sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
				case K_Down:      return sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
				case K_Up:        return sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
				case K_Space:     return sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
				case K_Enter:     return sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
				case K_Tab:       return sf::Keyboard::isKeyPressed(sf::Keyboard::Tab);
				case K_Backspace: return sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace);
				case K_Escape:    return sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
				case K_Shift:     return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
				case K_Ctrl:      return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
				case K_F2:        return sf::Keyboard::isKeyPressed(sf::Keyboard::F2);
				default:
					return false;
				}
			})();
		}

		for (int k = 0; k < EH3MouseButton_Count; ++k)
		{
			gPreviousMouseButtonStates[k] = gCurrentMouseButtonStates[k];
			gCurrentMouseButtonStates[k] = sf::Mouse::isButtonPressed(([k]() -> sf::Mouse::Button {
				switch (k)
				{
				case MB_Left:   return sf::Mouse::Left;
				case MB_Right:  return sf::Mouse::Right;
				case MB_Middle: return sf::Mouse::Middle;
				default:
					return sf::Mouse::ButtonCount;
				}
			})());
		}

		sf::Event e;
		while (window->pollEvent(e))
		{
			ImGui::SFML::ProcessEvent(e);

			switch (e.type)
			{
			case sf::Event::Closed:
				window->close();

			case sf::Event::Resized:
			{
				sf::Vector2f size = static_cast<sf::Vector2f>(window->getSize());
				if (size.x < 128.0f) size.x = 128.0f;
				if (size.y < 128.0f) size.y = 128.0f;

				window->setSize(static_cast<sf::Vector2u>(size));
				window->setView(sf::View(sf::FloatRect(0.f, 0.f, size.x, size.y)));
			}	break;

			default:
				break;
			}

			if (e.type == sf::Event::Closed)
				window->close();
		}
	}

	return window->isOpen();
}

// ============================================================================

void H3Internal_AddObjectToScene(SH3SceneObject_* object, const std::string& sObjName, SH3Scene_* scene)
{
	scene->allObjects[sObjName] = object;
	scene->objectNames[object] = sObjName;
	scene->objectsByRenderOrder[0].push_back(object);

	if (object->parent)
	{
		H3_ASSERT(H3Internal_SceneOwnsObject(scene, object->parent), "Handle type mismatch");

		SH3SceneObject_* oParent = (SH3SceneObject_*)object->parent;
		object->parent = oParent;
		oParent->children.push_back(object);
	}
	else
		scene->rootObjects.push_back(object);
}

void H3Internal_UpdateObjectRecursive(H3Handle h3, SH3SceneObject_* object, float t, float dt, sf::Transform* globalTransform, uint32_t componentTypeFlag)
{
	if (!object->enabled)
		return;

	sf::Transform xform = (*globalTransform) * object->transform;

	for (auto& c : object->components)
	{
		bool processUpdate = true;
		if (componentTypeFlag)
			processUpdate = (c.componentType & componentTypeFlag);

		if (c.Update && processUpdate)
			c.Update(h3, object, &xform, t, dt, c.properties);
	}

	for (auto c : object->children)
		H3Internal_UpdateObjectRecursive(h3, c, t, dt, &xform, componentTypeFlag);
}

void H3Internal_PreUpdateObjectRecursive(H3Handle h3, SH3SceneObject_* object, float t, float dt, sf::Transform* globalTransform, uint32_t componentTypeFlag)
{
	if (!object->enabled)
		return;

	sf::Transform xform = (*globalTransform) * object->transform;

	for (auto& c : object->components)
	{
		bool processUpdate = true;
		if (componentTypeFlag)
			processUpdate = (c.componentType & componentTypeFlag);

		if (c.PreUpdate && processUpdate)
			c.PreUpdate(h3, object, &xform, t, dt, c.properties);
	}

	for (auto c : object->children)
		H3Internal_PreUpdateObjectRecursive(h3, c, t, dt, &xform, componentTypeFlag);
}

void H3Internal_RenderObjectRecursive(H3Handle h3, SH3SceneObject_* object, sf::Transform* globalTransform)
{
	if (!object->enabled)
		return;

	sf::Transform xform = (*globalTransform) * object->transform;

	for (auto& c : object->components)
		if (c.Draw)
			c.Draw(h3, &xform, c.properties);

	for (auto c : object->children)
		H3Internal_RenderObjectRecursive(h3, c, &xform);
}

void H3Internal_RenderObjectRecursiveWithRenderOrder(H3Handle h3, SH3SceneObject_* object, sf::Transform* globalTransform, int32_t renderOrder)
{ // TODO: This is not optimized and computes way more transforms than necessary.
	if (!object->enabled)
		return;

	sf::Transform xform = (*globalTransform) * object->transform;

	for (auto& c : object->components)
		if ((c.Draw) && (object->renderOrder == renderOrder))
			c.Draw(h3, &xform, c.properties);

	for (auto c : object->children)
		H3Internal_RenderObjectRecursiveWithRenderOrder(h3, c, &xform, renderOrder);
}

bool H3Internal_SceneOwnsObject(H3Handle scene, H3Handle object)
{
	H3_ASSERT(((SH3ObjectBase_*)scene)->type == EH3TypeInternal::Scene, "Handle type mismatch");
	SH3Scene_* scn = (SH3Scene_*)scene;

	for (auto o : scn->rootObjects)
		if (H3Internal_ObjectOwnsObject(object, o))
			return true;

	return false;
}

bool H3Internal_ObjectOwnsObject(H3Handle needle, H3Handle haystack)
{
	H3_ASSERT(((SH3ObjectBase_*)needle)->type   == EH3TypeInternal::SceneObject, "Handle type mismatch");
	H3_ASSERT(((SH3ObjectBase_*)haystack)->type == EH3TypeInternal::SceneObject, "Handle type mismatch");

	if (needle == haystack)
		return true;

	SH3SceneObject_* oHaystack = (SH3SceneObject_*)haystack;

	for (auto o : oHaystack->children)
		if (H3Internal_ObjectOwnsObject(needle, o))
			return true;

	return false;
}

void H3Internal_DestroyObject(SH3SceneObject_* object, bool recursive)
{
	for (auto& c : object->components)
		if (c.Terminate)
			c.Terminate(c.properties);

	if (recursive)
	{ // Delete this object's children
		auto children = object->children;
		for (auto c : children)
			H3Internal_DestroyObject(c, true);
	}
	else
	{ // Reparent this object's children
		for (auto c : object->children)
		{
			c->parent = object->parent;

			if (c->parent)
				c->parent->children.push_back(c);
		}
	}

	if (object->parent)
	{ // Remove object from its parent
		auto it = std::find(object->parent->children.begin(), object->parent->children.end(), object);
		object->parent->children.erase(it);
	}
	else
	{ // Remove object from the scene's root list
		auto it = std::find(object->scene->rootObjects.begin(), object->scene->rootObjects.end(), object);
		object->scene->rootObjects.erase(it);
	}

	{ // Remove object from the global scene list
		object->scene->allObjects.erase(object->scene->objectNames[object]);
		object->scene->objectNames.erase(object);
	}

	{ // Remove object from its render ordered list
		auto& renderOrderedList = object->scene->objectsByRenderOrder[object->renderOrder];
		auto it = std::find(renderOrderedList.begin(), renderOrderedList.end(), object);
		renderOrderedList.erase(it);
	}

	if (object->physicsEnabled)
	{
		object->scene->physicsWorld->DestroyBody(object->physicsBody);
	}

	delete object;
}

void H3Internal_DecomposeTransform(sf::Transform* transform, float& px, float& py, float& r, float& sx, float& sy)
{
	if (!transform)
	{
		px = py = r = 0.0f;
		sx = sy = 1.0f;
		return;
	}

	sf::Vector2f origin = transform->transformPoint(sf::Vector2f(0.0f, 0.0f));
	px = origin.x; py = origin.y;

	sf::Vector2f unit   = transform->transformPoint(sf::Vector2f(1.0f, 0.0f));
	unit.x -= origin.x;
	unit.y -= origin.y;

	float dist = sqrtf(unit.x * unit.x + unit.y * unit.y);
	sx = sy = dist;

	r = atan2f(unit.y, unit.x) * H3_RAD2DEG;
	//if (r < 0.0f)
	//	r = 360.0f - r;
}

b2Shape* H3Internal_MakePhysicsShape(const SH3ColliderDesc& desc)
{
	b2Shape* shape = nullptr;

	switch (desc.shapeType)
	{
	case CST_Circle:
		shape = new b2CircleShape;
		static_cast<b2CircleShape*>(shape)->m_radius = desc.shapeData.circle.radius * 0.01f;
		static_cast<b2CircleShape*>(shape)->m_p.Set(desc.offset.x * 0.01f, desc.offset.y * 0.01f);
		break;

	case CST_Box:
		shape = new b2PolygonShape;
		static_cast<b2PolygonShape*>(shape)->SetAsBox(desc.shapeData.box.width * 0.5f * 0.01f, desc.shapeData.box.height * 0.5f * 0.01f);
		for (auto& v : static_cast<b2PolygonShape*>(shape)->m_vertices)
			v += b2Vec2(desc.offset.x * 0.01f, desc.offset.y * 0.01f);
		shape->m_radius = b2_polygonRadius;
		break;

	default:
		break;
	}

	return shape;
}

b2Body* H3Internal_CreateAndAddPhysicsBody(b2World* world, float px, float py, SH3ColliderDesc* descList, uint32_t numShapes, bool rotationLocked, void* userData)
{
	b2BodyDef bodyDef;
	bodyDef.type = [](EH3ColliderDynamicsType type) {
		switch (type)
		{
		case CDT_Dynamic: return b2_dynamicBody; // FIXME: This needs a separate b2FixtureDef to work properly.
		case CDT_Static:  return b2_staticBody;
		default:          return b2_kinematicBody;
		}
	}(descList[0].dynamicsType);

	bodyDef.position.Set(px, py);
	bodyDef.fixedRotation = rotationLocked ? (descList[0].dynamicsType == CDT_Dynamic) : false;
	bodyDef.userData.pointer = (uintptr_t)userData;
	b2Body* result = world->CreateBody(&bodyDef);

	for (uint32_t i = 0; i < numShapes; ++i)
	{
		SH3ColliderDesc& desc = descList[i];

		b2Shape* shape = H3Internal_MakePhysicsShape(desc);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = shape;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		fixtureDef.restitution = 0.0f;
		fixtureDef.filter.categoryBits = 0xffff;
		fixtureDef.filter.maskBits = 0xffff;

		result->CreateFixture(&fixtureDef);
		delete shape;
	}

	return result;
}
