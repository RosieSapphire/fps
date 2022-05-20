#include "light_shader.h"

void light_shader_initialize(LightShader *light_shader, const char *vert_path, const char *frag_path) {
	light_shader->shader = LoadShader(vert_path, frag_path);
	light_shader->ambient_color_loc = GetShaderLocation(light_shader->shader, "ambient");
	light_shader->ambient_color[0] = 0x01;
	light_shader->ambient_color[1] = 0x01;
	light_shader->ambient_color[2] = 0x01;
	light_shader->ambient_color[3] = 0xFF;
	SetShaderValue(light_shader->shader, light_shader->ambient_color_loc,
				light_shader->ambient_color, SHADER_UNIFORM_VEC4);

	light_shader->view_pos_loc = GetShaderLocation(light_shader->shader, "viewPos");
	light_shader->view_tar_loc = GetShaderLocation(light_shader->shader, "viewTar");
}

void light_shader_terminate(LightShader *light_shader) {
	 UnloadShader(light_shader->shader);
}

void light_shader_update_uniforms(LightShader *light_shader, const Vector3 pos, const Vector3 tar) {
	light_shader->view_pos[0] = pos.x;
	light_shader->view_pos[1] = pos.y;
	light_shader->view_pos[2] = pos.z;
	SetShaderValue(light_shader->shader, light_shader->view_pos_loc, light_shader->view_pos, SHADER_UNIFORM_VEC3);
	light_shader->view_tar[0] = tar.x;
	light_shader->view_tar[1] = tar.y;
	light_shader->view_tar[2] = tar.z;
	SetShaderValue(light_shader->shader, light_shader->view_tar_loc, light_shader->view_tar, SHADER_UNIFORM_VEC3);
}
