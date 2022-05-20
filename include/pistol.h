#ifndef PISTOL_H
#define PISTOL_H

#define PISTOL_FIRE_FRAMES		30
#define PISTOL_RELOAD_FRAMES	80
#define PISTOL_MAX_PER_MAG		25
#define PISTOL_ADS_LERP_SPEED	12.0f

#include <raylib.h>
#include <raymath.h>

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

void pistol_initialize(Pistol *pistol, const Shader shader);
void pistol_terminate(Pistol *pistol);
void pistol_reload(Pistol *pistol);
void pistol_fire(Pistol *pistol, Vector3 *recoil_dir);

#endif
