#version 420 core

#extension GL_ARB_shader_texture_image_samples : require

layout(binding=13) uniform sampler2DMS MultisampleSampler;

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	int SampleCount = textureSamples(MultisampleSampler);

  outFragColor = vec4(inColor, 1.0);
}
