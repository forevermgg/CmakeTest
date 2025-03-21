#ifndef VARIANT_UTIL_H_
#define VARIANT_UTIL_H_

#include <string>

#include "variant.h"
#include "flatbuffers/flexbuffers.h"

namespace FOREVER {
namespace UTIL {

// Convert from a JSON string to a Variant.
Variant JsonToVariant(const char* json);

// Converts a Variant to a JSON string.
std::string VariantToJson(const Variant& variant);
std::string VariantToJson(const Variant& variant, bool prettyPrint);

// Converts an std::map<Variant, Variant> to Json
std::string StdMapToJson(const std::map<Variant, Variant>& map);

// Converts an std::vector<Variant> to Json
std::string StdVectorToJson(const std::vector<Variant>& vector);

// Convert from a Flexbuffer reference to a Variant.
Variant FlexbufferToVariant(const flexbuffers::Reference& ref);

// Convert from a Flexbuffer map to a Variant.
Variant FlexbufferMapToVariant(const flexbuffers::Map& map);

// Convert from a Flexbuffer vector to a Variant.
Variant FlexbufferVectorToVariant(const flexbuffers::Vector& vector);

// Convert from a Variant to a Flexbuffer buffer.
std::vector<uint8_t> VariantToFlexbuffer(const Variant& variant);

// Convert from a Variant map to a Flexbuffer buffer.
std::vector<uint8_t> VariantMapToFlexbuffer(
    const std::map<Variant, Variant>& map);

// Convert from a Variant vector to a Flexbuffer buffer.
std::vector<uint8_t> VariantVectorToFlexbuffer(
    const std::vector<Variant>& vector);

// Convert from a variant to a Flexbuffer using the given flexbuffer Builder.
// Returns true on success, false otherwise.
bool VariantToFlexbuffer(const Variant& variant, flexbuffers::Builder* fbb);

// Convert from a variant to a Flexbuffer using the given flexbuffer Builder.
// Returns true on success, false otherwise.
bool VariantMapToFlexbuffer(const std::map<Variant, Variant>& map,
                            flexbuffers::Builder* fbb);

// Convert from a variant to a Flexbuffer using the given flexbuffer Builder.
// Returns true on success, false otherwise.
bool VariantVectorToFlexbuffer(const std::vector<Variant>& vector,
                               flexbuffers::Builder* fbb);

}  // namespace UTIL
// NOLINTNEXTLINE - allow namespace overridden
}  // namespace FOREVER

#endif  // VARIANT_UTIL_H_
