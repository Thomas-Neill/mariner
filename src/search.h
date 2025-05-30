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

#include "board.h"
#include "types.h"


typedef struct {
    TimePoint start;
    int time, inc, movestogo, movetime, depth;
    uint64_t nodes;
    int optimalUsage, maxUsage;
    int mate;
    bool timelimit, nodeTime, infinite;
    Move searchmoves[64];
    int multiPV;
} SearchLimits;


extern SearchLimits Limits;
extern std::atomic_bool ABORT_SIGNAL;
extern std::atomic_bool SEARCH_STOPPED;
extern std::atomic_bool Minimal;


void *SearchPosition(void *pos);
