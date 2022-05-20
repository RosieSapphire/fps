#ifndef PISTOL_H
#define PISTOL_H

#define PISTOL_FIRE_FRAMES		30
#define PISTOL_RELOAD_FRAMES	80
#define PISTOL_MAX_PER_MAG		25
#define PISTOL_ADS_LERP_SPEED	12.0f
#define ANIM_FRAMERATE		60.0f

#include "light_shader.h"

typedef struct {
	Color color;
	Vector3 pos;
	Vector3 ads_pos;
	Vector3 hip_pos;

	Model model;
	ModelAnimation *anims;

	unsigned int anim_count;
	float fire_frame;
	float reload_frame;
	int reload_sfx_play;

	int ammo_loaded;
	int ammo_reserve;

	Sound sfx_fire;
	Sound sfx_click;
	Sound sfx_eject;
	Sound sfx_load;
} Pistol;

void pistol_initialize(Pistol *pistol, const LightShader light_shader);
void pistol_terminate(Pistol *pistol);
void pistol_reload(Pistol *pistol);
void pistol_fire(Pistol *pistol, Vector3 *recoil_dir);
void pistol_update_anim_reload(Pistol *pistol, const float time_delta, const float rate);
void pistol_update_anim_fire(Pistol *pistol, const float time_delta, const float rate);
RayCollision pistol_get_wall_collision(const Vector3 pos, const Vector3 dir, const Model room_model);

#endif
