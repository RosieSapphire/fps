#ifndef PLAYER_H
#define PLAYER_H

#include "pistol.h"

#define PLAYER_TURN_SPEED		0.004f
#define PLAYER_MOVE_SPEED		4.0f
#define PLAYER_LERP_SPEED		8.0f
#define PLAYER_ACCELERATION		1.2f
#define PLAYER_RADIUS			0.2f
#define PLAYER_HEIGHT			1.0f
#define PLAYER_NORMAL_FOV		52.0f
#define PLAYER_AIM_FOV			PLAYER_NORMAL_FOV / 2

typedef struct {
	Vector2 angles;
	Camera cam_view;
	Camera cam_weapon;
	RenderTexture2D weapon_render;

	Vector3 pos;
	Vector3 wish_pos;
	Vector3 view_target;
} Player;

void player_initialize(Player *player, const int screen_width, const int screen_height);
void player_terminate(Player *player);

#endif
