#version 300 es
precision highp float;

in vec2 fragTexCoord;

uniform float groundSize;
uniform int shadowCount;
uniform vec3 shadowPositions[64];  // Define MAX_SHADOWS as 64 or adjust as needed
uniform float shadowRadii[64];

out vec4 finalColor;

void main() {
    vec3 worldPos = vec3((fragTexCoord.x - 0.5) * groundSize, 0.0, (fragTexCoord.y - 0.5) * groundSize);
    float totalShadow = 0.0;

    for (int i = 0; i < shadowCount && i < 64; i++) {
        vec3 shadowPos = shadowPositions[i];
        float shadowRadius = shadowRadii[i];

        float dist = distance(worldPos.xz, shadowPos.xz);

        float normalizedDist = dist / shadowRadius;

        float alpha = 1.0 - smoothstep(0.0, 1.0, normalizedDist);
        alpha = alpha * alpha;
        alpha *= 0.8;
        alpha *= smoothstep(1.0, 0.8, normalizedDist);

        totalShadow = max(totalShadow, alpha);
    }

    totalShadow *= 0.5;
    finalColor = vec4(1.0 - totalShadow, 1.0 - totalShadow, 1.0 - totalShadow, 1.0);
}