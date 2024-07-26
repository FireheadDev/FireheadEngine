#version 450

layout(binding = 0) uniform Camera {
	mat4 view;
	mat4 projection;
} camera;
layout(std140, binding = 1) readonly buffer InstanceData {
	mat4 transforms[];
} instanceData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
	gl_Position = camera.projection * camera.view * instanceData.transforms[gl_InstanceIndex] * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
