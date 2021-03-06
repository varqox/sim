#pragma once

#include <simlib/inplace_buff.hh>

namespace sim {

template <size_t STATIC_LEN = 32>
class BlobField : public InplaceBuff<STATIC_LEN> {
public:
	using StrType = InplaceBuff<STATIC_LEN>;

	using StrType::StrType;
	using StrType::operator=;

	constexpr BlobField(const BlobField&) = default;
	constexpr BlobField(BlobField&&) noexcept = default;
	constexpr BlobField& operator=(const BlobField&) = default;
	constexpr BlobField& operator=(BlobField&&) noexcept = default;
	~BlobField() = default;
};

} // namespace sim
