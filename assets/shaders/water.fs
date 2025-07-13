#version 300 es
precision highp float;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragTangent;
in vec3 fragBitangent;

out vec4 finalColor;

uniform sampler2D texture0; // depth texture
uniform sampler2D texture2; // normal map
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float time;

void main() {
    float depth = -texture(texture0, fragTexCoord).r;

    vec2 worldPos2D = fragPosition.xz;
    vec2 toCenter = normalize(-worldPos2D);

    // Simulate waves
    float pushPull = sin(worldPos2D.x * 0.3 + time * 0.05) * sin(worldPos2D.y * 0.25 + time * 0.04);
    vec2 organicFlow = toCenter * (1.0 + pushPull * 0.1);
    vec2 animatedUV = fragTexCoord * 3.0 + organicFlow * time * -0.001;
    vec3 normalMap = texture(texture2, animatedUV).rgb;
    normalMap = normalMap * 2.0 - 1.0;

    float invertPattern = sin(worldPos2D.x * 0.15 + time * 0.1) * sin(worldPos2D.y * 0.12 + time * 0.05);
    normalMap.y *= invertPattern * depth * 0.5;

    mat3 TBN = mat3(
        normalize(fragTangent),
        normalize(fragBitangent),
        normalize(fragNormal)
    );

    vec3 finalNormal = normalize(TBN * normalMap);

    vec3 light = normalize(-lightDir);
    vec3 view = normalize(viewPos - fragPosition);

    // Diffuse
    float diff = max(dot(finalNormal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(finalNormal, halfway), 0.0), 32.0);

    // Depth and color calculations
    vec3 shallowColor = vec3(0.4, 0.9, 0.8);
    vec3 deepColor = vec3(0.0, 0.5, 0.7);
    float depthCurve = smoothstep(0.0, 1.0, depth * 0.4);
    vec3 baseColor = mix(shallowColor, deepColor, depthCurve);

    float x = depth * 4.0;
    float alpha = (x * x);
    alpha = clamp(alpha, 0.0, 0.5);
    alpha += (depth * depth) * 0.5;

    vec3 ambient = baseColor * 0.5;
    vec3 diffuse = baseColor * lightColor * diff * 0.5;
    vec3 specular = lightColor * spec; // Increased specular for water

    vec3 color = ambient + diffuse + specular;
    finalColor = vec4(color, alpha);
}