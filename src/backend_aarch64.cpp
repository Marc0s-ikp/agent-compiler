#include <stdexcept>

#include "backend.hpp"

// Placeholder until the skeleton lands (D27 A1): selecting --target aarch64 reaches here.
void EmitAArch64(const ir::Module&, std::ostream&) {
    throw std::runtime_error("not in AArch64 yet: backend");
}
