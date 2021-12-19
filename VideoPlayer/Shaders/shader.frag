#version 430 core

out vec4 FragColor;

in vec2 tex_coords;

uniform int is_texture_bound;
uniform sampler2D texture1;

void main()
{
	vec4 color;
	if (is_texture_bound == 1) color = texture(texture1, tex_coords);
	else color = vec4(1.0, 0.0, 0.0, 1.0);
	FragColor = color;
}