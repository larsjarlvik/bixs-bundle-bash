#version 300 es
precision highp float;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform float time;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragTangent;
out vec3 fragBitangent;
out float fragSurfaceLevel;

void main() {
    fragPosition = vertexPosition;
    fragNormal = normalize(mat3(transpose(inverse(matModel))) * vertexNormal);
    fragTexCoord = vertexTexCoord;
    fragTangent = normalize(vec3(matNormal * vec4(1.0, 0.0, 0.0, 0.0)));
    fragBitangent = normalize(vec3(matNormal * vec4(0.0, 0.0, 1.0, 0.0)));

    float invertPattern = sin(vertexPosition.x * 2.0 + time * 0.1) * sin(vertexPosition.z * 2.0 + time * 0.05);
    fragSurfaceLevel = invertPattern * clamp(vertexPosition.y * 0.5, 0.08, 0.4);

    gl_Position = mvp * vec4(vertexPosition.x, fragSurfaceLevel, vertexPosition.z, 1.0);
}

