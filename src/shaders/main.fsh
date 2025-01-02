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
  mat4 transformation;
  ivec4 typeMatId;
};

layout(std140, binding = 0) uniform uObjectBlock {
  int objectsCount;
  ObjectUboData objects[MAX_OBJECTS];
};

// SDF functions: https://iquilezles.org/articles/distfunctions/

float sdfSphere(vec3 p) {
  return length(p)-1.0;
}

float sdfBox(vec3 p) {
  vec3 q = abs(p) - 1.0;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

vec3 applyTransform(vec3 p, mat4 t) {
  p *= vec3(t[3][0], t[3][1], t[3][2]);
  p += vec3(t[0][3], t[1][3], t[2][3]);
  p = mat3(t) * p;
  return p;
}

float sceneSdf(vec3 p) {
  float f = 1e10;
  for (int i = 0; i < objectsCount; i++) {
    // Applying transformation here is a bad idea, I know
    vec3 q = applyTransform(p, objects[i].transformation);
    float dist;
    switch(objects[i].typeMatId.x) {
      case 0:
        dist = sdfBox(q); break;
      case 1:
        dist = sdfSphere(q); break;
      default:
        dist = 1e10;
    }
    f = min(f, dist);
  }
  return f;
}

vec3 calcNormal(vec3 p, float d0) {
  const vec2 o = vec2(0.001, 0.0);
  float dx = d0 - sceneSdf(p + o.xyy);
  float dy = d0 - sceneSdf(p + o.yxy);
  float dz = d0 - sceneSdf(p + o.yyx);
  return normalize(vec3(dx, dy, dz));
}

// TODO: use enhanced sphere tracing
vec3 rayMarch(vec3 ro, vec3 rd) {
  const int STEPS = 32;
  const float HIT_THRESHOLD = 0.01;

  const float CLIP_START = 0.0;
  const float CLIP_END = 50.0;

  float dist = CLIP_START;

  vec3 col;

  for (int i=0; i < STEPS; i++) {
    vec3 pos = ro + rd * dist;
    float df = sceneSdf(pos);

    if (df < HIT_THRESHOLD) {
      vec3 nrm = calcNormal(pos, df);
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

  vec3 c = rayMarch(ray_org, ray_dir);
  return c;
}

void main() {
  vec2 uv = (gl_FragCoord.xy - uResolution*0.5) / max(uResolution.x, uResolution.y);
  fragColor = vec4(render(uv), 1.0);
}
