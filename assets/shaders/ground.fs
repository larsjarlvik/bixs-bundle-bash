#version 300 es
precision highp float;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform int shadowCount;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 shadowPositions[64];
uniform float shadowRadii[64];
uniform float shadowIntensities[64];

out vec4 finalColor;

void main() {
    // Shadow calculations
    float totalShadow = 0.0;

    for (int i = 0; i < shadowCount && i < 64; i++) {
        vec3 shadowPos = shadowPositions[i];
        float shadowRadius = shadowRadii[i];
        float shadowIntensity = shadowIntensities[i];

        float dist = distance(fragPosition, shadowPos);
        float normalizedDist = dist / shadowRadius;

        float alpha = exp(-normalizedDist * normalizedDist * 2.5);
        alpha *= shadowIntensity;

        totalShadow += alpha;
    }

    totalShadow = min(totalShadow, 1.0) * 0.5;

    // Light stuff
    vec3 normal = normalize(fragNormal);
    vec3 light = normalize(-lightDir);
    vec3 view = normalize(viewPos - fragPosition);

    // Diffuse
    float diff = max(dot(normal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 baseColor = vec3(0.4, 0.8, 0.2); // TODO: Add color/texture

    vec3 ambient = baseColor * 0.5;
    vec3 diffuse = baseColor * lightColor * diff;
    vec3 specular = lightColor * spec * 0.5;

    vec3 color = ambient + diffuse + specular;

    // Combine results
    finalColor = vec4(color - totalShadow, 1.0);
}