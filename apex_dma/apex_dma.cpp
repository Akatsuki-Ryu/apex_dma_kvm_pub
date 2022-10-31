#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <random>
#include <chrono>
#include <iostream>
#include <cfloat>
#include "Game.h"
#include <thread>
//this is a test, with seconds
#include <fcntl.h>
#include <sys/stat.h>

Memory apex_mem;
Memory client_mem;

bool DEBUG_PRINT = false;

bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t lastaimentity = 0;
float max = 999.0f;
float max_dist = 300.0f*40.0f;	// ESP & Glow distance in meters (*40)
int localTeamId = 0; //this might be int team_player
//int tmp_spec = 0, spectators = 0; // defined below in the new version
//int tmp_all_spec = 0, allied_spectators = 0; // defined below in the new version
float max_fov = 10.0f;
int team_player = 0;
const int toRead = 100;
//int aim = true;
bool esp = true;
int aim = 2; 					// 0 = off, 1 = on - no visibility check, 2 = on - use visibility check
int player_glow = 2;			// 0 = off, 1 = on - not visible through walls, 2 = on - visible through walls
float recoil_control = 0.50f;	// recoil reduction by this value, 1 = 100% = no recoil
QAngle last_sway = QAngle();	// used to determine when to reduce recoil
int last_sway_counter = 0;		// to determine if we might be shooting a semi-auto rifle so we need to hold to last_sway

bool item_glow = true;
//bool player_glow = true;
//extern bool aim_no_recoil;
bool firing_range = false;
bool target_allies = false;
int aim_no_recoil = 0;			//  0= normal recoil, 1 = use recoil control, 2 = aiming no recoil // when using recoil control , make sure the aimbot is off
int safe_level = 0;
bool aiming = false;
//extern float smooth;
//extern int bone;
bool thirdperson = false;

//chargerifle hack
bool chargerifle = false;
bool shooting = false;

int smooth = 50;
int bone = 3;
bool walls = false;
int dynamicsmooth = 50;
float dynamicmax_fov = 10.0f;
bool thirdPerson = false;

//Player Glow Color and Brightness
float glowr = 0.0f;
float glowg = 120.0f;
float glowb = 120.0f;
int glowtype = 1;
int glowtype2 = 2;


bool actions_t = false;
bool esp_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
bool recoil_t = false;
uint64_t g_Base;
uint64_t c_Base;
bool next2 = false;
bool valid = false;
bool lock = false;

//recoil control branch related
const char* printPipe = "/tmp/myfifo";	// output pipe
const char* pipeClearCmd = "\033[H\033[2J\033[3J";	// escaped 'clear' command
int shellOut = -1;

typedef struct player
{
	float dist = 0;
	int entity_team = 0;
	float boxMiddle = 0;
	float h_y = 0;
	float width = 0;
	float height = 0;
	float b_x = 0;
	float b_y = 0;
	bool knocked = false;
	bool visible = false;
	int health = 0;
	int shield = 0;
	int maxshield = 0;
	int armortype = 0;
	Vector EntityPosition;
	Vector LocalPlayerPosition;
	QAngle localviewangle;
	char name[33] = { 0 };
}player;



struct Matrix
{
	float matrix[16];
};

float lastvis_esp[toRead];
float lastvis_aim[toRead];

int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////

void SetPlayerGlow(Entity& LPlayer, Entity& Target, int index)
{
	if (player_glow >= 1)
	{
		if (LPlayer.getPosition().z < 8000.f && Target.getPosition().z < 8000.f)
		{
			if (!Target.isGlowing() || (int)Target.buffer[OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE] != 1 || (int)Target.buffer[GLOW_FADE] != 872415232) {
				float currentEntityTime = 5000.f;
				if (!isnan(currentEntityTime) && currentEntityTime > 0.f) {
					GColor color;
					if ((Target.getTeamId() == LPlayer.getTeamId()) && !target_allies)
					{
						color = { 0.f, 2.f, 3.f };
					}
					else if (!(firing_range) && (Target.isKnocked() || !Target.isAlive()))
					{
						color = { 3.f, 3.f, 3.f };
					}
					else if (Target.lastVisTime() > lastvis_aim[index] || (Target.lastVisTime() < 0.f && lastvis_aim[index] > 0.f))
					{
						color = { 0.f, 1.f, 0.f };
					}
					else
					{
						int shield = Target.getShield();

						if (shield > 100)
						{ //Heirloom armor - Red
							color = { 3.f, 0.f, 0.f };
						}
						else if (shield > 75)
						{ //Purple armor - Purple
							color = { 1.84f, 0.46f, 2.07f };
						}
						else if (shield > 50)
						{ //Blue armor - Light blue
							color = { 0.39f, 1.77f, 2.85f };
						}
						else if (shield > 0)
						{ //White armor - White
							color = { 2.f, 2.f, 2.f };
						}
						else if (Target.getHealth() > 50)
						{ //Above 50% HP - Orange
							color = { 3.5f, 1.8f, 0.f };
						}
						else
						{ //Below 50% HP - Light Red
							color = { 3.28f, 0.78f, 0.63f };
						}
					}

					Target.enableGlow(color);
				}
			}
		}

	}
	else if ((player_glow == 0) && Target.isGlowing())
	{
		Target.disableGlow();
	}
}


void ProcessPlayer(Entity& LPlayer, Entity& target, uint64_t entitylist, int index)
{
	int entity_team = target.getTeamId();

	/*bool obs = target.Observing(entitylist);
	if (obs)
	{
		if(obs == LPlayer.ptr)
		{
			if (entity_team == localTeamId)
			{
				tmp_all_spec++;
			}
			else
			{
				tmp_spec++;
			}
		}
		tmp_spec++;
		return;
	}*/

	if (!target.isAlive())
	{
		float localyaw = LPlayer.GetYaw();
		float targetyaw = target.GetYaw();

		if (localyaw == targetyaw)
		{
			if (LPlayer.getTeamId() == entity_team)
				tmp_all_spec++;
			else
				tmp_spec++;
		}
		return;
	}

	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = LPlayer.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);
	if (dist > max_dist)
	{
		if (target.isGlowing())
		{
			target.disableGlow();
		}
		return;
	}

	//if (!target.isAlive()) return;

	if(!firing_range)
		if (entity_team < 0 || entity_team>50 || entity_team == team_player) return;

	if (!target_allies && (entity_team == localTeamId)) return; //todo check logic

	if(aim==2)
	{
		if((target.lastVisTime() > lastvis_aim[index]))
		{
			float fov = CalculateFov(LPlayer, target);
			if (fov < max)
			{
				max = fov;
				tmp_aimentity = target.ptr;
			}
		}
		else
		{
			if(aimentity==target.ptr)
			{
				aimentity=tmp_aimentity=lastaimentity=0;
			}
		}
	}
	else
	{
		float fov = CalculateFov(LPlayer, target);
		if (fov < max)
		{
			max = fov;
			tmp_aimentity = target.ptr;
		}
	}

	SetPlayerGlow(LPlayer, target, index);

	lastvis_aim[index] = target.lastVisTime();
}

void DoActions()
{
	actions_t = true;
	while (actions_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		bool tmp_thirdperson = false;
		bool tmp_chargerifle = false;
		uint32_t counter = 0;

		while (g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));	

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			localTeamId = LPlayer.getTeamId(); // original logic
			if (localTeamId < 0 || localTeamId > 50)
			{
				continue;
			}

			team_player = LPlayer.getTeamId(); //new logic
			if (team_player < 0 || team_player>50)
			{
				continue;
			}

			if(thirdperson && !tmp_thirdperson)
			{
				if(!aiming)
				{
					apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, 1);
					apex_mem.Write<int>(LPlayer.ptr + OFFSET_THIRDPERSON_SV, 1);
					tmp_thirdperson = true;
				}			
			}
			else if((!thirdperson && tmp_thirdperson) || aiming)
			{
				if(tmp_thirdperson)
				{
					apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, -1);
					apex_mem.Write<int>(LPlayer.ptr + OFFSET_THIRDPERSON_SV, 0);
					tmp_thirdperson = false;
				}	
			}

			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}

			max = 999.0f;
			tmp_spec = 0;
			tmp_all_spec = 0;
			tmp_aimentity = 0;
			if(firing_range)
			{
				int c=0;
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isDummy()) //&& !target_allies
					{
						continue;
					}

					if(player_glow && !Target.isGlowing())
					{
						Target.enableGlow();
					}
					else if(!player_glow && Target.isGlowing())
					{
						Target.disableGlow();
					}

					ProcessPlayer(LPlayer, Target, entitylist, c);
					c++;
				}
			}
			else
			{
				for (int i = 0; i < toRead; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isPlayer())
					{
						continue;
					}
					
					ProcessPlayer(LPlayer, Target, entitylist, i);

					int entity_team = Target.getTeamId();
					if (!target_allies &&(entity_team == team_player))
					{
						continue;
					}


					switch (safe_level)
					{
					case 1:
						if (spectators > 0)
						{
							if(Target.isGlowing())
							{
								Target.disableGlow();
							}
							continue;
						}
						break;
					case 2:
						if (spectators + allied_spectators > 0)
						{
							if(Target.isGlowing())
							{
								Target.disableGlow();
							}
							continue;
						}
						break;
					default:
						break;
					}

					ProcessPlayer(LPlayer, Target, entitylist, i);

					if(player_glow && !Target.isGlowing()) //new stuff , but this logic is in the previous session about safetylevel
					{
						Target.enableGlow();
					}
					else if(!player_glow && Target.isGlowing())
					{
						Target.disableGlow();
					}
				}
			}


			if (!spectators && !allied_spectators)
			{
				spectators = tmp_spec;
				allied_spectators = tmp_all_spec;
			}
			else
			{
				//refresh spectators count every ~2 seconds
				counter++;
				if (counter == 70)
				{
					spectators = tmp_spec;
					allied_spectators = tmp_all_spec;
					counter = 0;
				}
			}

			if(!lock)
				aimentity = tmp_aimentity;
			else
				aimentity = lastaimentity;
			
			if(chargerifle)
			{
				charge_rifle_hack(LocalPlayer);
				tmp_chargerifle = true;
			}
			else
			{
				if(tmp_chargerifle)
				{
					apex_mem.Write<float>(g_Base + OFFSET_TIMESCALE + 0x68, 1.f);
					tmp_chargerifle = false;
				}
			}
		}
	}
	actions_t = false;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////

player players[toRead]; //todo double check if it is duplicated


static void EspLoop()  //todo double check if it is duplicated
{
	esp_t = true;
	while(esp_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (esp)
			{
				valid = false;

				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0)
				{
					next2 = true;
					while(next2 && g_Base!=0 && c_Base!=0 && esp)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					continue;
				}
				Entity LPlayer = getEntity(LocalPlayer);
				int team_player = LPlayer.getTeamId();
				if (team_player < 0 || team_player>50)
				{
					next2 = true;
					while(next2 && g_Base!=0 && c_Base!=0 && esp)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					continue;
				}
				Vector LocalPlayerPosition = LPlayer.getPosition();

				uint64_t viewRenderer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_RENDER, viewRenderer);
				uint64_t viewMatrix = 0;
				apex_mem.Read<uint64_t>(viewRenderer + OFFSET_MATRIX, viewMatrix);
				Matrix m = {};
				apex_mem.Read<Matrix>(viewMatrix, m);

				uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
				
				memset(players,0,sizeof(players));

				if(firing_range)
				{
					int c=0;
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>( entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0)
						{
							continue;
						}		
						
						if (LocalPlayer == centity)
						{
							continue;
						}

						Entity Target = getEntity(centity);

						if (!Target.isDummy())
						{
							continue;
						}

						if (!Target.isAlive())
						{
							continue;
						}
						int entity_team = Target.getTeamId();

						Vector EntityPosition = Target.getPosition();
						float dist = LocalPlayerPosition.DistTo(EntityPosition);

						if (dist > max_dist || dist < 50.0f)
						{	
							continue;
						}
						
						Vector bs = Vector();
						WorldToScreen(EntityPosition, m.matrix, 1920, 1080, bs);
						if (esp)
						{
							Vector hs = Vector();
							Vector HeadPosition = Target.getBonePositionByHitbox(0);
							WorldToScreen(HeadPosition, m.matrix, 1920, 1080, hs);
							float height = abs(abs(hs.y) - abs(bs.y));
							float width = height / 2.0f;
							float boxMiddle = bs.x - (width / 2.0f);
							int health = Target.getHealth();
							int shield = Target.getShield();
							int maxshield = Target.getMaxshield();
							int armortype = Target.getArmortype();
							players[c] = 
							{
								dist,
								entity_team,
								boxMiddle,
								hs.y,
								width,
								height,
								bs.x,
								bs.y,
								0,
								(Target.lastVisTime() > lastvis_esp[c]),
								health,
								shield,
								maxshield,
								armortype
								
							};
							Target.get_name(g_Base, i-1, &players[c].name[0]);
							lastvis_esp[c] = Target.lastVisTime();
							valid = true;
							c++;
						}
					}
				}	
				else
				{
					for (int i = 0; i < toRead; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>( entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0)
						{
							continue;
						}
						
						if (LocalPlayer == centity)
						{
							continue;
						}

						Entity Target = getEntity(centity);
						
						if (!Target.isPlayer())
						{
							continue;
						}

						if (!Target.isAlive())
						{
							continue;
						}

						int entity_team = Target.getTeamId();
						if (entity_team < 0 || entity_team>50 || entity_team == team_player)
						{
							continue;
						}

						Vector EntityPosition = Target.getPosition();
						float dist = LocalPlayerPosition.DistTo(EntityPosition);
						if (dist > max_dist || dist < 50.0f)
						{	
							continue;
						}

						Vector bs = Vector();
						WorldToScreen(EntityPosition, m.matrix, 1920, 1080, bs);
						if (esp)
						{
							Vector hs = Vector();
							Vector HeadPosition = Target.getBonePositionByHitbox(0);
							WorldToScreen(HeadPosition, m.matrix, 1920, 1080, hs);
							float height = abs(abs(hs.y) - abs(bs.y));
							float width = height / 2.0f;
							float boxMiddle = bs.x - (width / 2.0f);
							int health = Target.getHealth();
							int shield = Target.getShield();
							int maxshield = Target.getMaxshield();
							int armortype = Target.getArmortype();
							Vector EntityPosition = Target.getPosition();
							Vector LocalPlayerPosition = LPlayer.getPosition();
							QAngle localviewangle = LPlayer.GetViewAngles();
							players[i] = 
							{
								dist,
								entity_team,
								boxMiddle,
								hs.y,
								width,
								height,
								bs.x,
								bs.y,
								Target.isKnocked(),
								(Target.lastVisTime() > lastvis_esp[i]),
								health,
								shield,
								maxshield,
								armortype,
								EntityPosition,
								LocalPlayerPosition,
								localviewangle
							};
							Target.get_name(g_Base, i-1, &players[i].name[0]);
							lastvis_esp[i] = Target.lastVisTime();
							valid = true;
						}
					}
				}

				next2 = true;
				while(next2 && g_Base!=0 && c_Base!=0 && esp)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
	}
	esp_t = false;
}

static void AimbotLoop()
{
	aim_t = true;
	while (aim_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (aim>0)
			{
				switch (safe_level)
				{
				case 1:
					if (spectators > 0)
					{
						continue;
					}
					break;
				case 2:
					if (spectators+allied_spectators > 0)
					{
						continue;
					}
					break;
				default:
					break;
				}
				
				if (aimentity == 0 || !aiming)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				lock=true;
				lastaimentity = aimentity;
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0) continue;
				Entity LPlayer = getEntity(LocalPlayer);
				Entity target = getEntity(aimentity);

				if (firing_range)
				{
					if (!target.isAlive())
					{
						continue;
					}
				}
				else
				{
					if (!target.isAlive() || target.isKnocked())
					{
						continue;
					}
				}

				Vector EntityPosition = target.getPosition();
				Vector LocalPlayerPosition = LPlayer.getPosition();
				float dist = LocalPlayerPosition.DistTo(EntityPosition);

				if (dist> 25*40)
				{
					//printf("dist: bigger than 25\t");
					dynamicsmooth = smooth;
					dynamicmax_fov = max_fov;

				}
				else if (dist <=25*40 && dist >10*40)
				{
					//printf("dist: 10 to 25\t");
					dynamicsmooth = smooth * 1.5f;
					dynamicmax_fov = max_fov * 2.0f;
				}
				else if (dist <= 10 * 40)
				{
					//printf("dist: smaller than 10");
					dynamicsmooth = smooth * 2.0f;
					dynamicmax_fov = max_fov * 4.0f;

				}
				//printf("dist:\t");
				//printf("%f\n", dist);


//				Vector Angles = CalculateBestBoneAim(LPlayer, target, dynamicmax_fov, bone, dynamicsmooth, aim_no_recoil);
				QAngle Angles = CalculateBestBoneAim(LPlayer, aimentity, dynamicmax_fov, bone, dynamicsmooth, aim_no_recoil); //todo tobe checked
				if (Angles.x == 0 && Angles.y == 0)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				LPlayer.SetViewAngles(Angles);
			}
		}
	}
	aim_t = false;
}

static void PrintVarsToConsole() {
	printf("\n Spectators\t\t\t\t\t\t\t     Glow\n");
	printf("Enemy  Ally   Smooth\t   Aimbot\t     If Spectators\t Items  Players\n");

	// spectators
	printf(" %d\t%d\t", spectators, allied_spectators);

	// smooth
	printf("%d\t", smooth);

	// aim definition
	switch (aim)
	{
	case 0:
		printf("OFF\t\t\t");
		break;
	case 1:
		printf("ON - No Vis-check    ");
		break;
	case 2:
		printf("ON - Vis-check       ");
		break;
	default:
		printf("--\t\t\t");
		break;
	}

	// safe level definition
	switch (safe_level)
	{
	case 0:
		printf("Keep ON\t\t");
		break;
	case 1:
		printf("OFF with enemy\t");
		break;
	case 2:
		printf("OFF with any\t");
		break;
	default:
		printf("--\t\t");
		break;
	}
	
	// glow items + key
	printf((item_glow ? "  ON\t" : "  OFF\t"));

	// glow players + key
	switch (player_glow)
	{
	case 0:
		printf("  OFF\t");
		break;
	case 1:
		printf("ON - without walls\t");
		break;
	case 2:
		printf("ON - with walls\t");
		break;
	default:
		printf("  --\t");
		break;
	}

	// new string
	printf("\nFiring Range\tTarget Allies\tNo-recoil    Max Distance\n");

	// firing range + key
	printf((firing_range ? "   ON\t\t" : "   OFF\t\t"));

	// target allies + key
	printf((target_allies ? "   ON\t" : "   OFF\t"));

	// recoil + key
	switch (aim_no_recoil)
	{
	case 0:
		printf("\t  OFF\t");
		break;
	case 1:
		printf("\t  RCS\t");
		break;
	case 2:
		printf("\t  ON\t");
		break;
	default:
		printf("  --\t");
		break;
	}


	// distance
	printf("\t%d\n\n", (int)max_dist);
}

static void set_vars(uint64_t add_addr)
{
	printf("Reading client vars...\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	//Get addresses of client vars
	uint64_t check_addr = 0;
	client_mem.Read<uint64_t>(add_addr, check_addr);
	uint64_t aim_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t), aim_addr);
	uint64_t esp_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*2, esp_addr);
	uint64_t aiming_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*3, aiming_addr);
	uint64_t g_Base_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*4, g_Base_addr);
	uint64_t next2_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*5, next2_addr);
	uint64_t player_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*6, player_addr);
	uint64_t valid_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*7, valid_addr);
	uint64_t max_dist_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*8, max_dist_addr);
	uint64_t item_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*9, item_glow_addr);
	uint64_t player_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*10, player_glow_addr);
	uint64_t aim_no_recoil_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*11, aim_no_recoil_addr);
	uint64_t smooth_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*12, smooth_addr);
	uint64_t max_fov_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*13, max_fov_addr);
	uint64_t bone_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*14, bone_addr);
	uint64_t thirdperson_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*15, thirdperson_addr);
	uint64_t spectators_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*16, spectators_addr);
	uint64_t allied_spectators_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*17, allied_spectators_addr); //todo need to know what is target_allies_addr at 14
	uint64_t chargerifle_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*18, chargerifle_addr);
	uint64_t shooting_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*19, shooting_addr);
	uint64_t glowr_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*20, glowr_addr);
	uint64_t glowg_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*21, glowg_addr);
	uint64_t glowb_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*22, glowb_addr);
	uint64_t firing_range_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*23, firing_range_addr);
	uint64_t glowtype_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*24, glowtype_addr);
	uint64_t glowtype2_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*25, glowtype2_addr);

	uint32_t check = 0;
	client_mem.Read<uint32_t>(check_addr, check);
	
	if(check != 0xABCD)
	{
		printf("Incorrect values read. Check if the add_off is correct. Quitting.\n");
		active = false;
		return;
	}
	vars_t = true;
	auto nextUpdateTime = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while(vars_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if(c_Base!=0 && g_Base!=0)
		{
			client_mem.Write<uint32_t>(check_addr, 0);
			printf("\nReady\n");
		}

		while(c_Base!=0 && g_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			client_mem.Write<uint64_t>(g_Base_addr, g_Base);
			client_mem.Write<int>(spectators_addr, spectators);
			client_mem.Write<int>(allied_spectators_addr, allied_spectators);

			client_mem.Read<int>(aim_addr, aim);
			client_mem.Read<bool>(esp_addr, esp);
			client_mem.Read<bool>(aiming_addr, aiming);
			client_mem.Read<float>(max_dist_addr, max_dist);
			client_mem.Read<bool>(item_glow_addr, item_glow);
//			client_mem.Read<bool>(player_glow_addr, player_glow);
//			client_mem.Read<bool>(aim_no_recoil_addr, aim_no_recoil);
//			client_mem.Read<float>(smooth_addr, smooth);
			client_mem.Read<int>(player_glow_addr, player_glow);
			client_mem.Read<int>(aim_no_recoil_addr, aim_no_recoil);
			client_mem.Read<int>(smooth_addr, smooth);
			client_mem.Read<float>(max_fov_addr, max_fov);
			client_mem.Read<int>(bone_addr, bone);
			client_mem.Read<bool>(thirdperson_addr, thirdperson);
			client_mem.Read<bool>(shooting_addr, shooting);
			client_mem.Read<bool>(chargerifle_addr, chargerifle);
			client_mem.Read<float>(glowr_addr, glowr);
			client_mem.Read<float>(glowg_addr, glowg);
			client_mem.Read<float>(glowb_addr, glowb);
			client_mem.Read<bool>(firing_range_addr, firing_range);
			client_mem.Read<int>(glowtype_addr, glowtype);
			client_mem.Read<int>(glowtype2_addr, glowtype2);

			if(esp && next2) //todo what is next2
			{
				if(valid)
				client_mem.WriteArray<player>(player_addr, players, toRead);
				client_mem.Write<bool>(valid_addr, valid);
				client_mem.Write<bool>(next2_addr, true); //next2

				bool next2_val = false;
				do
				{
					client_mem.Read<bool>(next2_addr, next2_val);
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				} while (next2_val && g_Base!=0 && c_Base!=0);
				
				next2 = false;

			if (nextUpdateTime < std::chrono::system_clock::now()) { //todo maybe this is next2 implemnetation in the old code ?
				PrintVarsToConsole();
				nextUpdateTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
			}
		}
	}
	vars_t = false;
}
}

//// Item Glow Stuff // this is relocated into the game.h
//struct GlowMode {
//	int8_t GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
//};
 
static void item_glow_t()
{
	item_t = true;
	while(item_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		int k = 0;
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			if (item_glow)
			{
				for (int i = 0; i < 20000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					Item item = getItem(centity);
 
 
					if (item.isBox())
					{
						apex_mem.Write<int>(centity + 0x262, 16256);
						apex_mem.Write<int>(centity + 0x2dc, 1193322764);
						apex_mem.Write<int>(centity + 0x3c8, 7);
						apex_mem.Write<int>(centity + 0x3d0, 2);
						
					}
					
					if(item.isItem() && !item.isGlowing())
					{
						item.enableGlow();
					}
					
				}
				k=1;
				std::this_thread::sleep_for(std::chrono::milliseconds(1)); //todo code base is 600
			}
			else
			{		
				if(k==1)
				{
					for (int i = 0; i < 20000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0) continue;
 
						Item item = getItem(centity);
 
						if(item.isItem() && item.isGlowing())
						{
							item.disableGlow();
						}
					}
					k=0;
				}
			}	
		}
	}
	item_t = false;
}

static void RecoilLoop() //todo consider remove
{
	recoil_t = true;
	while (recoil_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(12));
			if (aim_no_recoil == 1)
			{
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);

				if (LocalPlayer == 0) continue;
				last_sway_counter++;
				if (last_sway_counter > 10000)
					last_sway_counter = 86;

				int attackState = 0;
				apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);

				if (attackState != 5) {
					if (last_sway.x != 0 && last_sway_counter > 85) {	// ~ about 1 second between shot is considered semi-auto so we keep last_sway
						last_sway.x = 0;
						last_sway.y = 0;
						last_sway_counter = 0;
					}
					continue; // is not firing
				}

				Entity LPlayer = getEntity(LocalPlayer);
				QAngle ViewAngles = LPlayer.GetViewAngles();
				QAngle SwayAngles = LPlayer.GetSwayAngles();

			// calculate recoil angles
			QAngle recoilAngles = SwayAngles - ViewAngles;
			if (recoilAngles.x == 0 || recoilAngles.y == 0 || (recoilAngles.x - last_sway.x) == 0) 
				continue;

				// reduce recoil angles by last recoil as sway is continous
				ViewAngles.x -= ((recoilAngles.x - last_sway.x) * recoil_control);
				ViewAngles.y -= ((recoilAngles.y - last_sway.y) * recoil_control);
				LPlayer.SetViewAngles(ViewAngles);
				last_sway = recoilAngles;
				last_sway_counter = 0;
			}
		}
	}
	recoil_t = false;
}

// Requires an open pipe
static void printToPipe(std::string msg, bool clearShell = false) //todo consider remove
{
	char buf[80];
	if (clearShell) {
		strcpy(buf, pipeClearCmd);
		write(shellOut, buf, strlen(buf)+1);
	}
	strcpy(buf, msg.c_str());
	write(shellOut, buf, strlen(buf)+1);
}

static void DebugLoop() //todo consider remove
{
	while (DEBUG_PRINT)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);

			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			int attackState = 0;
			apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);
			Vector LocalCamera = LPlayer.GetCamPos();
			QAngle ViewAngles = LPlayer.GetViewAngles();
			QAngle SwayAngles = LPlayer.GetSwayAngles();

			uint64_t wepHandle = 0;
			apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wepHandle);
			wepHandle &= 0xffff;
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			uint64_t wep_entity = 0;
			apex_mem.Read<uint64_t>(entitylist + (wepHandle << 5), wep_entity);
			int ammoInClip = 0;
			apex_mem.Read<int>(wep_entity + OFFSET_AMMO_IN_CLIP, ammoInClip);

			printToPipe("Attack State:\t" + std::to_string(attackState) + "\n", true);
			//printToPipe("Local Camera:\t" + std::to_string(LocalCamera.x) + " " + std::to_string(LocalCamera.y) + " " + std::to_string(LocalCamera.z) + "\n");
			printToPipe("View Angles: \t" + std::to_string(ViewAngles.x) + " " + std::to_string(ViewAngles.y) + " " + std::to_string(ViewAngles.z) + "\n");
			printToPipe("Sway Angles: \t" + std::to_string(SwayAngles.x) + " " + std::to_string(SwayAngles.y) + " " + std::to_string(SwayAngles.z) + "\n");
			printToPipe("Ammo Count:  \t" + std::to_string(ammoInClip)  + "\n");
		}
	}
}

int main(int argc, char *argv[])
{
	if(geteuid() != 0)
	{
		printf("Error: %s is not running as root\n", argv[0]);
		return 0;
	}

	const char* cl_proc = "boxsecclient.exe";
	const char* ap_proc = "R5Apex.exe";
	//const char* ap_proc = "EasyAntiCheat_launcher.exe";

	//Client "add" offset
	uint64_t add_off = 0x40d0;
	
	// start external terminal and open pipe to print to it
	//todo consider remove
	if (DEBUG_PRINT) {
		system("gnome-terminal -- cat /tmp/myfifo");
		mkfifo(printPipe, 0666);
		shellOut = open(printPipe, O_WRONLY);
	}


	std::thread aimbot_thr;
	std::thread esp_thr;
	std::thread actions_thr;
	std::thread itemglow_thr;
	std::thread vars_thr;
	std::thread recoil_thr;//todo consider remove
	std::thread debug_thr;//todo consider remove
	
	while(active)
	{
		if(apex_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(aim_t)
			{
				aim_t = false;
				esp_t = false;
				actions_t = false;
				item_t = false;
				recoil_t = false; //todo consider remove
				g_Base = 0;

				aimbot_thr.~thread();
				esp_thr.~thread();
				actions_thr.~thread();
				itemglow_thr.~thread();
				recoil_thr.~thread(); //todo consider remove
				debug_thr.~thread();//todo consider remove
				
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for apex process...\n");

			apex_mem.open_proc(ap_proc);

			if(apex_mem.get_proc_status() == process_status::FOUND_READY)
			{
				g_Base = apex_mem.get_proc_baseaddr();
				printf("\nApex process found\n");
				printf("Base: %lx\n", g_Base);

				aimbot_thr = std::thread(AimbotLoop);
				esp_thr = std::thread(EspLoop);
				actions_thr = std::thread(DoActions);
				itemglow_thr = std::thread(item_glow_t);
				recoil_thr = std::thread(RecoilLoop);//todo consider remove

				if (DEBUG_PRINT)//consider remove
				{
					debug_thr = std::thread(DebugLoop);
					debug_thr.detach();
				}

				aimbot_thr.detach();
				esp_thr.detach();
				actions_thr.detach();
				itemglow_thr.detach();
				recoil_thr.detach(); //todo consider remove
			}
		}
		else
		{
			apex_mem.check_proc();
		}

		if(client_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(vars_t)
			{
				vars_t = false;
				c_Base = 0;

				vars_thr.~thread();
			}
			
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for client process...\n");

			client_mem.open_proc(cl_proc);

			if(client_mem.get_proc_status() == process_status::FOUND_READY)
			{
				c_Base = client_mem.get_proc_baseaddr();
				printf("\nClient process found\n");
				printf("Base: %lx\n", c_Base);

				vars_thr = std::thread(set_vars, c_Base + add_off);
				vars_thr.detach();
			}
		}
		else
		{
			client_mem.check_proc();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}