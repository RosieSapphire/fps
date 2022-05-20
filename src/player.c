#include "player.h"

void player_initialize(Player *player, const int screen_width, const int screen_height) {
	player->angles = Vector2Zero();
	player->angles.x = -PI/2;
	player->cam_view.fovy = PLAYER_NORMAL_FOV;
	player->cam_view.projection = CAMERA_PERSPECTIVE;
	player->cam_view.up = (Vector3){0.0f, 1.0f, 0.0f};
	player->cam_view.position = Vector3Scale(player->cam_view.up, PLAYER_HEIGHT);
	player->cam_view.target = player->cam_view.position;
	player->cam_weapon.fovy = 52.0f;
	player->cam_weapon.projection = CAMERA_PERSPECTIVE;
	player->cam_weapon.up = player->cam_view.up;
	player->cam_weapon.position = Vector3Zero();
	player->cam_weapon.target = (Vector3){-1.0f, 0.0f, 0.0f};
	player->weapon_render = LoadRenderTexture(screen_width, screen_height);
	player->pos = player->cam_view.position;
	player->wish_pos = player->pos;
}

void player_terminate(Player *player) {
	UnloadRenderTexture(player->weapon_render);
}

void player_lerp_ads(Player *player, Pistol *pistol, bool is_scoping, const float time_delta, Color *crosshair_color) {
	float new_crosshair_opacity;
	if(is_scoping) {
		pistol->pos = Vector3Lerp(pistol->pos, pistol->ads_pos, time_delta * PISTOL_ADS_LERP_SPEED);
		player->cam_view.fovy = Lerp(player->cam_view.fovy, PLAYER_AIM_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
		player->cam_weapon.fovy = Lerp(player->cam_weapon.fovy, PLAYER_AIM_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
		new_crosshair_opacity = Lerp((float)crosshair_color->a / 255, 0.0f, time_delta * PISTOL_ADS_LERP_SPEED);
	} else {
		pistol->pos = Vector3Lerp(pistol->pos, pistol->hip_pos, time_delta * PISTOL_ADS_LERP_SPEED);
		player->cam_view.fovy = Lerp(player->cam_view.fovy, PLAYER_NORMAL_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
		player->cam_weapon.fovy = Lerp(player->cam_weapon.fovy, PLAYER_NORMAL_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
		new_crosshair_opacity = Lerp((float)crosshair_color->a / 255, 1.0f, time_delta * PISTOL_ADS_LERP_SPEED);
	}
	crosshair_color->a = (int)(new_crosshair_opacity * 255.0f);
}
