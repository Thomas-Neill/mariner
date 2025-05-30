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

#include "board.h"
#include "evaluate.h"
#include "psqt.h"



int PSQT[PIECE_NB][64];

// Black's point of view - easier to read as it's not upside down
const int PieceSqValue[6][64] = {

    { S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0),
      S( 57,  1), S( 49, 22), S( 34, 65), S( 74, 31), S( 76, 36), S( 85, 27), S(-34, 91), S(-49, 63),
      S( 15, 77), S( 14, 84), S( 42, 43), S( 47, 17), S( 64, 21), S(125, 22), S( 98, 68), S( 43, 76),
      S(-15, 43), S(-13, 20), S( -8,  2), S( -1,-22), S( 21,-15), S( 29,-14), S(  5, 13), S(  8, 18),
      S(-27, 15), S(-32, 10), S(-17, -8), S( -8,-18), S( -2,-15), S(  3,-15), S(-15, -5), S(-10, -7),
      S(-34,  4), S(-35, -2), S(-27, -5), S(-24, -7), S(-14,  0), S(-13, -1), S(  2,-16), S(-12,-14),
      S(-19, 12), S( -9, 13), S(-11,  8), S(-11, 16), S( -4, 31), S( 15,  9), S( 31, -2), S( -6,-13),
      S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0), S(  0,  0) },

    { S(-201,-70), S(-116,-15), S(-145, 31), S(-58,  1), S( -7,  9), S(-133, 34), S(-83,-15), S(-153,-115),
      S( -9,-24), S( -6,  3), S( 38,-11), S( 46, 16), S( 52, -2), S( 69,-34), S(-12,  0), S(  8,-47),
      S(-15, -7), S( 23,  7), S( 24, 46), S( 43, 45), S( 84, 20), S( 69, 23), S( 33,-15), S( -3,-25),
      S( 13,  6), S( 23, 23), S( 45, 50), S( 52, 62), S( 40, 57), S( 72, 39), S( 25, 17), S( 39, -7),
      S( 10, 12), S( 22, 14), S( 27, 52), S( 36, 49), S( 35, 54), S( 44, 41), S( 48, 11), S( 30, 14),
      S(-13,-31), S( -5,  6), S(  9, 22), S( 12, 44), S( 24, 40), S( 17, 16), S( 21,  4), S( 11, -8),
      S(-32,-29), S(-30,  1), S(-13, -5), S(  0, 16), S(  0,  9), S(-10, -5), S(-15,-10), S( -1,  0),
      S(-82,-53), S(-18,-37), S(-30, -9), S( -1, 12), S(  5, 14), S(  5,-16), S(-14,-12), S(-45,-30) },

    { S(-35, 47), S(-57, 38), S(-128, 54), S(-126, 56), S(-128, 52), S(-143, 43), S(-28, 21), S(-44, 18),
      S(-14, 15), S( 10, 18), S(  1, 19), S(-25, 30), S( -2, 13), S(-15, 25), S(-28, 27), S(-56, 23),
      S(  2, 26), S( 28, 17), S( 43, 12), S( 32,  9), S( 37, 12), S( 52, 24), S( 19, 24), S( 19, 12),
      S( -7, 16), S( 29, 14), S( 27, 15), S( 49, 40), S( 39, 25), S( 31, 19), S( 33,  7), S( -5, 17),
      S( 13,  0), S( 10,  5), S( 23, 23), S( 39, 29), S( 29, 25), S( 34, 11), S( 16,  7), S( 41,-11),
      S(  9, -3), S( 43, 21), S( 28, 18), S( 18, 27), S( 27, 32), S( 33, 20), S( 47,  9), S( 37, -9),
      S( 30, -6), S( 24,-23), S( 29,-12), S(  5, 12), S( 14, 12), S( 16, -3), S( 36,-19), S( 35,-44),
      S( 26,-16), S( 41,  4), S( 25, 17), S(  6, 15), S( 21, 14), S( 15, 13), S( 29, -2), S( 39,-27) },

    { S( 24, 56), S( 24, 67), S(-12, 88), S( -8, 76), S(  3, 75), S(  7, 77), S( 25, 73), S( 40, 64),
      S(-13, 63), S(-26, 81), S( -1, 83), S(  7, 76), S( -4, 72), S(  8, 52), S( -3, 56), S( 19, 42),
      S(-13, 62), S( 39, 47), S( 15, 58), S( 39, 35), S( 57, 24), S( 52, 32), S( 81, 17), S( 23, 35),
      S(-13, 67), S(  8, 59), S( 14, 59), S( 32, 37), S( 27, 31), S( 33, 25), S( 35, 27), S( 16, 34),
      S(-22, 51), S(-26, 58), S(-23, 55), S(-16, 47), S(-18, 41), S(-30, 38), S( -1, 29), S(-15, 26),
      S(-25, 32), S(-23, 36), S(-21, 35), S(-18, 30), S(-13, 26), S(-11, 11), S( 14, -3), S( -8,  5),
      S(-35, 33), S(-16, 31), S( -5, 33), S( -4, 25), S(  2, 17), S(-14, 11), S(  5,  4), S(-31, 21),
      S(-11, 38), S(-13, 35), S(-11, 39), S( -1, 21), S(  4, 14), S(  2, 21), S(  5, 20), S(-11, 20) },

    { S(-42, 81), S(-21, 93), S(-12,125), S(  4,130), S( -3,148), S( 35,138), S( 33,125), S( 18,115),
      S( -9, 54), S(-48,103), S(-28,113), S(-84,202), S(-88,236), S(-28,170), S(-43,163), S( 11,146),
      S( -3, 50), S(  5, 40), S(  1, 89), S(-10,128), S( -3,159), S( 29,157), S( 49,114), S( 13,140),
      S(  6, 25), S( 13, 65), S( -1, 79), S(-11,130), S(-14,162), S( -9,171), S( 27,155), S(  9,123),
      S( 20,  5), S(  5, 50), S(  5, 57), S(  0, 99), S( -1,104), S(  6, 82), S( 26, 70), S( 29, 69),
      S( 10,-21), S( 18, 10), S(  7, 37), S(  7, 32), S( 13, 35), S( 18, 32), S( 43,  1), S( 31, -7),
      S( 13,-31), S( 16,-22), S( 22,-26), S( 23,  8), S( 26, -1), S( 22,-83), S( 40,-115), S( 33,-73),
      S( -2,-39), S( -9,-38), S(  1,-31), S( 11,-35), S(  7,-37), S( -9,-44), S(  7,-77), S( 14,-55) },

    { S(-24,-68), S( 23, 15), S( 10, 45), S(  7, 82), S(  1, 62), S( 26, 76), S( 45, 91), S( 16,-57),
      S( -7, 34), S( 52,112), S( 53,118), S( 81,107), S( 85,108), S( 94,125), S( 98,139), S( 47, 54),
      S( 31, 63), S(129,112), S(114,133), S( 91,148), S(133,144), S(165,148), S(150,133), S( 49, 72),
      S( 33, 59), S(102, 93), S( 95,135), S( 58,164), S( 71,164), S(123,139), S(114,111), S(  0, 79),
      S( 25, 31), S(114, 68), S(119,107), S( 36,146), S( 79,134), S(114,108), S(111, 74), S(-41, 68),
      S( 32, 25), S(117, 55), S( 86, 83), S( 63,103), S( 81, 99), S( 69, 90), S( 86, 67), S( -3, 52),
      S( 85, 25), S( 84, 50), S( 72, 66), S( 19, 86), S( 31, 86), S( 47, 75), S( 84, 50), S( 60, 25),
      S( 37,-41), S( 89, -1), S( 64, 30), S(-39, 43), S( 33, 17), S(-18, 50), S( 66,  3), S( 36,-41) },
};

// Initialize the piece square tables with piece values included
CONSTR(1, InitPSQT) {
    for (PieceType pt = PAWN; pt <= KING; ++pt)
        for (Square sq = A1; sq <= H8; ++sq) {
            int value = PieceTypeValue[pt] + PieceSqValue[pt-1][sq];
            PSQT[MakePiece(WHITE, pt)][MirrorSquare(sq)] =  value;
            PSQT[MakePiece(BLACK, pt)][             sq ] = -value;
        }
}
