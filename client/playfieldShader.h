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

uniform sampler2D cellProps;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    int cellType = int(texel.r);
    int paletteIdx = clamp(cellType, 0, 255);
    vec4 cellTypeCol = palette[paletteIdx];
    int cellId = int(texel.g);
    vec4 cellP = texelFetch(cellProps, ivec2(cellId, 0), 0);
    float cellHp = cellP.r;
    vec3 solidColor = cellTypeCol.rgb * cellHp;
    finalColor = vec4(solidColor, cellTypeCol.a);
}
    )";
}