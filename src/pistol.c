#include "pistol.h"

void pistol_initialize(Pistol *pistol, const Shader shader) {
	pistol->anim_count = 2;
	pistol->fire_frame = 0.0f;
	pistol->reload_frame = 0.0f;
	pistol->reload_sfx_play = 0;
	pistol->ammo_loaded = PISTOL_MAX_PER_MAG;
	pistol->ammo_reserve = 50;

	pistol->color.r = 0x20;
	pistol->color.g = 0x20;
	pistol->color.b = 0x22;
	pistol->color.a = 0xFF;

	pistol->ads_pos.x = -0.62f;
	pistol->ads_pos.y = -0.096f; 
	pistol->ads_pos.z = 0.0f;
	pistol->hip_pos.x = -0.62f;
	pistol->hip_pos.y = -0.25f;
	pistol->hip_pos.z = -0.21f;
	pistol->pos = pistol->hip_pos;

	pistol->model = LoadModel("res/models/weapons/pistol/pistol.iqm");
	pistol->model.materials->shader = shader;
	pistol->model.materials->maps->color = pistol->color;
	pistol->anims = LoadModelAnimations("res/models/weapons/pistol/pistol.iqm", &pistol->anim_count);
	pistol->model.transform = MatrixMultiply(pistol->model.transform, MatrixRotateX(PI/2));

	pistol->sfx_fire = LoadSound("res/sounds/pistol-fire.wav");
	pistol->sfx_click = LoadSound("res/sounds/pistol-click.wav");
	pistol->sfx_eject = LoadSound("res/sounds/pistol-eject.wav");
	pistol->sfx_load = LoadSound("res/sounds/pistol-load.wav");
	SetSoundVolume(pistol->sfx_fire, 0.7f);
	SetSoundVolume(pistol->sfx_click, 0.45f);
	SetSoundVolume(pistol->sfx_eject, 0.18f);
	SetSoundVolume(pistol->sfx_load, 0.62f);
}

void pistol_terminate(Pistol *pistol) {
	UnloadModel(pistol->model);
	UnloadModelAnimations(pistol->anims, pistol->anim_count);
	UnloadSound(pistol->sfx_fire);
	UnloadSound(pistol->sfx_click);
	UnloadSound(pistol->sfx_eject);
	UnloadSound(pistol->sfx_load);
}

void pistol_reload(Pistol *pistol) {
	pistol->reload_frame = PISTOL_RELOAD_FRAMES;
	pistol->reload_sfx_play = 0;
	const int ammo_exchange = PISTOL_MAX_PER_MAG - pistol->ammo_loaded;
	if(pistol->ammo_reserve - ammo_exchange >= 0) {
		pistol->ammo_reserve -= ammo_exchange;
		pistol->ammo_loaded += ammo_exchange;
	} else {
		pistol->ammo_loaded += pistol->ammo_reserve;
		pistol->ammo_reserve = 0;
	}
}

void pistol_fire(Pistol *pistol, Vector3 *recoil_dir) {
	/* play bullet fire sound */
	SetSoundPitch(pistol->sfx_fire,
			1.0f + ((float)GetRandomValue(-1, 1) / 32));
	PlaySound(pistol->sfx_fire);
	pistol->fire_frame = PISTOL_FIRE_FRAMES;

	*recoil_dir = (Vector3) {
		(float)GetRandomValue(-1, 1),
		(float)GetRandomValue(2, 4),
		(float)GetRandomValue(-1, 1),
	};
	*recoil_dir = Vector3Scale(*recoil_dir, 0.004f);

	pistol->ammo_loaded--;
}
