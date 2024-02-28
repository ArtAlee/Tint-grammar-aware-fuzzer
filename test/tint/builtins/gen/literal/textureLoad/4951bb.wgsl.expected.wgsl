enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r8unorm, read>;

fn textureLoad_4951bb() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<u32>(1u), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_4951bb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_4951bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_4951bb();
}
