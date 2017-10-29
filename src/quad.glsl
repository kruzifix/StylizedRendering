#version 330 core

const vec2 position[4] = vec2[]
(
  vec2(-1.0,  1.0),
  vec2(-1.0, -1.0),
  vec2( 1.0,  1.0),
  vec2( 1.0, -1.0)
);

out vec2 uv;

void main()
{
    uv = position[gl_VertexID] * 0.5 + 0.5;
    gl_Position = vec4(position[gl_VertexID], 0.0, 1.0);
}