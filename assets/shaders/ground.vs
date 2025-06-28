#version 300 es
precision highp float;

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform vec2 radius;

out vec2 fragTexCoord;

void main() {
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}