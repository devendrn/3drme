#version 430

out vec4 fragColor;

uniform mat3 uViewRot;
uniform vec3 uProj;
uniform vec3 uCamTarget;
uniform vec2 uJitterOffset;
uniform vec2 uResolution;
uniform float uTime;
uniform int uRaymarchSteps;
uniform int uReflRaymarchSteps;
uniform vec3 uRaymarchParams;
uniform vec2 uOcclusionParams;
uniform vec3 uAmbientColor;
uniform float uFogFadeIn;

uniform float uN[1024];

#define MAX_OBJECTS 32

#define FLOAT_MAX 1e10

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
// Smooth min: https://iquilezles.org/articles/smin/

float dot2(vec2 x) { return dot(x, x); }
float dot2(vec3 x) { return dot(x, x); }

// legacy
float sdfSphere(vec3 p) {
  return length(p)-1.0;
}
float sdfBox(vec3 p) {
  vec3 q = abs(p) - 1.0;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// used by node editor
float sdfSphere(vec3 p, float r) {
  return length(p)-r;
}
float sdfBox(vec3 p, vec3 b, float r) {
  vec3 q = abs(p) - b + r;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}
float sdfCylinder(vec3 p, float ra, float h, float rb) {
  vec2 d = vec2( length(p.xz)-2.0*ra+rb, abs(p.y)-h+rb);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rb;
}
float sdfTorus(vec3 p, float r, float t) {
  vec2 q = vec2(length(p.xz)-r,p.y);
  return length(q)-t;
}
float sdfPlane(vec3 p, vec3 n) {
  return dot(p,n);
}
float sdfCappedCone(vec3 p, float h, float r1, float r2, float r) {
  h -= r;
  p.y += 0.5*r;
  r1 = max(r1-r, 0.0);
  r2 = max(r2-r, 0.0);
  vec2 q = vec2(length(p.xz), -p.y);
  vec2 k1 = vec2(r2,h);
  vec2 k2 = vec2(r2-r1,2.0*h);
  vec2 ca = vec2(q.x-min(q.x,(q.y<0.0)?r1:r2), abs(q.y)-h);
  vec2 cb = q - k1 + k2*clamp(dot(k1-q,k2)/dot2(k2), 0.0, 1.0);
  float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
  return s*sqrt(min(dot2(ca),dot2(cb))) - 0.5*r;
}

float sdfShape(vec3 p, int type) {
  switch(type) {
    case 0:
      return sdfBox(p);
    case 1:
      return sdfSphere(p);
    default:
      return 1e10;
  }
}

struct Surface {
  float dist;
  vec3 color;
  float selected;
  float roughness;
};

struct Light {
  vec3 position;
  vec3 color;
  int shadowSteps;
  float radius;
  float attenuation;
  bool isDirectional;
};

mat3 rmat(vec3 r) {
  vec3 s = sin(r);
  vec3 c = cos(r);
  return mat3(
    c.y * c.z, s.x * s.y * c.z - c.x * s.z, c.x * s.y * c.z + s.x * s.z,
    c.y * s.z, s.x * s.y * s.z + c.x * c.z, c.x * s.y * s.z - s.x * c.z,
    -s.y, s.x * c.y, c.x * c.y
  );
}

vec2 smin(float a, float b, float k) {
  float h = 1.0-min(abs(a-b)/(4.0*k), 1.0);
  float w = h*h;
  float m = w*0.5;
  float s = w*k;
  return (a < b) ? vec2(a-s,m) : vec2(b-s,1.0-m);
}

vec2 smax(float a, float b, float k) {
  float h = 1.0-min(abs(a-b)/(4.0*k), 1.0);
  float w = h*h;
  float m = w*0.5;
  float s = w*k;
  return (a > b) ? vec2(a+s,m) : vec2(b+s,1.0-m);
}

vec2 sdiff(float a, float b, float k) {
  float h = 1.0-min(abs(a+b)/(4.0*k), 1.0);
  float w = h*h;
  float m = w*0.5;
  float s = w*k;
  return (a > -b) ? vec2(a+s,m) : vec2(-b+s,1.0-m);
}

Surface mixSurfParams(Surface a, Surface b, vec2 m) {
  a.dist = m.x;
  a.color = mix(a.color, b.color, m.y);
  a.roughness = mix(a.roughness, b.roughness, m.y);
  return a;
}

Surface mSurf(Surface a, Surface b, float k) {
  return mixSurfParams(a, b, vec2(mix(a.dist, b.dist, k), k));
}

Surface uSurf(Surface a, Surface b) {
  return (a.dist < b.dist) ? a : b;
}

Surface uSurf(Surface a, Surface b, float k) {
  vec2 m = smin(a.dist, b.dist, k);
  return mixSurfParams(a, b, m);
}

Surface iSurf(Surface a, Surface b) {
  return (a.dist > b.dist) ? a : b;
}

Surface iSurf(Surface a, Surface b, float k) {
  vec2 m = smax(a.dist, b.dist, k);
  return mixSurfParams(a, b, m);
}

Surface dSurf(Surface a, Surface b) {
  b.dist = -b.dist;
  return (a.dist > b.dist) ? a : b;
}

Surface dSurf(Surface a, Surface b, float k) {
  vec2 m = sdiff(a.dist, b.dist, k);
  return mixSurfParams(a, b, m);
}

// TODO: Find a better place to do transformations

vec3 applyTransform(vec3 p, mat4 t) {
  p += vec3(t[0][3], t[1][3], t[2][3]);
  p = mat3(t) * p;
  p *= vec3(t[3][0], t[3][1], t[3][2]);
  return p;
}

vec3 renderSky(vec3 pos, float t) {
  vec3 s = uAmbientColor;
  // !sky_inline
  return s;
}

Surface nodeEditorSdf(vec3 pos, float t) {
  Surface s = Surface(FLOAT_MAX, vec3(0.0), 0.0, 0.0);
  // !sdf_inline
  return s;
}

float sceneSdf(vec3 p) {
  float f = FLOAT_MAX;
  for (int i = 0; i < objectsCount; i++) {
    vec3 q = applyTransform(p, objects[i].transformation); // This is expensive
    float dist = sdfShape(q, objects[i].typeMatId.x);
    f = min(f, dist);
  }
  // FIXME: Construct simpler separate sdf from node editor
  f = min(f, nodeEditorSdf(p, uTime).dist);
  return f;
}

Surface sceneSdfSurf(vec3 p)  {
  Surface s = Surface(FLOAT_MAX, vec3(0.0), 0.0, 0.0);
  for (int i = 0; i < objectsCount; i++) {
    ObjectUboData obj = objects[i];
    vec3 q = applyTransform(p, obj.transformation); // This is expensive
    float dist = sdfShape(q, obj.typeMatId.x);
    vec3 col = obj.typeMatId.x > 0 ? vec3(1.0, 0.0, 0.0) : vec3(1.0); // TODO: Implement materials
    s = uSurf(s, Surface(dist, col, float(obj.typeMatId.z), 0.0));
  }
  s = uSurf(s, nodeEditorSdf(p, uTime));
  return s;
}

vec3 calcNormal(vec3 p, float d0) {
  const vec2 o = vec2(0.001, 0.0);
  float dx = d0 - sceneSdf(p + o.xyy);
  float dy = d0 - sceneSdf(p + o.yxy);
  float dz = d0 - sceneSdf(p + o.yyx);
  return normalize(vec3(dx, dy, dz));
}

// unused - preserved for comparision
vec3 rayMarchSimple(vec3 ro, vec3 rd) {
  const int STEPS = 32;
  const float HIT_THRESHOLD = 0.01;

  const float CLIP_START = 0.0;
  const float CLIP_END = 50.0;

  float dist = CLIP_START;

  vec3 col = vec3(0.05);

  for (int i=0; i < STEPS; i++) {
    vec3 pos = ro + rd * dist;
    Surface s = sceneSdfSurf(pos);

    if (s.dist < HIT_THRESHOLD) {
      vec3 nrm = calcNormal(pos, s.dist);
      col = s.color;
      col *= 0.1 + 0.9*max(dot(normalize(-vec3(0.8,1.0,0.5)), nrm), 0.0);
    } else if (s.dist > CLIP_END) {
      break;
    }

    dist += s.dist;
  }

  return col;
}

// https://iquilezles.org/articles/rmshadows/
float softShadow(vec3 ro, vec3 rd, int steps, float mint, float maxt, float w) {
  float res = 1.0;
  float ph = 1e20;
  float t = mint;
  for (int i=0; i<steps && t<maxt; i++) {
    float h = sceneSdfSurf(ro + rd*t).dist;
    if (h<0.01) return 0.0;
    float y = h*h/(2.0*ph);
    float d = sqrt(h*h-y*y);
    res = min( res, d/(w*max(0.0,t-y)) );
    ph = h;
    t += h;
  }
  return res;
}

vec3 rayMarch(vec3 ro, vec3 rd) {
  const int MAX_ITERATIONS = uRaymarchSteps;
  const float TMIN = uRaymarchParams.x;
  const float TMAX = uRaymarchParams.y;
  const float PIXEL_RADIUS = uRaymarchParams.z;

  float candidateError = FLOAT_MAX;
  float previousRadius = 0.0;
  float stepLength = 0.0;
  float omega = 1.2;
  float dist = TMIN;

  Surface s = Surface(FLOAT_MAX, vec3(0.0), 0.0, 0.0);
  vec3 pos = ro;
  for (int i=0; i < MAX_ITERATIONS; i++) {
    pos = ro + rd * dist;

    s = sceneSdfSurf(pos);

    float signedRadius = s.dist;
    float radius = abs(s.dist);

    bool sorFail = omega > 1.0 && (radius + previousRadius) < stepLength;

    if (sorFail) {
      stepLength -= omega * stepLength;
      omega = 1.0;
    } else {
      stepLength = omega * signedRadius; 
    }

    previousRadius = radius;

    float error = radius / dist;
    if (!sorFail && error < candidateError) {
      candidateError = error;
    }

    if (!sorFail && error < PIXEL_RADIUS || dist > TMAX) break;

    dist += stepLength;
  }

  float t = uTime;
  Light lights[] = Light[](
    Light(vec3(0),vec3(0),0,0.0,0.0,true) // unused
    // !lights_inline
  );

  vec3 lightGlow = vec3(0.0);
  for (int i = 1; i<lights.length(); i++) {
    Light l = lights[i];
    vec3 lpos = l.position;
    if (l.isDirectional) {
      lpos = normalize(lpos);
      if (dist < TMAX && candidateError <= PIXEL_RADIUS) continue;
      float g = max(dot(lpos, rd), 0.0);
      g *= g;
      lightGlow += l.color*g/(1.0+((1.0-g)/(0.0005 + 0.02*l.radius)));
      continue;
    }
    vec3 ab = ro - lpos;
    float pd = length(ro-pos);
    float d = length(ab);
    if (pd < d || dot(ab, rd)>0.0) continue;
    vec3 ac = (ro + rd*d) - lpos;
    vec3 abd = cross(ab, ac);
    float ar = dot(abd, abd)/d;
    lightGlow += l.color/(0.001+(30.0*ar*ar*(0.5+l.attenuation)/(0.005+l.radius)));
  }

  vec3 sky = renderSky(rd*TMAX, uTime);

  if (dist > TMAX || candidateError > PIXEL_RADIUS) {
    sky += lightGlow;
    return sky;
  }

  vec3 col = s.color;
  vec3 nrm = calcNormal(pos, s.dist);
  float cosr = 1.0-max(dot(nrm, rd), 0.0);

  vec3 lighting = vec3(0.0);
  for (int i = 1; i<lights.length(); i++) {
    Light l = lights[i];
    vec3 lightDir;
    if (l.isDirectional) {
      lightDir = -normalize(l.position);
    } else {
      lightDir = normalize(pos - l.position);
    }
    vec3 reflectDir = reflect(-lightDir, nrm);

    float diffuse =  max(dot(lightDir, nrm), 0.0);
    float r0 = s.roughness;
    float r1 = 1.0-r0;
    float specular = 2.0*pow(max(dot(rd, reflectDir), 0.0), 1.0/(r0*r0))*(r1*r1);

    float dr = 100.0;
    if (!l.isDirectional) {
        dr = length(pos - l.position);
    }
    float shadow = softShadow(pos, -lightDir, l.shadowSteps, 0.2, dr, l.radius);
    if (!l.isDirectional) {
      shadow /= (1.0+(dr*dr)*l.attenuation);
    }

    lighting += (diffuse*col + specular)*l.color*shadow;
  }

  float oct = uOcclusionParams.y;
  float occl = sceneSdfSurf(pos- nrm*oct).dist - oct;
  occl = 1.0-min(occl*occl, 1.0);

  vec3 ambient = uAmbientColor*mix(1.0, occl, uOcclusionParams.x);
  lighting += col*ambient;

  col = lighting;

  if (s.roughness < 0.99) {
    vec3 reflVdir = reflect(rd, nrm.xyz);
    vec3 co = renderSky(reflVdir, t);
    float r0 = 1.0-s.roughness;
    float reflecOccl = softShadow(pos, reflVdir, uReflRaymarchSteps, 0.3, 30.0, (1.0-r0*r0)*0.7);
    float fresnel = 0.06+0.94*cosr*cosr*cosr;
    col += reflecOccl*fresnel*co*r0;
  }

  // legacy
  float selfactor = 0.1 + 0.2*cosr+ 0.7*cosr*cosr*cosr;
  selfactor *= s.selected;
  col = mix(col, vec3(1.0, 0.7, 0.3), selfactor);
  col *= 1.0+selfactor;
  col = mix(col, sky, smoothstep(uFogFadeIn,1.0,dist/TMAX));

  col += lightGlow;

  return col;
}

vec3 render(vec2 uv) {
  float scale = uProj.x;
  float fovVal = 1.0 + tan(uProj.y);
  float dist = -uProj.z;

  uv += uJitterOffset;

  uv *= scale;

  vec3 ray_backplane = vec3(uv, dist) * uViewRot;
  vec3 ray_frontplane = vec3(fovVal*uv, dist+0.5) * uViewRot;

  vec3 ray_org = ray_backplane - uCamTarget;
  vec3 ray_dir = normalize(ray_frontplane-ray_backplane);

  vec3 c = rayMarch(ray_org, ray_dir);

  const float ws = 0.063;
  c = c*(1.0+c*ws)/(1.0+c);

  return c;
}

void main() {
  vec2 uv = (gl_FragCoord.xy - uResolution*0.5) / max(uResolution.x, uResolution.y);
  fragColor = vec4(render(uv), 1.0);
}
