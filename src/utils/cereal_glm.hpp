#ifndef CEREAL_GLM_H
#define CEREAL_GLM_H

#include <glm/glm.hpp>

namespace cereal {

template <class Archive> void serialize(Archive& archive, glm::vec2& d) { archive(d.x, d.y); };
template <class Archive> void serialize(Archive& archive, glm::vec3& d) { archive(d.x, d.y, d.z); };
template <class Archive> void serialize(Archive& archive, glm::vec4& d) { archive(d.x, d.y, d.z, d.w); };

} // namespace cereal

#endif
