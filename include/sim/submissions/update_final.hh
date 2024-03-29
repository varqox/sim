#pragma once

#include <cstdint>
#include <optional>
#include <simlib/mysql/mysql.hh>

namespace sim::submissions {

// Has to be called in a transaction before update_final() is called with
// make_transaction == false, in order to avoid deadlocks; It does not have to be freed.
void update_final_lock(
    mysql::Connection& mysql, std::optional<uint64_t> submission_owner, uint64_t problem_id
);

void update_final(
    mysql::Connection& mysql,
    std::optional<uint64_t> submission_owner,
    uint64_t problem_id,
    std::optional<uint64_t> contest_problem_id,
    bool make_transaction = true
);

} // namespace sim::submissions
