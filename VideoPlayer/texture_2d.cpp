#include "texture_2d.h"
#include <iostream>

texture_2d::texture_2d() : width{0}, height{0}
{
	glGenTextures(1, &texture_id);

	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unbind();
}

void texture_2d::load_texture(uint8_t *texture_data, int width, int height)
{
	bind();
	glTexImage2D(GL_TEXTURE_2D, 
				 0, 
				 GL_RGB, 
				 width, 
				 height, 
				 0, 
				 GL_RGBA,
				 GL_UNSIGNED_BYTE, 
				 texture_data);
	unbind();

	delete[] texture_data;
}

void texture_2d::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_2d::bind() const
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

