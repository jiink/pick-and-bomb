#pragma once

namespace Shaders {
    const char* playfield = R"(

#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

uniform vec4 palette[256];

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    int cellType = int(texel.r);
    int paletteIdx = clamp(cellType, 0, 255);
    finalColor = palette[paletteIdx];
}
    )";
}