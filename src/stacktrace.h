#pragma once

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

using traced = boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace>;

template <class E>
inline void throw_with_trace(const E &e) {
    throw boost::enable_error_info(e) << traced(boost::stacktrace::stacktrace());
}
