#version 300 es
precision highp float;
precision highp sampler2D;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;
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

    totalShadow = 1.0 - min(totalShadow, 1.0);

    // Light stuff
    vec3 light = normalize(-lightDir);
    vec3 view = normalize(viewPos - fragPosition);

    // Diffuse
    float diff = max(dot(fragNormal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(fragNormal, halfway), 0.0), 32.0);

    vec3 baseColor = texture(texture0, fragTexCoord * 8.0).rgb;
    vec3 ambient = baseColor * 0.5;
    vec3 diffuse = baseColor * lightColor * diff;
    vec3 specular = lightColor * spec * 0.5;

    vec3 color = (ambient + diffuse + specular) * totalShadow;


    // Mix the ground color with water
    vec3 deepColor = vec3(0.0, 0.5, 0.7);
    float depthCurve = smoothstep(0.0, 1.0, 0.0 - (fragPosition.y * 0.5));

    finalColor = vec4(mix(color, deepColor, depthCurve), 1.0);
}