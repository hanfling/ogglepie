#version 460 core

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	const vec3 vPos[3] =
	{
		vec3( 1.0,-1.0, 0.0 ),
		vec3(-1.0,-1.0, 0.0 ),
		vec3( 0.0, 1.0, 0.0 )
	};

	const vec3 inPos = vPos[gl_VertexID%3];

	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
