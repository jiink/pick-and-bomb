////////////////////////// COMMON 

// Addresses:
// These should be ivec2s containing the pixel coordinates of where certain data
// should go. The coordinates are not normalize but can range from (0, 0) to
// (iResolution.x, iResolution.y).
const ivec2 NUM_PTS_ADDR = ivec2(0, 0);
const ivec2 PT_ADDR = ivec2(1, 0);

// Unfortunately, the only way to define functions that sample iChannels in Common
// is with #define :(

// buf - iChannel to read from
// addr - the data address in the form of an ivec2 (vector containing two integers)
#define fetchData(buf, addr) texelFetch(buf, addr, 0)

// buf_pos - fragment position (fragCoord)
// addr - the data address in the form of an ivec2
// storeData() just evaluates if the data address matches the fragment position
// in which case the data should be stored in fragColor.
#define storeData(buf_pos, addr) ivec2(buf_pos) == addr

//////////////////// BUFFER A


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    if (storeData(fragCoord, NUM_PTS_ADDR)) {
        fragColor = vec4(3.0, 0.0, 0.0, 1.0); // R is how many points there are
    }
    if (storeData(fragCoord, PT_ADDR)) {
        fragColor = vec4(0.25, 0.5, 0.2, 1.0); // R and G are x and y
    }
    if (storeData(fragCoord, ivec2(PT_ADDR.x + 1, PT_ADDR.y))) {
        fragColor = vec4(0.85, 0.2, 0.8, 1.0); // R and G are x and y
    }
    if (storeData(fragCoord, ivec2(PT_ADDR.x + 2, PT_ADDR.y))) {
        fragColor = vec4(0.65, 0.7, 0.1, 1.0); // R and G are x and y
    }
}


//////////////////////// IMAGE


// Example of using buffers to carry states between frames.
// Important note: colors are not clamped so you have a pretty broad range to store data in.

// Common - code available to all tabs (it is quite literally inserted into each)
// Buffer A - code for variable management
// Image - main shader that outputs to the display

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    
    int numPts = int(floor(fetchData(iChannel0, NUM_PTS_ADDR).r));
    float lum = 0.0;
    float minDist = 80.0;
    for (int i = 0; i < numPts; i++) {
        vec4 cellData = fetchData(iChannel0, ivec2(PT_ADDR.x + i, PT_ADDR.y));
        vec2 cellCoord = cellData.rg;
        float cellLum = cellData.b;
        if (i == 0 && iMouse.z > 0.0) {
            cellCoord = iMouse.xy/iResolution.xy;
        }
        float dist = distance(uv, cellCoord);
        if (dist < minDist) {
            minDist = dist;
            lum = cellLum + dist *0.4;
        }
    }
    fragColor = vec4(lum, lum, lum, 1.0);
}