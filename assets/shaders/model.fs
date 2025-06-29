#version 300 es
precision highp float;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool useTexture;
uniform vec4 colDiffuse;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 light = normalize(-lightDir);
    vec3 view = normalize(viewPos - fragPosition);

    // Diffuse
    float diff = max(dot(normal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 baseColor = (useTexture ? texture(texture0, fragTexCoord).rgb : fragColor) * colDiffuse.rgb;

    vec3 ambient = baseColor * 0.5;
    vec3 diffuse = baseColor * lightColor * diff;
    vec3 specular = lightColor * spec * 0.5;

    vec3 color = ambient + diffuse + specular;
    finalColor = vec4(color, 1.0);
}
