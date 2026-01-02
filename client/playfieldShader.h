#pragma once

namespace Shaders {
    const char* playfield = R"(

#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;  // R=ID, GB=Vector to Center (Pixels), A=Type
uniform vec4 palette[256];
uniform sampler2D cellProps; // R=HP

out vec4 finalColor;

// Helper to get Cell Data from a specific ID
vec4 GetColorForCell(int id, int type) {
    if (id < 0) return vec4(0.0);
    vec4 props = texelFetch(cellProps, ivec2(id, 0), 0);
    float hp = props.r;
    if (hp <= 0.0) type = 0; // Dead
    vec4 col = palette[clamp(type, 0, 255)];
    return vec4(col.rgb * hp, col.a);
}

void main()
{
    // Texture size info
    ivec2 texSize = textureSize(texture0, 0);
    
    // 1. Calculate the center of the current texel in UV space
    vec2 texelSize = 1.0 / vec2(texSize);
    vec2 coordPixels = fragTexCoord * vec2(texSize);
    vec2 centerPixel = floor(coordPixels) + 0.5;
    
    // 2. We need to find the 2 closest cells.
    // We search the 3x3 neighborhood around the current texel.
    
    float d1 = 99999.0;
    float d2 = 99999.0;
    int id1 = -1;
    int id2 = -1;
    int type1 = 0;
    
    // Loop -1 to +1 in x and y
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            
            // Sample neighbor
            vec2 offset = vec2(float(x), float(y));
            vec2 uv = (centerPixel + offset) * texelSize;
            
            vec4 data = texture(texture0, uv);
            
            int neighborID = int(round(data.r));
            vec2 neighborVec = data.gb; // Vector from Center of Neighbor -> Cell Center
            
            // Reconstruct the vector from OUR FRAGMENT to that Cell Center.
            // 1. Vector from Fragment to Center of Neighbor Pixel:
            vec2 fragToNeighborCenter = (centerPixel + offset) - coordPixels;
            
            // 2. Add the stored vector (Neighbor Center -> Cell Center)
            vec2 fragToCell = fragToNeighborCenter + neighborVec;
            
            float distSq = dot(fragToCell, fragToCell);
            
            // Standard Insertion Sort for Top 2
            if (distSq < d1) {
                // If this is the same cell ID we already found, just update dist
                if (neighborID != id1) {
                    d2 = d1;
                    id2 = id1;
                }
                d1 = distSq;
                id1 = neighborID;
                type1 = int(data.a);
            } else if (distSq < d2 && neighborID != id1) {
                d2 = distSq;
                id2 = neighborID;
            }
        }
    }
    
    // 3. Convert to linear distance for AA calculation
    d1 = sqrt(d1);
    d2 = sqrt(d2);
    
    // 4. Smooth Edge Logic
    float distDiff = d2 - d1;
    
    // fwidth now works perfectly because d1/d2 are mathematically continuous per-pixel
    float aaWidth = fwidth(distDiff);
    aaWidth = max(aaWidth, 0.001); 
    
    float edgeFactor = smoothstep(0.0, aaWidth * 1.5, distDiff);
    
    // 5. Colors
    vec4 color1 = GetColorForCell(id1, type1);
    
    // We need to look up type/color for ID2. 
    // Since we didn't save Type2 in the loop for brevity, let's just use palette[0] or look it up.
    // Ideally, pass Type2 in the loop or look up in cellProps if type is there.
    // Assuming type is constant per ID:
    // (Here we cheat and assume neighbor is same type, or look up strictly via ID/Props)
    // For full correctness, you might want to move Type into cellProps entirely.
    vec4 color2 = vec4(0.0, 0.0, 0.0, 1.0); // Border color logic
    
    // If you want to blend colors of neighbors:
    // You would need to fetch props for id2. 
    // Since id2 came from a neighbor, you can't get its type from 'type1'.
    // If 'type' is in cellProps (Green channel), use this:
    if (id2 != -1) {
        // Fetch type from props
        vec4 props2 = texelFetch(cellProps, ivec2(id2, 0), 0);
        float hp2 = props2.r;
        // Assuming you moved Type to cellProps.g as discussed previously:
        // int type2 = int(props2.g); 
        // For now, let's just make the edge black or darken closest color
         color2 = color1 * 0.5; // Simple border
    }

    finalColor = mix(color2, color1, edgeFactor);
}
    )";
}