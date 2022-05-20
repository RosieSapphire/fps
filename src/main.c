#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pistol.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define SCREEN_LABEL		"Raylib Test"

#define GLSL_VERSION		330

#define TURN_SPEED			0.004f
#define MOVE_SPEED			4.0f
#define MOVE_LERP_SPEED		8.0f
#define ACCELERATION		1.2f

#define PLAYER_RADIUS		0.2f
#define PLAYER_HEIGHT		1.0f
#define HEADBOB_SCALE		0.04f
#define HEADBOB_SPEED		250.0f

#define PLAYER_NORMAL_FOV	52.0f
#define PLAYER_AIM_FOV		PLAYER_NORMAL_FOV / 2

#define RAY_COUNT			8
#define COLLISION_PUSHBACK	0.01f

#define SFX_FOOTSTEP_COUNT	10
#define SFX_BULLET_COUNT	5

#define ANIM_FRAMERATE		60.0f

#define X 0
#define Y 1
#define S 2

int main() {
	int monitor;
	int screen_width = 0;
	int screen_height = 0;
	float frames_per_second;

	Color crosshair_color;
	Color ray_colors[RAY_COUNT] = {
		RED,
		ORANGE,
		YELLOW,
		GREEN,
		BLUE,
		SKYBLUE,
		PURPLE,
		MAGENTA
	};

	float time_delta = 0.0f;
	float time_elapsed = 0.0f;

	bool view_toggle = 1;
	bool hide_toggle = 0;
	bool show_empty_room = 0;
	
	Shader shader_lights;
	int ambient_color_loc;
	float ambient_color[4];

	int view_pos_loc;
	float view_pos[3];
	int view_tar_loc;
	float view_tar[3];

	Vector2 player_angle;
	Camera player;
	Pistol pistol;

	Camera cam_view_model;
	Camera viewport;

	Model room_model_full;
	Model room_model_empty;
	Texture2D tex_room_floor;
	Texture2D tex_room_wall;
	Texture2D tex_room_ceiling;

	int ammo_text_pos[3];
	Vector3 recoil_dir;

	Mesh sphere_mesh;
	Model sphere_model;

	Color background_color;

	RenderTexture2D render_texture;
	RenderTexture2D view_model_render;
	Rectangle render_rect;

	Vector3 light_pos;
	Light light;

	Vector2 mouse_pos;
	Vector3 player_target_pos;
	Vector3 player_raw_pos;
	Vector3 player_raw_target;

	Sound sfx_footsteps[SFX_FOOTSTEP_COUNT];
	Sound sfx_bullet_bounce[SFX_BULLET_COUNT];
	int bullet_ricochet_play = 0;
	bool footstep_played = 0;

	InitWindow(screen_width, screen_height, SCREEN_LABEL);
	monitor = GetCurrentMonitor();
	screen_width = GetMonitorWidth(monitor);
	screen_height = GetMonitorHeight(monitor);
	SetWindowSize(screen_width, screen_height);

	InitAudioDevice();
	SetMasterVolume(0.5f);
	SetRandomSeed((unsigned int)time(0));
	HideCursor();

	frames_per_second = 60.0f; // GetMonitorRefreshRate(0);
	SetTargetFPS(frames_per_second);
	SetMousePosition(screen_width / 2, screen_height / 2);

	crosshair_color.r = 0xFF;
	crosshair_color.g = 0xFF;
	crosshair_color.b = 0xFF;
	crosshair_color.a = 0xFF;

	shader_lights = LoadShader("res/shaders/base_lighting_vert.glsl", "res/shaders/base_lighting_frag.glsl");
	ambient_color_loc = GetShaderLocation(shader_lights, "ambient");
	ambient_color[0] = 0x01;
	ambient_color[1] = 0x01;
	ambient_color[2] = 0x01;
	ambient_color[3] = 0xFF;
	SetShaderValue(shader_lights, ambient_color_loc, &ambient_color, SHADER_UNIFORM_VEC4);

	view_pos_loc = GetShaderLocation(shader_lights, "viewPos");
	view_tar_loc = GetShaderLocation(shader_lights, "viewTar");

	player_angle = Vector2Zero();
	player_angle.x = -PI/2;
	player.fovy = PLAYER_NORMAL_FOV;
	player.projection = CAMERA_PERSPECTIVE;
	player.up = (Vector3){0.0f, 1.0f, 0.0f};
	player.position = Vector3Scale(player.up, PLAYER_HEIGHT);
	player.target = player.position;

	cam_view_model.fovy = 52.0f;
	cam_view_model.projection = CAMERA_PERSPECTIVE;
	cam_view_model.up = player.up;
	cam_view_model.position = Vector3Zero();
	cam_view_model.target = (Vector3){-1.0f, 0.0f, 0.0f};

	viewport.fovy = 8.0f;
	viewport.position = (Vector3){0.0f, 5.0f, 0.0f};
	viewport.target = (Vector3){0.1f, 0.0f, 0.00f};
	viewport.projection = CAMERA_ORTHOGRAPHIC;
	viewport.up = player.up;

	tex_room_floor = LoadTexture("res/models/room/textures/floor/diffuse.png");
	tex_room_wall = LoadTexture("res/models/room/textures/wall/diffuse.png");
	tex_room_ceiling = LoadTexture("res/models/room/textures/ceiling/diffuse.png");

	room_model_full = LoadModel("res/models/room/room.obj");
	SetMaterialTexture(&room_model_full.materials[0], MATERIAL_MAP_DIFFUSE, tex_room_ceiling);
	SetMaterialTexture(&room_model_full.materials[1], MATERIAL_MAP_DIFFUSE, tex_room_floor);
	SetMaterialTexture(&room_model_full.materials[2], MATERIAL_MAP_DIFFUSE, tex_room_wall);
	room_model_full.materials[0].shader = shader_lights;
	room_model_full.materials[1].shader = shader_lights;
	room_model_full.materials[2].shader = shader_lights;

	room_model_empty = LoadModel("res/models/room/room-empty.obj");
	room_model_empty.materials->shader = shader_lights;

	ammo_text_pos[X] = screen_width / 64;
	ammo_text_pos[S] = screen_height / 32;
	ammo_text_pos[Y] = screen_height - 32 - ammo_text_pos[S];

	sphere_mesh = GenMeshSphere(1.0f, 32, 32);
	sphere_model = LoadModelFromMesh(sphere_mesh);

	background_color = (Color){0x01, 0x02, 0x02, 0xFF};

	render_texture = LoadRenderTexture(screen_width, screen_height);
	view_model_render = LoadRenderTexture(screen_width, screen_height);
	render_rect = (Rectangle){0.0f, screen_height, screen_width, -screen_height};

	sfx_footsteps[0] = LoadSound("res/sounds/footstep-01.wav");
	sfx_footsteps[1] = LoadSound("res/sounds/footstep-02.wav");
	sfx_footsteps[2] = LoadSound("res/sounds/footstep-03.wav");
	sfx_footsteps[3] = LoadSound("res/sounds/footstep-04.wav");
	sfx_footsteps[4] = LoadSound("res/sounds/footstep-05.wav");
	sfx_footsteps[5] = LoadSound("res/sounds/footstep-06.wav");
	sfx_footsteps[6] = LoadSound("res/sounds/footstep-07.wav");
	sfx_footsteps[7] = LoadSound("res/sounds/footstep-08.wav");
	sfx_footsteps[8] = LoadSound("res/sounds/footstep-09.wav");
	sfx_footsteps[9] = LoadSound("res/sounds/footstep-10.wav");

	sfx_bullet_bounce[0] = LoadSound("res/sounds/ricoche-01.wav");
	sfx_bullet_bounce[1] = LoadSound("res/sounds/ricoche-02.wav");
	sfx_bullet_bounce[2] = LoadSound("res/sounds/ricoche-03.wav");
	sfx_bullet_bounce[3] = LoadSound("res/sounds/ricoche-04.wav");
	sfx_bullet_bounce[4] = LoadSound("res/sounds/ricoche-05.wav");

	pistol_initialize(&pistol, shader_lights);

	light_pos = (Vector3){2.0f, 8.0f, 2.0f};
	light = CreateLight(LIGHT_POINT, light_pos, Vector3Zero(), WHITE, shader_lights);
	light.color = WHITE;
	UpdateLightValues(shader_lights, light);

	player_target_pos = player.position;
	player_raw_pos = player_target_pos;

	while(!WindowShouldClose()) {
		const int is_running = IsKeyDown(KEY_LEFT_SHIFT) + 1;
		const float sin_player_angle_x = sinf(player_angle.x);
		const float cos_player_angle_x = cosf(player_angle.x);
		RayCollision mouse_ray_collision[RAY_COUNT];
		Ray mouse_ray[RAY_COUNT];
		Vector2 move_input = Vector2Zero();
		Vector2 move_direction;
		Vector3 move_speed;
		Vector3 player_delta;
		Vector3 headbob;
		float headbob_intensity;
		float headbob_sin;
		float headbob_cos;
		move_speed = Vector3Zero();
		time_delta = GetFrameTime();
		time_elapsed += time_delta;

		headbob = Vector3Zero();

		/* input */
		mouse_pos = GetMousePosition();
		SetMousePosition(screen_width / 2, screen_height / 2);

		move_input.x += IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
		move_input.y += IsKeyDown(KEY_W) - IsKeyDown(KEY_S);

		if(IsKeyPressed(KEY_Q)) {
			view_toggle = !view_toggle;
		}

		if(IsKeyPressed(KEY_H))
			hide_toggle = !hide_toggle;

		if(IsKeyPressed(KEY_E))
			show_empty_room = !show_empty_room;

		if(IsKeyPressed(KEY_BACKSPACE)) {
			player_target_pos = player.up;
			player_angle = Vector2Zero();
			pistol.ammo_loaded = PISTOL_MAX_PER_MAG;
			pistol.ammo_reserve = 50;
			viewport.fovy = 8.0f;
			viewport.position = (Vector3){0.0f, 5.0f, 0.0f};
			viewport.target = (Vector3){0.0f, 0.0f, 0.01f};
		}

		if(IsKeyPressed(KEY_R)) {
			if(pistol.ammo_loaded < PISTOL_MAX_PER_MAG && pistol.reload_frame <= 0.0f) {
				if(pistol.ammo_reserve > 0) {
					pistol_reload(&pistol);
				} else {
					PlaySound(pistol.sfx_click);
				}
			}
		}

		float new_crosshair_opacity;
		if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && pistol.reload_frame <= 0.0f && !(is_running - 1)) {
			pistol.pos = Vector3Lerp(pistol.pos, pistol.ads_pos, time_delta * PISTOL_ADS_LERP_SPEED);
			player.fovy = Lerp(player.fovy, PLAYER_AIM_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
			cam_view_model.fovy = Lerp(cam_view_model.fovy, PLAYER_AIM_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
			new_crosshair_opacity = Lerp((float)crosshair_color.a / 255, 0.0f, time_delta * PISTOL_ADS_LERP_SPEED);
		} else {
			pistol.pos = Vector3Lerp(pistol.pos, pistol.hip_pos, time_delta * PISTOL_ADS_LERP_SPEED);
			player.fovy = Lerp(player.fovy, PLAYER_NORMAL_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
			cam_view_model.fovy = Lerp(cam_view_model.fovy, PLAYER_NORMAL_FOV, time_delta * PISTOL_ADS_LERP_SPEED);
			new_crosshair_opacity = Lerp((float)crosshair_color.a / 255, 1.0f, time_delta * PISTOL_ADS_LERP_SPEED);
		}
		crosshair_color.a = (int)(new_crosshair_opacity * 255.0f);

		if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			if(pistol.fire_frame <= PISTOL_FIRE_FRAMES * 0.75f
			&& pistol.reload_frame <= 0.0f) {
				if(pistol.ammo_loaded > 0) {
					Ray gun_ray;
					RayCollision gun_ray_collision;
					float bullet_ricochet_volume;
					int last_bullet_sfx_play = bullet_ricochet_play;

					pistol_fire(&pistol, &recoil_dir);

					do { bullet_ricochet_play = GetRandomValue(0, 4);
					} while(bullet_ricochet_play == last_bullet_sfx_play);

					gun_ray.position = player_raw_pos;
					gun_ray.direction = Vector3Subtract(player.target, player.position);
					gun_ray_collision = GetRayCollisionModel(gun_ray, room_model_full);
					bullet_ricochet_volume = 1 / Vector3Distance(player.position, gun_ray_collision.point);
					bullet_ricochet_volume = Clamp(bullet_ricochet_volume, 0.0f, 1.0f);
					SetSoundVolume(sfx_bullet_bounce[bullet_ricochet_play], bullet_ricochet_volume);
					PlaySound(sfx_bullet_bounce[bullet_ricochet_play]);
				} else {
					if(pistol.ammo_reserve > 0) {
						pistol_reload(&pistol);
					} else {
						PlaySound(pistol.sfx_click);
					}
				}
			}
		}

		/* updating */
		view_pos[0] = player.position.x;
		view_pos[1] = player.position.y;
		view_pos[2] = player.position.z;
		SetShaderValue(shader_lights, view_pos_loc, &view_pos, SHADER_UNIFORM_VEC3);

		view_tar[0] = player.target.x;
		view_tar[1] = player.target.y;
		view_tar[2] = player.target.z;
		SetShaderValue(shader_lights, view_tar_loc, &view_tar, SHADER_UNIFORM_VEC3);

		recoil_dir = Vector3Lerp(recoil_dir, Vector3Zero(), time_delta * PISTOL_ADS_LERP_SPEED);

		pistol.fire_frame -= time_delta * ANIM_FRAMERATE;
		pistol.fire_frame = Clamp(pistol.fire_frame, 0.0f, PISTOL_FIRE_FRAMES);

		UpdateModelAnimation(pistol.model, pistol.anims[1],
				PISTOL_FIRE_FRAMES - (int)pistol.fire_frame);

		/* TODO: add Scout's reload as a 1/100 chance every time you reload */
		pistol.reload_frame -= time_delta * ANIM_FRAMERATE;
		pistol.reload_frame = Clamp(pistol.reload_frame, 0.0f, PISTOL_RELOAD_FRAMES);
		if(pistol.reload_frame > 0.0f) {
			if(pistol.reload_frame < 64 && pistol.reload_sfx_play < 1) {
				PlaySound(pistol.sfx_eject);
				pistol.reload_sfx_play++;
			} else if(pistol.reload_frame < 40 && pistol.reload_sfx_play < 2) {
				PlaySound(pistol.sfx_load);
				pistol.reload_sfx_play++;
			}

			UpdateModelAnimation(pistol.model,
					pistol.anims[2], PISTOL_RELOAD_FRAMES - (int)pistol.reload_frame);
		}

		Vector2 player_angle_delta;
		player_angle_delta.x = (mouse_pos.x - ((float)screen_width / 2)) * TURN_SPEED;
		player_angle_delta.y = -((mouse_pos.y - ((float)screen_height / 2)) * TURN_SPEED);
		player_angle = Vector2Add(player_angle, player_angle_delta);
		player_angle.y = Clamp(player_angle.y, -1.0f, 1.0f);

		move_direction.x = cos_player_angle_x * move_input.y;
		move_direction.x += -sin_player_angle_x * move_input.x;
		move_direction.y = sin_player_angle_x * move_input.y;
		move_direction.y += cos_player_angle_x * move_input.x;
		move_direction = Vector2Normalize(move_direction);

		move_speed.x = move_direction.x * MOVE_SPEED * is_running * time_delta;
		move_speed.z = move_direction.y * MOVE_SPEED * is_running * time_delta;

		int i;
		float angle_offset;
		Vector3 wall_test_points[RAY_COUNT] = {Vector3Zero()};
		int ignore_raycast[RAY_COUNT];

		Vector3 raw_pos_push;
		Vector3 target_pos_push;

		Vector3 player_raw_pos_old = player_raw_pos;

		player_target_pos = Vector3Add(player_target_pos, move_speed);
		player_raw_pos = Vector3Lerp(player_raw_pos, player_target_pos, time_delta * MOVE_LERP_SPEED);

		raw_pos_push = player_raw_pos;
		target_pos_push = player_target_pos;

		/* TODO: Fix the janky fucking collision */
		for(i = 0; i < RAY_COUNT; i++) {
			angle_offset = i * (PI * (0.5f / (int)(RAY_COUNT / 4)));
			mouse_ray[i].position = player_raw_pos;
			mouse_ray[i].direction = (Vector3){cosf(player_angle.x + angle_offset), 0.0f,
												sinf(player_angle.x + angle_offset)};

			if(show_empty_room)
				mouse_ray_collision[i] = GetRayCollisionMesh(mouse_ray[i], *room_model_empty.meshes, MatrixIdentity());
			else 
				mouse_ray_collision[i] = GetRayCollisionMesh(mouse_ray[i], *room_model_full.meshes, MatrixIdentity());

			if(mouse_ray_collision[i].normal.z < 0.0f || mouse_ray_collision[i].normal.x < 0.0f) {
				wall_test_points[i] = Vector3Subtract(target_pos_push, Vector3Multiply(mouse_ray_collision[i].normal,
					Vector3Subtract(mouse_ray_collision[i].point, target_pos_push)));
			} else {
				wall_test_points[i] = Vector3Subtract(target_pos_push, Vector3Multiply(mouse_ray_collision[i].normal,
					Vector3Subtract(target_pos_push, mouse_ray_collision[i].point)));
			}

			ignore_raycast[i] = Vector3Distance(target_pos_push, wall_test_points[i]) > PLAYER_RADIUS * 2;
			if(ignore_raycast[i]) continue;

			/* while(Vector3Distance(target_pos_push, wall_test_points[i]) < PLAYER_RADIUS)
				target_pos_push = Vector3Add(target_pos_push, Vector3Scale(mouse_ray_collision[i].normal, time_delta)); */
		}

		player_raw_pos = raw_pos_push;
		player_target_pos = target_pos_push;

		player_delta = Vector3Subtract(player_raw_pos_old, player_raw_pos);

		headbob_sin = sinf(time_elapsed * 16.0f * ((float)is_running * 0.75f));
		headbob_cos = cosf(time_elapsed * 8.0f * ((float)is_running * 0.75f));
		headbob_intensity = (fabsf(player_delta.x * 24.0f) + fabsf(player_delta.z * 24.0f));
		headbob.x = sin_player_angle_x * headbob_cos;
		headbob.y = headbob_sin;
		headbob.z = -cos_player_angle_x * headbob_cos;
		headbob = Vector3Scale(headbob, HEADBOB_SCALE * headbob_intensity);

		player_raw_target = (Vector3){cos_player_angle_x, player_angle.y, sin_player_angle_x};
		player_raw_target = Vector3Normalize(player_raw_target);
		player_raw_target = Vector3Multiply(player_raw_target, (Vector3){PLAYER_RADIUS, PLAYER_RADIUS, PLAYER_RADIUS});
		player_raw_target = Vector3Add(player_raw_target, player_raw_pos);
		player.target = Vector3Add(player_raw_target, headbob);
		player.target = Vector3Add(player.target, recoil_dir);
		player.position = Vector3Add(player_raw_pos, headbob);

		/* 3rd-person camera controls */
		viewport.fovy += (IsKeyDown(KEY_RIGHT_SHIFT) * time_delta * 4.0f) -
						(IsKeyDown(KEY_RIGHT_CONTROL) * time_delta * 4.0f);
		
		viewport.position.x += (IsKeyDown(KEY_LEFT) * time_delta * 4.0f) -
						(IsKeyDown(KEY_RIGHT) * time_delta * 4.0f);
		viewport.position.z += (IsKeyDown(KEY_UP) * time_delta * 4.0f) -
						(IsKeyDown(KEY_DOWN) * time_delta * 4.0f);

		viewport.target = Vector3Add(viewport.position, (Vector3){0.0f, -1.0f, 0.01f});

		if(fabsf(headbob_cos) > 0.9f) {
			Sound footstep_cur = sfx_footsteps[GetRandomValue(0, SFX_FOOTSTEP_COUNT - 1)];
			float clamped_volume = Clamp(headbob_intensity * 1.5f * (headbob_intensity > 0.1f), 0.0f, 1.0f);
			SetSoundVolume(footstep_cur, clamped_volume);

			if(!footstep_played) {
				PlaySound(footstep_cur);
				footstep_played = 1;
			}
		} else {
			footstep_played = 0;
		}

		/* drawing */
		BeginTextureMode(view_model_render); {
			BeginMode3D(cam_view_model); {
				ClearBackground((Color){0xFF, 0xFF, 0xFF, 0x00});
				DrawModel(pistol.model,
					Vector3Add(pistol.pos,
					Vector3Scale((Vector3){0.0f, headbob_sin * 0.5f, -headbob_cos},
					headbob_intensity * HEADBOB_SCALE * 0.2f)), 1.0f, pistol.color);
			} EndMode3D();
		} EndTextureMode();

		BeginTextureMode(render_texture); {
			ClearBackground(background_color);
			if(view_toggle) {
				BeginMode3D(player); {
					if(show_empty_room)
						DrawModel(room_model_empty, Vector3Zero(), 1.0f, DARKGREEN);
					else
						DrawModel(room_model_full, Vector3Zero(), 1.0f, WHITE);

				} EndMode3D();

			} else {
				BeginMode3D(viewport); {
					if(show_empty_room)
						DrawModel(room_model_empty, Vector3Zero(), 1.0f, DARKGREEN);
					else
						DrawModel(room_model_full, Vector3Zero(), 1.0f, WHITE);

					DrawModel(sphere_model, player_raw_pos, 0.01f, PINK);
					DrawSphere(player.position, 0.005f, RED);
					DrawSphere(player_target_pos, 0.02f, WHITE);
					DrawModel(pistol.model, pistol.pos, 1.0f, pistol.color);

					DrawRay((Ray){player_raw_pos, (Vector3){cos_player_angle_x * 0.2f, 1.0f, sin_player_angle_x * 0.2f}},
					RAYWHITE);

					for(i = 0; i < RAY_COUNT; i++) {
						if(ignore_raycast[i]) continue;
						DrawLine3D(player_target_pos, wall_test_points[i], ray_colors[i]);
						DrawLine3D(player_target_pos, mouse_ray_collision[i].point, ray_colors[i]);
						DrawSphere(wall_test_points[i], 0.04f, ray_colors[i]);
					}

					if(!hide_toggle)
						DrawCylinder(Vector3Multiply(player_raw_pos, (Vector3){1.0f, 0.0f, 1.0f}),
							PLAYER_RADIUS, PLAYER_RADIUS, PLAYER_HEIGHT, 12, (Color){0xFF, 0x97, 0x00, 0x80});

					/* axis lines */
					/* DrawLine3D(player_raw_pos, Vector3Add(player_raw_pos, (Vector3){0.2f, 0.0f, 0.0f}), RED);
					DrawLine3D(player_raw_pos, Vector3Add(player_raw_pos, (Vector3){0.0f, 0.2f, 0.0f}), GREEN);
					DrawLine3D(player_raw_pos, Vector3Add(player_raw_pos, (Vector3){0.0f, 0.0f, 0.2f}), BLUE); */

				} EndMode3D();
			}
		} EndTextureMode();

		BeginDrawing(); {
			DrawTextureRec(render_texture.texture, render_rect, Vector2Zero(), WHITE);
			if(view_toggle) {
				DrawCircle(screen_width / 2, screen_height / 2, 1.0f, crosshair_color); /* crosshair */
				DrawTextureRec(view_model_render.texture, render_rect, Vector2Zero(), WHITE);
			}

			DrawFPS(16, 16);

			/* pistol ammo counter */
			DrawText(TextFormat("Ammo: %d/%d", pistol.ammo_loaded, pistol.ammo_reserve),
				ammo_text_pos[X], ammo_text_pos[Y], ammo_text_pos[S], WHITE);
		} EndDrawing();
	}

	UnloadTexture(tex_room_floor);
	UnloadTexture(tex_room_wall);
	UnloadTexture(tex_room_ceiling);
	UnloadModel(room_model_full);
	UnloadModel(room_model_empty);
	UnloadModel(sphere_model);

	pistol_terminate(&pistol);

	/* unloading sounds */
	const Sound *cur = sfx_footsteps;
	const Sound *end = cur + SFX_FOOTSTEP_COUNT;
	while(cur != end) {
		UnloadSound(*cur);
		cur++;
	}

	cur = sfx_bullet_bounce;
	end = cur + SFX_BULLET_COUNT;
	while(cur != end) {
		UnloadSound(*cur);
		cur++;
	}

	CloseAudioDevice();
	CloseWindow();

	return 0;
}
