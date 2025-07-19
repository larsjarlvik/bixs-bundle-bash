#version 300 es
precision highp float;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;  

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragColor;
out vec2 fragTexCoord;
out vec4 fragClipPos;

void main() {
    fragPosition = (matModel * vec4(vertexPosition, 1.0)).xyz;
    fragNormal = normalize(mat3(transpose(inverse(matModel))) * vertexNormal);
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
    fragClipPos = gl_Position;
}

