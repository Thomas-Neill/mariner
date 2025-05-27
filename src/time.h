/*
  Weiss is a UCI compliant chess engine.
  Copyright (C) 2023 Terje Kirstihagen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <chrono>
#include "types.h"


typedef std::chrono::high_resolution_clock::time_point TimePoint;
struct Thread;

inline TimePoint Now()
{
	return std::chrono::high_resolution_clock::now();
}
inline uint32_t millisecs(const TimePoint& t1, const TimePoint& t2)
{
	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    auto count = time_span.count();
	return static_cast<uint32_t>((count > 0 ? count : -count) * 1000.0);
}
inline int TimeSince(const TimePoint& start)
{
    return static_cast<int>(millisecs(start, Now()));
}

void InitTimeManagement();
bool OutOfTime(Thread *thread);
