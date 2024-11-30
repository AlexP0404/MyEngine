#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstring>
#include <type_traits>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult err = x;                                                          \
    if (err) {                                                                 \
      throw std::runtime_error("Detected Vulkan error : " +                    \
                               std::string(string_VkResult(err)));             \
    }                                                                          \
  } while (0)

class Utils {
public:
  template <typename T> inline static void zeroInitializeStruct(T &a) {
    static_assert(std::is_class<T>::value, "T must be a struct! (or class)");
    memset(&a, 0, sizeof(a));
  }
};

#endif
