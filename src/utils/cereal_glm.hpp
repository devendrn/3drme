#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

namespace cereal {

template <class Archive> void serialize(Archive& archive, glm::vec2& d) { archive(d.x, d.y); };
template <class Archive> void serialize(Archive& archive, glm::vec3& d) { archive(d.x, d.y, d.z); };
template <class Archive> void serialize(Archive& archive, glm::vec4& d) { archive(d.x, d.y, d.z, d.w); };

} // namespace cereal
