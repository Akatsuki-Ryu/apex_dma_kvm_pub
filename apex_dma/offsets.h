#define ORIGIN 1
#define STEAM 2

#define VERSION STEAM

#if VERSION == STEAM

    #define OFFSET_ENTITYLIST			0x1e23418
    #define OFFSET_LOCAL_ENT			0x21d3758     //0x22244E8  //0x2224528     //.?AVC_GameMovement@@ + 0x8
    #define OFFSET_NAME_LIST            0xc266a60
    #define OFFSET_THIRDPERSON          0x01db1f70 + 0x6c//thirdperson_override + 0x6c
    #define OFFSET_TIMESCALE            0x01768ad0 //host_timescale

    #define OFFSET_TEAM					0x037c //m_iTeamNum
    #define OFFSET_HEALTH				0x036c  //m_iHealth
    #define OFFSET_SHIELD				0x01a0 //m_shieldHealth
    #define OFFSET_MAXSHIELD            0x01a4 //m_shieldHealthMax
    #define OFFSET_ARMORTYPE            0x4694 //armortype
    #define OFFSET_NAME					0x04b9 //m_iName
    #define OFFSET_SIGN_NAME			0x04b0 //m_iSignifierName
    #define OFFSET_ABS_VELOCITY         0x0170 //m_vecAbsVelocity
    #define OFFSET_VISIBLE_TIME         0x19B0 //0x19c8   //CPlayer!lastVisibleTime
    #define OFFSET_ZOOMING      		0x1b91 //m_bZooming
    #define OFFSET_THIRDPERSON_SV       0x3728 //m_thirdPersonShoulderView
    #define OFFSET_YAW                  0x21fc - 0x8//m_currentFramePlayer.m_ammoPoolCount - 0x8

    #define OFFSET_LIFE_STATE			0x06c8  //m_lifeState, >0 = dead
    #define OFFSET_BLEED_OUT_STATE		0x26a0          //m_bleedoutState, >0 = knocked

    #define OFFSET_ORIGIN				0x017c //m_vecAbsOrigin
    #define OFFSET_BONES				0x0dd0 + 0x48  //m_nForceBone + 0x48
    #define OFFSET_STUDIOHDR            0x1020 //CBaseAnimating!m_pStudioHdr
    #define OFFSET_AIMPUNCH				0x23f8  //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
    #define OFFSET_CAMERAPOS			0x1e90   //CPlayer!camera_origin
    #define OFFSET_VIEWANGLES			0x24f4 - 0x14 //m_ammoPoolCapacity - 0x14
    #define OFFSET_BREATH_ANGLES		OFFSET_VIEWANGLES - 0x10
    #define OFFSET_OBSERVER_MODE		0x3534 //m_iObserverMode
    #define OFFSET_OBSERVING_TARGET		0x3540 //m_hObserverTarget

    #define OFFSET_MATRIX				0x11a350
    #define OFFSET_RENDER				0x74210a8

    #define OFFSET_WEAPON				0x1a44 //m_latestPrimaryWeapons
    #define OFFSET_BULLET_SPEED         0x19B0 + 0x04d4//0x1f6c//0x1aa0 + 0x04cc   // WeaponSettingsMeta.base + WeaponSettings.projectile_launch_speed 0x1f9c
    #define OFFSET_BULLET_SCALE         0x19B0 + 0x04dc//0x1f74//0x1aa0 + 0x04d4  // WeaponSettingsMeta.base + WeaponSettings.projectile_gravity_scale 0x1f94
    #define OFFSET_ZOOM_FOV             0x15f0 + 0xb8 //m_playerData + m_curZoomFOV
    #define OFFSET_AMMO                 0x1664 //m_ammoInClip

    #define OFFSET_ITEM_GLOW            0x02f0  //m_highlightFunctionBits

    #define OFFSET_GLOW_T1              0x292 //16256 = enabled, 0 = disabled
    #define OFFSET_GLOW_T2              0x30c //1193322764 = enabled, 0 = disabled
    #define OFFSET_GLOW_ENABLE          0x294  //7 = enabled, 2 = disabled
    #define OFFSET_GLOW_THROUGH_WALLS   0x278  //2 = enabled, 5 = disabled

    #define OFFSET_GLOW_ENABLE_GLOW_CONTEXT                 OFFSET_GLOW_ENABLE // Script_Highlight_SetCurrentContext
    #define OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE     OFFSET_GLOW_THROUGH_WALLS // Script_Highlight_SetVisibilityType 5th mov
    #define GLOW_LIFE_TIME              0x3A4 + 0x30 // Script_Highlight_SetLifeTime + 4
    #define GLOW_DISTANCE               0x26c // Script_Highlight_SetFarFadeDist
    #define GLOW_TYPE                   0x29c//0x2C4+ 0x30//0x29c // Script_Highlight_GetState + 4
    #define GLOW_COLOR                  0x1D0 + 0x30 // Script_CopyHighlightState 15th mov
    #define GLOW_FADE                   0x388 // Script_Highlight_GetCurrentInsideOpacity 3rd result of 3 offsets consecutive or first + 8
    #define OFFSET_HIGHLIGHTSERVERACTIVESTATES 0x298

    #define HIGHLIGHT_SETTINGS 0xb5f9620 //?
    #define HIGHLIGHT_TYPE_SIZE 0x28	//?

    #define OFFSET_IS_ATTACKING         0x075197f0 + 0x8 // kbutton_t in_attack state..

    #define OFFSET_AMMO_IN_CLIP         0x1634 // m_ammoInClip

#elif VERSION == ORIGIN

    #define OFFSET_ENTITYLIST			0x1b0e718
    #define OFFSET_LOCAL_ENT			0x1ebf528 //LocalPlayer
    #define OFFSET_NAME_LIST            0xbc1c1c0
    #define OFFSET_THIRDPERSON          0x01af35c0 + 0x6c //thirdperson_override + 0x6c
    #define OFFSET_TIMESCALE            0x014ad6a0 //host_timescale

    #define OFFSET_TEAM					0x044c //m_iTeamNum
    #define OFFSET_HEALTH				0x043c //m_iHealth
    #define OFFSET_SHIELD				0x170 //m_shieldHealth
    #define OFFSET_NAME					0x589 //m_iName
    #define OFFSET_SIGN_NAME			0x580 //m_iSignifierName
    #define OFFSET_ABS_VELOCITY         0x140 //m_vecAbsVelocity
    #define OFFSET_VISIBLE_TIME         0x1a70 //CPlayer!lastVisibleTime
    #define OFFSET_ZOOMING      		0x1c31 //m_bZooming
    #define OFFSET_THIRDPERSON_SV       0x36a0 //m_thirdPersonShoulderView
    #define OFFSET_YAW                  0x228c - 0x8 //m_currentFramePlayer.m_ammoPoolCount - 0x8

    #define OFFSET_LIFE_STATE			0x798  //m_lifeState, >0 = dead
    #define OFFSET_BLEED_OUT_STATE		0x2718 //m_bleedoutState, >0 = knocked

    #define OFFSET_ORIGIN				0x014c //m_vecAbsOrigin
    #define OFFSET_BONES				0x0e98 + 0x48 //m_nForceBone + 0x48
    #define OFFSET_STUDIOHDR            0x10f0 //CBaseAnimating!m_pStudioHdr
    #define OFFSET_AIMPUNCH				0x2488 //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
    #define OFFSET_CAMERAPOS			0x1f28 //CPlayer!camera_origin
    #define OFFSET_VIEWANGLES			0x2584 - 0x14 //m_ammoPoolCapacity - 0x14
    #define OFFSET_BREATH_ANGLES		OFFSET_VIEWANGLES - 0x10
    #define OFFSET_OBSERVER_MODE		0x34ac //m_iObserverMode
    #define OFFSET_OBSERVING_TARGET		0x34b8 //m_hObserverTarget

    #define OFFSET_MATRIX				0x11a210
    #define OFFSET_RENDER				0x76419b0

    #define OFFSET_WEAPON				0x1a14 //m_latestPrimaryWeapons
    #define OFFSET_BULLET_SPEED         0x1ef0 //CWeaponX!m_flProjectileSpeed
    #define OFFSET_BULLET_SCALE         0x1ef8 //CWeaponX!m_flProjectileScale
    #define OFFSET_ZOOM_FOV             0x16b8 + 0xb8 //m_playerData + m_curZoomFOV
    #define OFFSET_AMMO                 0x1670 //m_ammoInClip

    #define OFFSET_ITEM_GLOW            0x2c0 //m_highlightFunctionBits

    #define OFFSET_GLOW_T1              0x262 //16256 = enabled, 0 = disabled
    #define OFFSET_GLOW_T2              0x2dc //1193322764 = enabled, 0 = disabled
    #define OFFSET_GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
    #define OFFSET_GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled

#endif