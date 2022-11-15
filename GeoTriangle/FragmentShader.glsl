#version 460 core

layout (location = 0) out vec4 outFragColor;

void main() 
{
	const vec4 MistyRose = vec4( 1.0, 0.9, 0.9, 1.0 );

	outFragColor = MistyRose;
}
