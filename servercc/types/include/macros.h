#ifndef SERVERCC_MACROS_H
#define SERVERCC_MACROS_H

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

// Concatenates two tokens.
#ifndef _CONCAT_IMPL
#define _CONCAT_IMPL(a, b) a##b
#endif

// Concatenates two tokens.
#ifndef CONCAT
#define CONCAT(a, b) _CONCAT_IMPL(a, b)
#endif

// Creates a variable name that is unique to the line number.
#ifndef VAR_LINENO
#define VAR_LINENO(var) CONCAT(var, __LINE__)
#endif

// Asserts that the specified expr is ok and returns the status if it is not logging the error.
// If any var args are provided they will added to the status message.
#ifndef ASSERT_OK
#define ASSERT_OK(expr, ...)                                                                 \
    auto VAR_LINENO(_status) = (expr);                                                       \
    if (!VAR_LINENO(_status).ok()) {                                                         \
        return absl::Status(VAR_LINENO(_status).code(),                                      \
                            absl::StrCat(__VA_ARGS__, ": ", VAR_LINENO(_status).message())); \
    }
#endif

// Asserts that the specified expr is ok and returns the status if it is not logging the error. If
// the status is ok assigns the value to the specified variable. Return of the expression must be
// pair<absl::Status, T>.
#ifndef ASSERT_OK_AND_ASSIGN
#define ASSERT_OK_AND_ASSIGN(variable, expr, ...)                                                  \
    auto VAR_LINENO(_status) = (expr);                                                             \
    if (!VAR_LINENO(_status).first.ok()) {                                                         \
        return absl::Status(VAR_LINENO(_status).first.code(),                                      \
                            absl::StrCat(__VA_ARGS__, ": ", VAR_LINENO(_status).first.message())); \
    }                                                                                              \
    auto variable = std::move(VAR_LINENO(_status).second);
#endif

#endif  // SERVERCC_MACROS_H
