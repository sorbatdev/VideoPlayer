#pragma once

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <iostream>

class shader_manager
{
private:
	static const unsigned short MAX_SHADERS = 1;
	unsigned short current_index;
	GLuint program_ids[MAX_SHADERS];

	static shader_manager &instance;

	static shader_manager &create()
	{
		static shader_manager sm;

		return sm;
	}

	shader_manager();

	static bool _is_successful(GLuint id, GLuint action);
public:

	static shader_manager &get_instance()
	{
		return instance;
	}

	void load(const char *vert_file, const char *frag_file);
	void use(unsigned short program_id_index);

	void set_int(const char* name, int value);
	void set_mat4(const char *name, float *value);
};
