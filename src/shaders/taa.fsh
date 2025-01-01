#version 430

out vec4 fragColor;

uniform sampler2D uCurrentFrame;
uniform sampler2D uHistoryFrame;

uniform float uFeedbackFactor;
uniform vec2 uJitterOffset;
uniform vec2 uResolution;

void main() {
  vec2 scale = 1.0 / uResolution;
  vec2 uv = gl_FragCoord.xy * scale;

  vec2 uvJittered = uv - 2.0*uJitterOffset;

  vec3 currentColor = texture(uCurrentFrame, uvJittered).rgb;
  vec3 historyColor = texture(uHistoryFrame, uv).rgb;

  vec3 diffColor = currentColor - historyColor;
  diffColor *= diffColor; 
  float feedback = min(20.0*max(diffColor.r, max(diffColor.g, diffColor.b)), 1.0);
  feedback = mix(0.7*uFeedbackFactor, uFeedbackFactor, feedback);

  vec3 resolvedColor = mix(currentColor, historyColor, feedback);

  fragColor = vec4(resolvedColor, 1.0);
}
