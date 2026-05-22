#pragma once

#include <cstdint>

namespace sema {

enum class OwnershipStatus {
  Uninitialized, // declared but no value
  Owned,         // var owns the data
  BorrowedImm,   // data is borrowed (read-only)
  BorrowedMut,   // data is borrowed (read-write, exclusive)
  Moved          // data has been moved elsewhere
};

struct BorrowState {
  OwnershipStatus status = OwnershipStatus::Uninitialized;
  uint32_t immutable_borrow_count = 0; // Tracks number of active &T
};

} // namespace sema