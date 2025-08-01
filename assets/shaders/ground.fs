#version 300 es
precision highp float;
precision highp sampler2D;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0; // ground
uniform sampler2D texture1; // coast

uniform int shadowCount;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 shadowPositions[128];
uniform float shadowRadii[128];
uniform float shadowIntensities[128];

out vec4 finalColor;

void main() {
    // Shadow calculations
    float lightTransmission = 1.0;
    for (int i = 0; i < shadowCount; i++) {
        vec3 delta = fragPosition - shadowPositions[i];
        float distSq = dot(delta, delta);

        if (distSq > shadowRadii[i]) continue;

        float t = distSq / shadowRadii[i];
        float alpha = (1.0 - t) * (1.0 - t) * shadowIntensities[i];

        lightTransmission *= (1.0 - alpha);
    }

    lightTransmission = clamp(lightTransmission, 0.3, 1.0);

    // Light stuff
    vec3 light = normalize(-lightDir);
    vec3 view = normalize(viewPos - fragPosition);

    // Diffuse
    float diff = max(dot(fragNormal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(fragNormal, halfway), 0.0), 6.0);

    vec3 baseColor = mix(
        texture(texture1, fragTexCoord * 6.0).rgb,
        texture(texture0, fragTexCoord * 6.0).rgb,
        clamp(fragPosition.y, 0.0, 1.0)
    );

    vec3 ambient = baseColor * 0.5;
    vec3 diffuse = baseColor * lightColor * diff;
    vec3 specular = lightColor * spec * 0.5;

    vec3 color = (ambient + diffuse + specular) * lightTransmission;

    // Mix the ground color with water
    vec3 deepColor = vec3(0.0, 0.5, 0.7);
    float depthCurve = smoothstep(0.0, 1.0, 0.0 - (fragPosition.y * 0.5));

    finalColor = vec4(mix(color, deepColor, depthCurve), 1.0);
}