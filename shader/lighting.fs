#version 330 core
//  텍스처있는 3d모델 전용

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D diffuseTexture;

void main() {

    vec3 texColor = texture(diffuseTexture, texCoord).rgb;
    fragColor = vec4(texColor, 1.0);  // 텍스처 색상만 사용하여 최종 색상 계산
}