#ifndef LIGHT_SHADER_H
#define LIGHT_SHADER_H

#include <raylib.h>
#include <raymath.h>

typedef struct {
	Shader shader;
	int ambient_color_loc;
	int view_pos_loc;
	int view_tar_loc;
	float ambient_color[4];
	float view_pos[3];
	float view_tar[3];
} LightShader;

void light_shader_initialize(LightShader *light_shader, const char *vert_path, const char *frag_path);
void light_shader_terminate(LightShader *light_shader);
void light_shader_update_uniforms(LightShader *light_shader, const Vector3 pos, const Vector3 tar);

#endif
