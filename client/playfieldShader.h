#pragma once

namespace Shaders {
    const char* playfield = R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 palette[256];
uniform sampler2D cellProps;

uniform int renderMode; 

out vec4 finalColor;

vec4 GetColorForCell(int id) {
    if (id < 0) return vec4(0.0);
    vec4 props = texelFetch(cellProps, ivec2(id, 0), 0);
    float hp = props.r;
    int type = int(props.g);
    if (hp <= 0.0) type = 0; 
    vec4 col = palette[clamp(type, 0, 255)];
    return vec4(col.rgb * hp, col.a);
}

void main()
{
    // --- MODE 2: RAW PIXELS ---
    if (renderMode == 2) {
        ivec2 texSize = textureSize(texture0, 0);
        ivec2 coord = ivec2(floor(fragTexCoord * vec2(texSize)));
        float id = texelFetch(texture0, coord, 0).r;
        finalColor = GetColorForCell(int(round(id)));
        return; 
    }

    // --- MODES 0 & 1: VECTOR RECONSTRUCTION ---

    ivec2 texSize = textureSize(texture0, 0);
    vec2 fw = fwidth(fragTexCoord * vec2(texSize));
    float pxScale = length(fw); 
    
    vec2 coordTexPixels = fragTexCoord * vec2(texSize);
    ivec2 centerTexel = ivec2(floor(coordTexPixels));
    
    float d1 = 1e10; 
    float d2 = 1e10;
    int id1 = -1;
    int id2 = -1;
    
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            ivec2 neighborCoord = centerTexel + ivec2(x, y);
            vec4 data = texelFetch(texture0, neighborCoord, 0);
            
            int nID = int(round(data.r));
            vec2 nVec = data.gb; 
            
            vec2 fragToNeighbor = vec2(x, y) - (coordTexPixels - vec2(centerTexel));
            vec2 fragToCell = fragToNeighbor + nVec; 
            
            float distSq = dot(fragToCell, fragToCell);
            
            if (distSq < d1) {
                if (nID != id1) {
                    d2 = d1;
                    id2 = id1;
                }
                d1 = distSq;
                id1 = nID;
            } else if (distSq < d2 && nID != id1) {
                d2 = distSq;
                id2 = nID;
            }
        }
    }
    
    d1 = sqrt(d1);
    d2 = sqrt(d2);
    
    vec4 color1 = GetColorForCell(id1);

    if (renderMode == 1) { // Trying to AA the edges
        vec4 color2 = GetColorForCell(id2);
        float aaWidth = pxScale * 1.5; 
        aaWidth = max(aaWidth, 0.0001);
        float edgeFactor = smoothstep(0.0, aaWidth, d2 - d1);
        float blendRatio = 0.5 + (0.5 * edgeFactor);
        finalColor = mix(color2, color1, blendRatio);
    } else { // Aliased edges
        finalColor = color1;
    }
}

    )";
}