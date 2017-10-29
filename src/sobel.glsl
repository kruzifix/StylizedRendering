#version 330 core

in vec2 uv;

out vec4 fragment;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform vec3 lightDirection = vec3(-1, -1, 1);
uniform float zNear = 0.1;
uniform float zFar = 100.0;

uniform float fogStart = 90.0;
uniform float fogEnd = 100.0;

// depthSample from depthTexture.r, for instance
float linearDepth()
{
    float depthSample = texture(depthTexture, uv).r;
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

float transformColor(vec3 color)
{
    vec3 conv = vec3(0.2126, 0.7152, 0.0722);
    return dot(color, conv);
}

float getSample(sampler2D sampler, vec2 offset)
{
    vec2 uvStep = vec2(1.0) / textureSize(sampler, 0);
    return transformColor(texture2D(sampler, uv + uvStep * offset).rgb);
}

float detectEdge(sampler2D sampler)
{
    float ul = getSample(sampler, vec2(-1, 1));
    float u = getSample(sampler, vec2(0, 1));
    float ur = getSample(sampler, vec2(1, 1));

    float dl = getSample(sampler, vec2(-1, -1));
    float d = getSample(sampler, vec2(0, -1));
    float dr = getSample(sampler, vec2(1, -1));
    
    float l = getSample(sampler, vec2(-1, 0));
    float m = getSample(sampler, vec2(0, 0));
    float r = getSample(sampler, vec2(1, 0));

    float gx = ul + 2.0 * l + dl - ur - 2.0 * r - dr;
    float gy = ul + 2.0 * u + ur - dl - 2.0 * d - dr;

    return sqrt(gx * gx + gy * gy);
}

void main()
{
    vec3 N = normalize(texture2D(normalTexture, uv).rgb * 2.0 - 1.0);

    float colorEdge = detectEdge(colorTexture);
    float normalEdge = detectEdge(normalTexture);
    float edge = step(0.05, (colorEdge + normalEdge) * 2.0);

    vec3 leftBot = vec3(1, 0.9, 0.4);
    vec3 rightTop = vec3(0.9, 0.7, 1);
    float luv = length(uv);
    vec3 mixColor = mix(leftBot, rightTop, luv * luv);
    vec3 faceColor = mix(vec3(1.0), mixColor, abs(dot(N, vec3(0, 1, 0))));

    vec3 edgeColor = vec3(0.2, 0, 0.1);

    vec3 L = -normalize(lightDirection);
    float diffuse = max(0, dot(N, L));
    float shadow = 0.7 + diffuse * 0.3;

    float depth = linearDepth();
    vec3 fogColor = mixColor * 0.2;

    vec3 finalColor = mix(faceColor, edgeColor, edge) * shadow;

    fragment = vec4(mix(finalColor, fogColor, smoothstep(fogStart, fogEnd, depth)), 1);
    //fragment = vec4(vec3(getSample(depthTexture, vec2(0))), 1);
}