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

    // Color
    vec4 baseColor;
    if (useTexture) {
        // Blend texture with vertex color
        vec4 textureColor = texture(texture0, fragTexCoord);
        vec3 vertexColor = pow(fragColor, vec3(1.0 / 2.2));
        baseColor = vec4(textureColor.rgb * vertexColor * colDiffuse.rgb, textureColor.a * colDiffuse.a);
    } else {
        // Use vertex color only
        baseColor = vec4(pow(fragColor, vec3(1.0 / 2.2)) * colDiffuse.rgb, colDiffuse.a);
    }

    // Diffuse
    float diff = max(dot(normal, light), 0.0);

    // Specular (Blinn–Phong)
    vec3 halfway = normalize(light + view);
    float spec = pow(max(dot(normal, halfway), 0.0), 6.0);

    vec3 ambient = baseColor.rgb * 0.5;
    vec3 diffuse = baseColor.rgb * lightColor * diff;
    vec3 specular = lightColor * spec * 0.3;

    vec3 color = ambient + diffuse + specular;

    // Fade out elements close to camera
    float distanceFromCamera = distance(fragPosition, viewPos);
    float fadeAlpha = smoothstep(1.0, 3.0, distanceFromCamera);

    if (baseColor.a < 0.5) discard;

    finalColor = vec4(color, fadeAlpha);
}
