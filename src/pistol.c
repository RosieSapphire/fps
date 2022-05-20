#include "pistol.h"

void pistol_initialize(Pistol *pistol, const LightShader light_shader) {
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
	pistol->model.materials->shader = light_shader.shader;
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
	SetSoundPitch(pistol->sfx_fire,
			1.0f + ((float)GetRandomValue(-1, 1) / 32));
	PlaySound(pistol->sfx_fire);
	pistol->fire_frame = PISTOL_FIRE_FRAMES;
	pistol->ammo_loaded--;

	*recoil_dir = (Vector3) {
		(float)GetRandomValue(-1, 1),
		(float)GetRandomValue(2, 4),
		(float)GetRandomValue(-1, 1),
	};
	*recoil_dir = Vector3Scale(*recoil_dir, 0.004f);
}

void pistol_update_anim_reload(Pistol *pistol, const float time_delta, const float rate) {
	/* TODO: add Scout's reload as a 1/100 chance every time you reload */
	pistol->reload_frame -= time_delta * rate;
	pistol->reload_frame = Clamp(pistol->reload_frame, 0.0f, PISTOL_RELOAD_FRAMES);
	if(pistol->reload_frame > 0.0f) {
		if(pistol->reload_frame < 64 && pistol->reload_sfx_play < 1) {
			PlaySound(pistol->sfx_eject);
			pistol->reload_sfx_play++;
		} else if(pistol->reload_frame < 40 && pistol->reload_sfx_play < 2) {
			PlaySound(pistol->sfx_load);
			pistol->reload_sfx_play++;
		}

		UpdateModelAnimation(pistol->model,
				pistol->anims[2], PISTOL_RELOAD_FRAMES - (int)pistol->reload_frame);
	}
}

void pistol_update_anim_fire(Pistol *pistol, const float time_delta, const float rate) {
	pistol->fire_frame -= time_delta * rate;
	pistol->fire_frame = Clamp(pistol->fire_frame, 0.0f, PISTOL_FIRE_FRAMES);
	UpdateModelAnimation(pistol->model, pistol->anims[1], PISTOL_FIRE_FRAMES - (int)pistol->fire_frame);
}

RayCollision pistol_get_wall_collision(const Vector3 pos, const Vector3 dir, const Model room_model) {
	Ray gun_ray;
	gun_ray.position = pos;
	gun_ray.direction = dir;
	return GetRayCollisionModel(gun_ray, room_model);
}
