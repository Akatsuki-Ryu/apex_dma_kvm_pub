#include "prediction.h"

#include <iostream>
#include <ostream>
#include <thread>
#include <chrono>
#include <array>

extern Memory apex_mem;

bool Entity::Observing(uint64_t entitylist)
{
	/*uint64_t index = *(uint64_t*)(buffer + OFFSET_OBSERVING_TARGET);
	index &= ENT_ENTRY_MASK;
	if (index > 0)
	{
		uint64_t centity2 = 0;
		apex_mem.Read<uint64_t>(entitylist + ((uint64_t)index << 5), centity2);
		return centity2;
	}
	return 0;*/
	return *(bool*)(buffer + OFFSET_OBSERVER_MODE);
}

//https://github.com/CasualX/apexbot/blob/master/src/state.cpp#L104
void get_class_name(uint64_t entity_ptr, char* out_str)
{
	uint64_t client_networkable_vtable;
	apex_mem.Read<uint64_t>(entity_ptr + 8 * 3, client_networkable_vtable);

	uint64_t get_client_class;
	apex_mem.Read<uint64_t>(client_networkable_vtable + 8 * 3, get_client_class);

	uint32_t disp;
	apex_mem.Read<uint32_t>(get_client_class + 3, disp);
	const uint64_t client_class_ptr = get_client_class + disp + 7;

	ClientClass client_class;
	apex_mem.Read<ClientClass>(client_class_ptr, client_class);

	apex_mem.ReadArray<char>(client_class.pNetworkName, out_str, 32);
}

int Entity::getTeamId()
{
	return *(int*)(buffer + OFFSET_TEAM);
}

int Entity::getHealth()
{
	return *(int*)(buffer + OFFSET_HEALTH);
}

int Entity::getShield()
{
	return *(int*)(buffer + OFFSET_SHIELD);
}

Vector Entity::getAbsVelocity()
{
	return *(Vector*)(buffer + OFFSET_ABS_VELOCITY);
}

Vector Entity::getPosition()
{
	return *(Vector*)(buffer + OFFSET_ORIGIN);
}

bool Entity::isPlayer()
{
	return *(uint64_t*)(buffer + OFFSET_NAME) == 125780153691248;
}

bool Entity::isDummy()
{
	char class_name[33] = {};
	get_class_name(ptr, class_name);

	return strncmp(class_name, "CAI_BaseNPC", 11) == 0;
}

bool Entity::isKnocked()
{
	return *(int*)(buffer + OFFSET_BLEED_OUT_STATE) > 0;
}

bool Entity::isAlive()
{
	return *(int*)(buffer + OFFSET_LIFE_STATE) == 0;
}

float Entity::lastVisTime()
{
  return *(float*)(buffer + OFFSET_VISIBLE_TIME);
}

Vector Entity::getBonePosition(int id)
{
	Vector position = getPosition();
	uintptr_t boneArray = *(uintptr_t*)(buffer + OFFSET_BONES);
	Vector bone = Vector();
	uint32_t boneloc = (id * 0x30);
	Bone bo = {};
	apex_mem.Read<Bone>(boneArray + boneloc, bo);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

//https://www.unknowncheats.me/forum/apex-legends/496984-getting-hitbox-positions-cstudiohdr-externally.html
//https://www.unknowncheats.me/forum/3499185-post1334.html
Vector Entity::getBonePositionByHitbox(int id)
{
	Vector origin = getPosition();

	//BoneByHitBox
	uint64_t Model = *(uint64_t*)(buffer + OFFSET_STUDIOHDR);

	//get studio hdr
	uint64_t StudioHdr;
	apex_mem.Read<uint64_t>(Model + 0x8, StudioHdr);

	//get hitbox array
	uint16_t HitboxCache;
	apex_mem.Read<uint16_t>(StudioHdr + 0x34, HitboxCache);
	uint64_t HitboxArray = StudioHdr + ((uint16_t)(HitboxCache & 0xFFFE) << (4 * (HitboxCache & 1)));

	uint16_t  IndexCache;
	apex_mem.Read<uint16_t>(HitboxArray + 0x4, IndexCache);
	int HitboxIndex = ((uint16_t)(IndexCache & 0xFFFE) << (4 * (IndexCache & 1)));

	uint16_t  Bone;
	apex_mem.Read<uint16_t>(HitboxIndex + HitboxArray + (id * 0x20), Bone);

	if (Bone < 0 || Bone > 255)
		return Vector();

	//hitpos
	uint64_t Bones = *(uint64_t*)(buffer + OFFSET_BONES);

	matrix3x4_t Matrix = {};
	apex_mem.Read<matrix3x4_t>(Bones + Bone * sizeof(matrix3x4_t), Matrix);

	return Vector(Matrix.m_flMatVal[0][3] + origin.x, Matrix.m_flMatVal[1][3] + origin.y, Matrix.m_flMatVal[2][3] + origin.z);
}

Vector Entity::GetSwayAngles()
{
	return *(Vector*)(buffer + OFFSET_BREATH_ANGLES);
}

Vector Entity::GetViewAngles()
{
	return *(Vector*)(buffer + OFFSET_VIEWANGLES);
}

Vector Entity::GetViewAnglesV()
{
	return *(Vector*)(buffer + OFFSET_VIEWANGLES);
}


float Entity::GetYaw()
{
	float yaw = 0;
	apex_mem.Read<float>(ptr + OFFSET_YAW, yaw);

	if (yaw < 0)
		yaw += 360;
	yaw += 90;
	if (yaw > 360)
		yaw -= 360;

	return yaw;
}


bool Entity::isGlowing()
{
	//return *(int*)(buffer + OFFSET_GLOW_ENABLE_GLOW_CONTEXT) == 1;
	return *(int*)(buffer + OFFSET_GLOW_ENABLE) == 7;
}

bool Entity::isZooming()
{
	return *(int*)(buffer + OFFSET_ZOOMING) == 1;
}


extern uint64_t g_Base;
extern int settingIndex;
extern int contextId;
extern std::array<float, 3> highlightParameter;
//custom glow color RGB
unsigned char outsidevalue = 125;
extern unsigned char insidevalue;
extern unsigned char outlinesize;

void Entity::enableGlow()
{
	/*
	apex_mem.Write<GlowMode>(ptr + GLOW_TYPE, { 101,102,96,90 });
	apex_mem.Write<GColor>(ptr + GLOW_COLOR, color);

	float currentEntityTime = 5000.f;
	apex_mem.Write<float>(ptr + GLOW_DISTANCE, 20000.f);
	apex_mem.Write<float>(ptr + GLOW_LIFE_TIME, currentEntityTime);

	currentEntityTime -= 1.f;

	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE_GLOW_CONTEXT, 1);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE, 1);
	apex_mem.Write<Fade>(ptr + GLOW_FADE, { 872415232, 872415232, currentEntityTime, currentEntityTime, currentEntityTime, currentEntityTime });
	*/

	//testing
	//apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, glowtype);
	//apex_mem.Write<int>(ptr + OFFSET_HIGHLIGHTSERVERACTIVESTATES + glowtype3, glowtype2);
	//apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE, 2);

	//apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 2);

	//72 purple block,71 black,70 orange outline ,66 white outline,65 cyan outline ,64 red outline, 63 white outline , 62 orange outline shaking
	// 61 orange outline, 60 white outline , 59 orange outline clear , 
	//48 green block
	//28 yellow block 
	//11 orange outline with a bit filling 
	//12 cyan outline 
	//15 green toxic shadow
	//20 white outline with a bit filling 
	//24 yellow outline 
	//25 yellow outline with a bit filing 
	//27 yellow block
	//6 red with a bit filling 
	//14 orange outline
	//29 purple with a bit filling
	//44 blue with filling 
	//46 white block bright 
	//48 green blcok
	//52 orange block maybe transparent



	/*
	int glowsetting = 58;
	int contextId = 1; // Same as glow enable
	std::array<unsigned char, 4> highlightFunctionBits = {
		125,   // InsideFunction
		125, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
		64,  // OutlineRadius: size * 255 / 8
		64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7)
	};
	std::array<float, 3> highlightParameter = { 0, 1, 0 };
	apex_mem.Write<unsigned char>(ptr + 0x298 + contextId, glowsetting);
	long highlightSettingsPtr;
	apex_mem.Read<long>(ptr + 0xb5f9620, highlightSettingsPtr);
	apex_mem.Write<int>(ptr + 0x294, 1); //has to be set 1 for glow to even show up at all?
	apex_mem.Write<typeof(highlightFunctionBits)>(highlightSettingsPtr + 40 * glowsetting + 4, highlightFunctionBits);
	apex_mem.Write<typeof(highlightParameter)>(highlightSettingsPtr + 40 * glowsetting + 8, highlightParameter);
	//printf("%d\n", contextId2);
	//printf("%d\n", settingIndex2);
	*/

	//static const int contextId = 5;
	//int settingIndex = 44;
	std::array<unsigned char, 4> highlightFunctionBits = {
		insidevalue,   // InsideFunction
		outsidevalue, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
		outlinesize,  // OutlineRadius: size * 255 / 8
		64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7)
	};
	//std::array<float, 3> highlightParameter = { 0, 1, 0 };
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, contextId);
	apex_mem.Write<unsigned char>(ptr + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId, settingIndex);
	//apex_mem.Write<int>(ptr + 0x298 + contextId, settingIndex);
	long highlightSettingsPtr;
	apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS, highlightSettingsPtr);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 2);
	apex_mem.Write<typeof(highlightFunctionBits)>(highlightSettingsPtr + HIGHLIGHT_TYPE_SIZE * settingIndex + 4, highlightFunctionBits);
	apex_mem.Write<typeof(highlightParameter)>(highlightSettingsPtr + HIGHLIGHT_TYPE_SIZE * settingIndex + 8, highlightParameter);
	apex_mem.Write(g_Base + 0x26C, 1);
	apex_mem.Write(ptr + 0x26C, 1);
	//printf("%f\n", deltaTime2);
	

}

void Entity::disableGlow()
{
	//apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE_GLOW_CONTEXT, 0);
	//mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE, 5);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T1, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T2, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 2);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 5);
}

void Entity::SetViewAngles(SVector angles)
{
	apex_mem.Write<SVector>(ptr + OFFSET_VIEWANGLES, angles);
}

void Entity::SetViewAngles(Vector& angles)
{
	SetViewAngles(SVector(angles));
}

Vector Entity::GetCamPos()
{
	return *(Vector*)(buffer + OFFSET_CAMERAPOS);
}

Vector Entity::GetRecoil()
{
	return *(Vector*)(buffer + OFFSET_AIMPUNCH);
}

void Entity::get_name(uint64_t g_Base, uint64_t index, char* name)
{
	index *= 0x10;
    uint64_t name_ptr = 0;
    apex_mem.Read<uint64_t>(g_Base + OFFSET_NAME_LIST + index, name_ptr);
	apex_mem.ReadArray<char>(name_ptr, name, 32);
}

bool Item::isItem()
{
	char class_name[33] = {};
	get_class_name(ptr, class_name);

	return strncmp(class_name, "CPropSurvival", 13) == 0;
}

bool Item::isGlowing()
{
	return *(int*)(buffer + OFFSET_ITEM_GLOW) == 1363184265;
}

void Item::enableGlow()
{
	//apex_mem.Write<int>(ptr + OFFSET_ITEM_GLOW, 1363184265);


}

void Item::disableGlow()
{
	//apex_mem.Write<int>(ptr + OFFSET_ITEM_GLOW, 1411417991);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 0);
	apex_mem.Write<int>(ptr + OFFSET_HIGHLIGHTSERVERACTIVESTATES + 0, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE, 5);
}

Vector Item::getPosition()
{
	return *(Vector*)(buffer + OFFSET_ORIGIN);
}

float CalculateFov(Entity& from, Entity& target)
{
	Vector ViewAngles = from.GetViewAngles();
	Vector LocalCamera = from.GetCamPos();
	Vector EntityPosition = target.getPosition();
	Vector Angle = Math::CalcAngle(LocalCamera, EntityPosition);
	return Math::GetFov2(ViewAngles, Angle);
}

Vector CalculateBestBoneAim(Entity& from, Entity& target, float max_fov, int bone, int smooth, int aim_no_recoil)
{
	Vector LocalCamera = from.GetCamPos();
	Vector TargetBonePosition = target.getBonePositionByHitbox(bone);
	Vector CalculatedAngles = Vector(0, 0, 0);
	
	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(from.ptr);
	float BulletSpeed = curweap.get_projectile_speed();
	float BulletGrav = curweap.get_projectile_gravity();
	float zoom_fov = curweap.get_zoom_fov();

	if (zoom_fov != 0.0f && zoom_fov != 1.0f)
	{
		max_fov *= zoom_fov/90.0f;
	}

	/*
	//simple aim prediction
	if (BulletSpeed > 1.f)
	{
		Vector LocalBonePosition = from.getBonePosition(bone);
		float VerticalTime = TargetBonePosition.DistTo(LocalBonePosition) / BulletSpeed;
		TargetBonePosition.z += (BulletGrav * 0.5f) * (VerticalTime * VerticalTime);

		float HorizontalTime = TargetBonePosition.DistTo(LocalBonePosition) / BulletSpeed;
		TargetBonePosition += (target.getAbsVelocity() * HorizontalTime);
	}
	*/
	
	//more accurate prediction
	if (BulletSpeed > 1.f)
	{
		PredictCtx Ctx;
		Ctx.StartPos = LocalCamera;
		Ctx.TargetPos = TargetBonePosition; 
		Ctx.BulletSpeed = BulletSpeed - (BulletSpeed*0.08);
		Ctx.BulletGravity = BulletGrav + (BulletGrav*0.05);
		Ctx.TargetVel = target.getAbsVelocity();

		if (BulletPredict(Ctx))
			CalculatedAngles = Vector{Ctx.AimAngles.x, Ctx.AimAngles.y, 0.f};
    }

	if (CalculatedAngles == Vector(0, 0, 0))
    	CalculatedAngles = Math::CalcAngle(LocalCamera, TargetBonePosition);
	Vector ViewAngles = from.GetViewAngles();
	Vector SwayAngles = from.GetSwayAngles();
	//remove sway and recoil , currently only support remove recoil compeletely
	if(aim_no_recoil==2)
		CalculatedAngles-=SwayAngles-ViewAngles;
	Math::NormalizeAngles(CalculatedAngles);
	Vector Delta = CalculatedAngles - ViewAngles;
	double fov = Math::GetFov2(SwayAngles, CalculatedAngles);
	if (fov > max_fov)
	{
		return Vector(0, 0, 0);
	}

	Math::NormalizeAngles(Delta);
	
	Vector SmoothedAngles = ViewAngles + Delta/smooth;
	SmoothedAngles.x = ViewAngles.x + (Delta.x / smooth)*(rand()%10)/7 ;
	SmoothedAngles.y = ViewAngles.y + (Delta.y / smooth) ;
	SmoothedAngles.z = ViewAngles.z + (Delta.z / smooth) * (rand() % 10) / 7;
	//std::cout << (Delta.x / smooth) * (rand() % 10) / 10 * (rand() % 2) << std::endl;

	return SmoothedAngles;
}

Entity getEntity(uintptr_t ptr)
{
	Entity entity = Entity();
	entity.ptr = ptr;
	apex_mem.ReadArray<uint8_t>(ptr, entity.buffer, sizeof(entity.buffer));
	return entity;
}

Item getItem(uintptr_t ptr)
{
	Item entity = Item();
	entity.ptr = ptr;
	apex_mem.ReadArray<uint8_t>(ptr, entity.buffer, sizeof(entity.buffer));
	return entity;
}

bool WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to)
{
	float w = m_vMatrix[12] * from.x + m_vMatrix[13] * from.y + m_vMatrix[14] * from.z + m_vMatrix[15];

	if (w < 0.01f) return false;

	to.x = m_vMatrix[0] * from.x + m_vMatrix[1] * from.y + m_vMatrix[2] * from.z + m_vMatrix[3];
	to.y = m_vMatrix[4] * from.x + m_vMatrix[5] * from.y + m_vMatrix[6] * from.z + m_vMatrix[7];

	float invw = 1.0f / w;
	to.x *= invw;
	to.y *= invw;

	float x = targetWidth / 2;
	float y = targetHeight / 2;

	x += 0.5 * to.x * targetWidth + 0.5;
	y -= 0.5 * to.y * targetHeight + 0.5;

	to.x = x;
	to.y = y;
	to.z = 0;
	return true;
}

void WeaponXEntity::update(uint64_t LocalPlayer)
{
    extern uint64_t g_Base;
	uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
	uint64_t wephandle = 0;
    apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wephandle);
	
	wephandle &= 0xffff;

	uint64_t wep_entity = 0;
    apex_mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);

	projectile_speed = 0;
    apex_mem.Read<float>(wep_entity + OFFSET_BULLET_SPEED, projectile_speed);
	projectile_scale = 0;
    apex_mem.Read<float>(wep_entity + OFFSET_BULLET_SCALE, projectile_scale);
	zoom_fov = 0;
    apex_mem.Read<float>(wep_entity + OFFSET_ZOOM_FOV, zoom_fov);
}

float WeaponXEntity::get_projectile_speed()
{
	return projectile_speed;
}

float WeaponXEntity::get_projectile_gravity()
{
	return 750.0f * projectile_scale;
}

float WeaponXEntity::get_zoom_fov()
{
	return zoom_fov;
}
