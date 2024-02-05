#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_ubo {
  f16vec3 inner;
  uint pad;
  uint pad_1;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  f16vec3 inner;
  uint pad;
  uint pad_1;
} s;

void tint_symbol() {
  f16vec3 x = u.inner;
  s.inner = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
