#version 430

out vec4 fragColor;

uniform mat3 uViewRot;
uniform vec3 uProj;
uniform vec2 uJitterOffset;
uniform vec2 uResolution;
uniform float uTime;

#define MAX_OBJECTS 32

// TODO: extend object types, properties

struct ObjectUboData {
//  int type;
//  int matId;
  mat4 model;
};

layout(std140, binding = 0) uniform uObjectBlock {
    int objectsCount;
    ObjectUboData objects[MAX_OBJECTS];
};

float sdSphere(vec3 p) {
  return length(p)-1.0;
}

// TODO: use vectors for translation, scale, and mat3 for rotation
float global_sdf(vec3 p) {
  float f = 1e10;
  for (int i = 0; i < objectsCount; i++) {
    vec3 pn = (objects[i].model * vec4(p, 1.0)).xyz;
    f = min(f, sdSphere(pn));
  }
  return f;
}

vec3 calc_normal(vec3 p, float d0) {
  const vec2 o = vec2(0.001, 0.0);
  float dx = d0 - global_sdf(p + o.xyy);
  float dy = d0 - global_sdf(p + o.yxy);
  float dz = d0 - global_sdf(p + o.yyx);
  return normalize(vec3(dx, dy, dz));
}

// TODO: use enhanced sphere tracing
vec3 ray_march(vec3 ro, vec3 rd) {
  const int STEPS = 32;
  const float HIT_THRESHOLD = 0.01;

  const float CLIP_START = 0.0;
  const float CLIP_END = 50.0;

  float dist = CLIP_START;

  vec3 col;

  for (int i=0; i < STEPS; i++) {
    vec3 pos = ro + rd * dist;
    float df = global_sdf(pos);

    if (df < HIT_THRESHOLD) {
      vec3 nrm = calc_normal(pos, df);
      col = 0.5*abs(nrm);
      col += 0.5*fract(pos);
      col *= 0.5 + 0.5*dot(normalize(vec3(-1.0,-1.0,0.0)), nrm);
    } else if (df > CLIP_END) {
      break;
    }

    dist += df;
  }

  return col;
}

vec3 render(vec2 uv) {
  float scale = uProj.x;
  float fovVal = 1.0 + tan(uProj.y);
  float dist = uProj.z;

  uv += uJitterOffset;

  uv *= scale;

  vec3 ray_backplane = vec3(uv, -dist) * uViewRot;
  vec3 ray_frontplane = vec3(fovVal*uv, -dist+0.5) * uViewRot;

  vec3 ray_org = ray_backplane;
  vec3 ray_dir = normalize(ray_frontplane-ray_backplane);

  vec3 c = ray_march(ray_org, ray_dir);
  return c;
}

void main() {
  vec2 uv = (gl_FragCoord.xy - uResolution*0.5) / max(uResolution.x, uResolution.y);
  fragColor = vec4(render(uv), 1.0);
}
