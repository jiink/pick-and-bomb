#pragma once

namespace Shaders {
    const char* playfield = R"(

#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;  // R=ID, GB=Vector (Pixels), A=Type
uniform vec4 palette[256];
uniform sampler2D cellProps; // R=HP, G=Type

out vec4 finalColor;

vec4 GetColorForCell(int id) {
    if (id < 0) return vec4(0.0); // Invalid
    vec4 props = texelFetch(cellProps, ivec2(id, 0), 0);
    float hp = props.r;
    int type = int(props.g);
    
    if (hp <= 0.0) type = 0; 
    
    vec4 col = palette[clamp(type, 0, 255)];
    return vec4(col.rgb * hp, col.a);
}

void main()
{
    // 1. Get Texture Dimensions
    ivec2 texSize = textureSize(texture0, 0);
    
    // 2. Calculate Screen-Space Metrics
    // This tells us: "How many Texture Pixels fit in one Screen Pixel?"
    // We use this to keep the AA line exactly 1-2 screen pixels wide.
    // We calculate this ONCE using UVs, which are smooth, so no grid artifacts.
    vec2 fw = fwidth(fragTexCoord * vec2(texSize));
    float pxScale = length(fw); // Approximation of zoom level
    
    // 3. Setup Integer Coordinates
    vec2 coordTexPixels = fragTexCoord * vec2(texSize);
    ivec2 centerTexel = ivec2(floor(coordTexPixels));
    
    // 4. Voronoi Search (3x3 Neighbors)
    float d1 = 1e10; // Infinity
    float d2 = 1e10;
    int id1 = -1;
    int id2 = -1;
    
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            
            ivec2 neighborCoord = centerTexel + ivec2(x, y);
            
            // texelFetch is fast and precise (no UV float errors)
            // It ignores filtering (always GL_NEAREST behavior)
            vec4 data = texelFetch(texture0, neighborCoord, 0);
            
            int nID = int(round(data.r));
            vec2 nVec = data.gb; // Stored vector: NeighborCenter -> CellCenter
            
            // Reconstruct Vector: Fragment -> NeighborCenter -> CellCenter
            // (coordTexPixels - centerTexel) gives sub-pixel offset within current texel
            // vec2(x, y) moves to the neighbor
            vec2 fragToNeighbor = vec2(x, y) - (coordTexPixels - vec2(centerTexel));
            
            // Full vector from Fragment to the actual Voronoi Point
            vec2 fragToCell = fragToNeighbor + nVec; // +0.5 logic handled in C++ generation? 
            // Note: If C++ stored vector from PixelCenter, we might need a +0.5 offset here 
            // depending on exactly how you calculated 'diff' in C++. 
            // If C++ used: diff = cellPos - samplePos; (where samplePos was center of pixel)
            // then we are good.
            
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
    
    // 5. Convert squared distance to linear distance (in Texture Pixels)
    d1 = sqrt(d1);
    d2 = sqrt(d2);
    
    // 6. Calculate Edge
    float distDiff = d2 - d1;
    
    float aaWidth = pxScale * 1.5; 
    aaWidth = max(aaWidth, 0.0001);
    
    // 0.0 = We are on the edge line
    // 1.0 = We are safely inside the cell (past the AA width)
    float edgeFactor = smoothstep(0.0, aaWidth, distDiff);
    
    // 8. Color Mixing
    vec4 color1 = GetColorForCell(id1);
    vec4 color2 = GetColorForCell(id2); 
    
    // --- THE FIX ---
    // We remap edgeFactor so that at the edge (0.0), we mix 50%.
    // At the center (1.0), we mix 100% of color1.
    float blendRatio = 0.5 + (0.5 * edgeFactor);
    
    finalColor = mix(color2, color1, blendRatio);

}

    )";
}