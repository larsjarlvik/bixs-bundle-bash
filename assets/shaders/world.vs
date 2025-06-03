#version 330

// Attributes from mesh
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 vertexTexCoord;

// Uniforms from raylib
uniform mat4 mvp;
uniform mat4 matModel;  

// Pass to fragment shader
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragColor;
out vec2 fragTexCoord;

void main() {
    vec4 worldPosition =  vec4(vertexPosition, 1.0);
    
    fragPosition = worldPosition.xyz;
    fragNormal = normalize(mat3(transpose(inverse(matModel))) * vertexNormal);
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

