/*
 * Copyright (C) 2024 The Android Open Source Project
 *
* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKAN_UTILS_HELPER_H
#define TNT_FILAMENT_BACKEND_VULKAN_UTILS_HELPER_H

#include <backend/DriverEnums.h>

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>
#include <utils/Log.h>

#include <bluevk/BlueVK.h>

#include <utility>

namespace filament::backend::fvkutils {

inline bool equivalent(const VkRect2D& a, const VkRect2D& b) {
    // These are all integers so there's no need for an epsilon.
    return a.extent.width == b.extent.width && a.extent.height == b.extent.height &&
           a.offset.x == b.offset.x && a.offset.y == b.offset.y;
}

inline bool equivalent(const VkExtent2D& a, const VkExtent2D& b) {
    return a.height == b.height && a.width == b.width;
}

// Helper function for the vkEnumerateX methods. These methods have the format of
// VkResult vkEnumerateX(InputType1 arg1, InputTyp2 arg2, ..., uint32_t* size,
//         OutputType* output_arg)
// Instead of macros and explicitly listing the template params, Variadic Template was also
// considered, but because the "variadic" part of the vk methods (i.e. the inputs) are before the
// non-variadic parts, this breaks the template type matching logic. Hence, we use a macro approach
// here.
#define EXPAND_ENUM(...)                                                          \
    uint32_t size = 0;                                                            \
    fprintf(stderr, "[ENUM DEBUG] Calling enumerate function at %s:%d\n", __FILE__, __LINE__); \
    fflush(stderr); \
    VkResult result = func(__VA_ARGS__, nullptr);                                 \
    fprintf(stderr, "[ENUM DEBUG] First call result: %d (0x%x), size: %u\n", (int)result, (int)result, (unsigned)size); \
    fflush(stderr); \
    if (result != VK_SUCCESS) {                                                   \
        fprintf(stderr, "[VULKAN ERROR] Enumeration failed with code: %d (0x%x)\n", (int)result, (int)result); \
        fflush(stderr); \
    }                                                                             \
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "enumerate size error"; \
    utils::FixedCapacityVector<OutType> ret(size);                                \
    result = func(__VA_ARGS__, ret.data());                                       \
    if (result != VK_SUCCESS) {                                                   \
        utils::slog.e << "Vulkan enumeration (second call) failed with error code: " << (int)result << " (" << result << ")" << utils::io::endl; \
    }                                                                             \
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "enumerate error";      \
    return std::move(ret);

#define EXPAND_ENUM_NO_ARGS() EXPAND_ENUM(&size)
#define EXPAND_ENUM_ARGS(...) EXPAND_ENUM(__VA_ARGS__, &size)

template <typename OutType>
utils::FixedCapacityVector<OutType> enumerate(VKAPI_ATTR VkResult (*func)(uint32_t*, OutType*)) {
    EXPAND_ENUM_NO_ARGS();
}

template <typename InType, typename OutType>
utils::FixedCapacityVector<OutType> enumerate(
        VKAPI_ATTR VkResult (*func)(InType, uint32_t*, OutType*), InType inData) {
    EXPAND_ENUM_ARGS(inData);
}

template <typename InTypeA, typename InTypeB, typename OutType>
utils::FixedCapacityVector<OutType> enumerate(
        VKAPI_ATTR VkResult (*func)(InTypeA, InTypeB, uint32_t*, OutType*), InTypeA inDataA,
        InTypeB inDataB) {
    EXPAND_ENUM_ARGS(inDataA, inDataB);
}

#undef EXPAND_ENUM
#undef EXPAND_ENUM_NO_ARGS
#undef EXPAND_ENUM_ARGS

} // namespace filament::backend::fvkutils

#endif // TNT_FILAMENT_BACKEND_VULKAN_UTILS_HELPER_H
