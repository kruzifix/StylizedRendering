#version 330 core

in vec3 normal;

layout (location = 0) out vec4 colorFragment;
layout (location = 1) out vec4 normalFragment;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

void main()
{
    colorFragment = vec4(Kd, 1);
    vec3 N = normalize(normal);
    normalFragment = vec4(N * 0.5 + 0.5, 1);
}