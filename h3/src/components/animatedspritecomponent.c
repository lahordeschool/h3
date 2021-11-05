#include <components/animatedspritecomponent.h>

#include <stdlib.h>

typedef struct
{
	H3Handle         Texture;
	EH3Anchor        Anchor;
	uint32_t         NumFrames;
	float            FrameDuration;

	bool             FlipX;
	bool             FlipY;

	uint32_t         curFrame;
	float            curFrameTime;
	bool             loop;
	bool             isAnimOver;
} AnimatedSpriteComponent_Properties;

void AnimatedSpriteComponent_Terminate(void* properties)
{
	AnimatedSpriteComponent_Properties* props = (AnimatedSpriteComponent_Properties*)properties;

	if (props->Texture)
		H3_Texture_Destroy(&props->Texture);

	free(properties);
}

void AnimatedSpriteComponent_Update(H3Handle h3, H3Handle object, SH3Transform* transform, float t, float dt, void* properties)
{
	AnimatedSpriteComponent_Properties* props = (AnimatedSpriteComponent_Properties*)properties;

	props->curFrameTime += dt;

	while (props->curFrameTime >= props->FrameDuration)
	{
		props->curFrame++;
		
		if (props->loop)
			props->curFrame = props->curFrame % props->NumFrames;
		else
		{
			if (props->curFrame >= props->NumFrames)
			{
				props->curFrame = props->NumFrames - 1;
				props->isAnimOver = true;
			}
		}

		props->curFrameTime -= props->FrameDuration;
	}
}

void AnimatedSpriteComponent_Draw(H3Handle h3, SH3Transform* transform, void* properties)
{
	AnimatedSpriteComponent_Properties* props = (AnimatedSpriteComponent_Properties*)properties;
	
	float x0 = (float)(props->curFrame) / (float)(props->NumFrames);
	float x1 = (float)(props->curFrame + 1) / (float)(props->NumFrames);
	float y0 = 0.0f;
	float y1 = 1.0f;

	if (props->FlipX)
	{
		float tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if (props->FlipY)
	{
		y0 = 1.0f;
		y1 = 0.0f;
	}

	H3_Texture_DrawRectEx(
		h3,
		transform,
		props->Texture,
		x0, y0, x1, y1,
		props->Anchor
	);
}

void* AnimatedSpriteComponent_CreateProperties(const char* texturePath, uint8_t anchor, uint32_t numFrames, float frameDuration, bool loop)
{
	AnimatedSpriteComponent_Properties* properties = malloc(sizeof(AnimatedSpriteComponent_Properties));
	H3_ASSERT_CONSOLE(properties, "Failed to allocate properties");

	uint32_t w, h;
	properties->Texture = H3_Texture_Load(texturePath, &w, &h);
	properties->Anchor = (EH3Anchor)anchor;
	properties->NumFrames = numFrames;
	properties->FrameDuration = frameDuration;

	properties->curFrame = 0;
	properties->curFrameTime = 0.0f;
	properties->loop = loop;
	properties->isAnimOver = false;

	properties->FlipX = properties->FlipY = false;

	return properties;
}

void AnimatedSpriteComponent_ResetAnimation(H3Handle object)
{
	void* properties = H3_Object_GetComponent(object, ANIMATEDSPRITECOMPONENT_TYPEID)->properties;
	AnimatedSpriteComponent_Properties* props = (AnimatedSpriteComponent_Properties*)properties;

	props->curFrame = 0;
	props->curFrameTime = 0.0f;
	props->isAnimOver = false;
}

bool AnimatedSpriteComponent_IsAnimOver(H3Handle object)
{
	void* properties = H3_Object_GetComponent(object, ANIMATEDSPRITECOMPONENT_TYPEID)->properties;
	AnimatedSpriteComponent_Properties* props = (AnimatedSpriteComponent_Properties*)properties;

	return props->isAnimOver;
}

H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, H3Handle,  Texture);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, EH3Anchor, Anchor);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, uint32_t,  NumFrames);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RO(AnimatedSpriteComponent, float,     FrameDuration);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, bool,      FlipX);
H3_DEFINE_COMPONENT_PROPERTY_ACCESSORS_RW(AnimatedSpriteComponent, bool,      FlipY);
