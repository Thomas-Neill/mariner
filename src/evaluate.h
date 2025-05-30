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


#define PAWN_CACHE_SIZE 128 * 1024

typedef struct PawnEntry {
    Key key;
    Bitboard passedPawns;
    int eval;
} PawnEntry;

typedef PawnEntry PawnCache[PAWN_CACHE_SIZE];


extern const int Tempo;
extern const int PieceValue[COLOR_NB][PIECE_NB];
extern const int PieceTypeValue[TYPE_NB];


// Tapered Eval
enum Phase { MG, EG };

static const int MidGame = 256;
extern const int PhaseValue[TYPE_NB];

#define S(mg, eg) ((int)((unsigned int)(eg) << 16) + (mg))
#define MgScore(s) ((int16_t)((uint16_t)((unsigned)((s)))))
#define EgScore(s) ((int16_t)((uint16_t)((unsigned)((s) + 0x8000) >> 16)))

// Calculates the phase from the phase values of the pieces left
INLINE int UpdatePhase(int value) {
    return (value * MidGame + 12) / 24;
}

// Returns a static evaluation of the position from the side to move's point of view
int EvalPosition(const Position *pos, PawnCache pc);

// Returns a static evaluation of the position from whites point of view
INLINE int EvalPositionWhitePov(const Position *pos, PawnCache pc) {
    int score = EvalPosition(pos, pc);
    return sideToMove == WHITE ? score : -score;
}

extern const int PieceSqValue[6][64];

// Misc
extern const int PawnDoubled;
extern const int PawnDoubled2;
extern const int PawnIsolated;
extern const int PawnSupport;
extern const int PawnThreat;
extern const int PushThreat;
extern const int PawnOpen;
extern const int BishopPair;
extern const int KingAtkPawn;
extern const int OpenForward;
extern const int SemiForward;
extern const int NBBehindPawn;
extern const int BishopBadP;
extern const int Shelter;

// Passed pawn
extern const int PawnPassed[RANK_NB];
extern const int PassedDefended[RANK_NB];
extern const int PassedBlocked[4];
extern const int PassedFreeAdv[4];
extern const int PassedDistUs[4];
extern const int PassedDistThem;
extern const int PassedRookBack;
extern const int PassedSquare;

// Pawn phalanx
extern const int PawnPhalanx[RANK_NB];


extern const int ThreatByMinor[8];
extern const int ThreatByRook[8];

// KingLineDanger
extern const int KingLineDanger[28];

// Mobility
extern const int Mobility[4][28];
