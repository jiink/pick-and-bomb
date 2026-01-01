#pragma once

namespace Shaders {
    const char* playfield = R"(

#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    finalColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.5, 1.0);
}

    )";
}