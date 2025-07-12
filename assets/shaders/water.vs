#version 300 es
precision highp float;

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;  

out vec3 fragPosition;
out vec3 fragNormal;

void main() {
    fragPosition = vertexPosition;
    fragNormal = normalize(mat3(transpose(inverse(matModel))) * vertexNormal);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

