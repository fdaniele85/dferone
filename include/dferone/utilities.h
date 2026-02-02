//
// Created by daniele on 27/11/23.
//

#pragma once

namespace dferone {

#define likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)

} // namespace dferone
