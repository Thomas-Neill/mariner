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

#include "bitboard.h"
#include "board.h"
#include "evaluate.h"
#include "makemove.h"
#include "move.h"
#include "psqt.h"
#include "transposition.h"


#define HASH_PCE(piece, sq) (pos->key ^= PieceKeys[(piece)][(sq)])
#define HASH_CA             (pos->key ^= CastleKeys[pos->castlingRights])
#define HASH_SIDE           (pos->key ^= SideKey)
#define HASH_EP             (pos->key ^= PieceKeys[EMPTY][pos->epSquare])


// Remove a piece from a square sq
static void ClearPiece(Position *pos, const Square sq, const bool hash) {

    const Piece piece = pieceOn(sq);
    const Color color = ColorOf(piece);
    const PieceType pt = PieceTypeOf(piece);

    assert(ValidPiece(piece));

    // Hash out the piece
    if (hash)
        HASH_PCE(piece, sq);

    if (PieceTypeOf(piece) == PAWN)
        pos->pawnKey ^= PieceKeys[piece][sq];
    else {
        pos->nonPawnKey[color] ^= PieceKeys[piece][sq];

        if (pt == KING) {
            pos->minorKey ^= PieceKeys[piece][sq];
            pos->majorKey ^= PieceKeys[piece][sq];
        } else if (pt >= ROOK)
            pos->majorKey ^= PieceKeys[piece][sq];
        else
            pos->minorKey ^= PieceKeys[piece][sq];
    }

    // Set square to empty
    pieceOn(sq) = EMPTY;

    // Update material
    pos->material -= PSQT[piece][sq];

    // Update phase
    pos->phaseValue -= PhaseValue[pt];
    pos->phase = UpdatePhase(pos->phaseValue);

    // Update non-pawn count
    pos->nonPawnCount[color] -= NonPawn[piece];

    // Update bitboards
    pieceBB(ALL)   ^= BB(sq);
    pieceBB(pt)    ^= BB(sq);
    colorBB(color) ^= BB(sq);

    pos->materialKey ^= PieceKeys[piece][PieceCount(pos, piece)];
}

// Add a piece piece to a square
static void AddPiece(Position *pos, const Square sq, const Piece piece, const bool hash) {

    const Color color = ColorOf(piece);
    const PieceType pt = PieceTypeOf(piece);

    // Hash in piece at square
    if (hash)
        HASH_PCE(piece, sq);

    pos->materialKey ^= PieceKeys[piece][PieceCount(pos, piece)];

    if (PieceTypeOf(piece) == PAWN)
        pos->pawnKey ^= PieceKeys[piece][sq];
    else {
        pos->nonPawnKey[color] ^= PieceKeys[piece][sq];

        if (pt == KING) {
            pos->minorKey ^= PieceKeys[piece][sq];
            pos->majorKey ^= PieceKeys[piece][sq];
        } else if (pt >= ROOK)
            pos->majorKey ^= PieceKeys[piece][sq];
        else
            pos->minorKey ^= PieceKeys[piece][sq];
    }

    // Update square
    pieceOn(sq) = piece;

    // Update material
    pos->material += PSQT[piece][sq];

    // Update phase
    pos->phaseValue += PhaseValue[pt];
    pos->phase = UpdatePhase(pos->phaseValue);

    // Update non-pawn count
    pos->nonPawnCount[color] += NonPawn[piece];

    // Update bitboards
    pieceBB(ALL)   |= BB(sq);
    pieceBB(pt)    |= BB(sq);
    colorBB(color) |= BB(sq);
}

// Move a piece from one square to another
static void MovePiece(Position *pos, const Square from, const Square to, const bool hash) {

    const Piece piece = pieceOn(from);
    const Color color = ColorOf(piece);
    const PieceType pt = PieceTypeOf(piece);

    assert(ValidPiece(piece));
    assert(pieceOn(to) == EMPTY || (Chess960 && pt == ROOK));

    // Hash out piece on old square, in on new square
    if (hash)
        HASH_PCE(piece, from),
        HASH_PCE(piece, to);

    if (PieceTypeOf(piece) == PAWN)
        pos->pawnKey ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];
    else {
        pos->nonPawnKey[color] ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];

        if (pt == KING) {
            pos->minorKey ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];
            pos->majorKey ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];
        } else if (pt >= ROOK)
            pos->majorKey ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];
        else
            pos->minorKey ^= PieceKeys[piece][from] ^ PieceKeys[piece][to];
    }

    // Set old square to empty, new to piece
    pieceOn(from) = EMPTY;
    pieceOn(to)   = piece;

    // Update material
    pos->material += PSQT[piece][to] - PSQT[piece][from];

    // Update bitboards
    pieceBB(ALL)   ^= BB(from) ^ BB(to);
    pieceBB(pt)    ^= BB(from) ^ BB(to);
    colorBB(color) ^= BB(from) ^ BB(to);
}

// Take back the previous move
void TakeMove(Position *pos) {

    // Incremental updates
    pos->histPly--;
    sideToMove ^= 1;

    // Get the move from history
    const Move move = history(0).move;
    const Square from = fromSq(move);
    const Square to = toSq(move);

    // Add in pawn captured by en passant
    if (moveIsEnPas(move))
        AddPiece(pos, to ^ 8, MakePiece(!sideToMove, PAWN), false);

    // Move rook back if castling
    else if (moveIsCastle(move)) {
        ClearPiece(pos, to, false);
        switch (to) {
            case C1: MovePiece(pos, D1, RookSquare[WHITE_OOO], false); break;
            case C8: MovePiece(pos, D8, RookSquare[BLACK_OOO], false); break;
            case G1: MovePiece(pos, F1, RookSquare[WHITE_OO ], false); break;
            default: MovePiece(pos, F8, RookSquare[BLACK_OO ], false); break;
        }
        AddPiece(pos, from, MakePiece(sideToMove, KING), false);
        goto done;
    }
    {

    // Make reverse move (from <-> to)
    MovePiece(pos, to, from, false);

    // Add back captured piece if any
    Piece capt = capturing(move);
    if (capt != EMPTY) {
        assert(ValidCapture(capt));
        AddPiece(pos, to, capt, false);
    }

    // Remove promoted piece and put back the pawn
    Piece promo = promotion(move);
    if (promo != EMPTY) {
        assert(ValidPromotion(promo));
        ClearPiece(pos, from, false);
        AddPiece(pos, from, MakePiece(sideToMove, PAWN), false);
    }
    }

done:

    // Get various info from history
    pos->key            = history(0).key;
    pos->materialKey    = history(0).materialKey;
    pos->checkers       = history(0).checkers;
    pos->epSquare       = history(0).epSquare;
    pos->rule50         = history(0).rule50;
    pos->castlingRights = history(0).castlingRights;

    assert(PositionOk(pos));
}

// Make a move - take it back and return false if move was illegal
void MakeMove(Position *pos, const Move move) {

    TTPrefetch(KeyAfter(pos, move));

    // Save position
    history(0).key            = pos->key;
    history(0).materialKey    = pos->materialKey;
    history(0).checkers       = pos->checkers;
    history(0).move           = move;
    history(0).epSquare       = pos->epSquare;
    history(0).rule50         = pos->rule50;
    history(0).castlingRights = pos->castlingRights;

    // Incremental updates
    pos->histPly++;
    pos->rule50++;

    // Hash out en passant if there was one, and unset it
    HASH_EP;
    pos->epSquare = 0;

    const Square from = fromSq(move);
    const Square to = toSq(move);

    // Rehash the castling rights
    HASH_CA;
    pos->castlingRights &= CastlePerm[from] & CastlePerm[to];
    HASH_CA;

    if (moveIsCastle(move)) {
        ClearPiece(pos, from, true);
        switch (to) {
            case C1: MovePiece(pos, RookSquare[WHITE_OOO], D1, true); break;
            case C8: MovePiece(pos, RookSquare[BLACK_OOO], D8, true); break;
            case G1: MovePiece(pos, RookSquare[WHITE_OO ], F1, true); break;
            default: MovePiece(pos, RookSquare[BLACK_OO ], F8, true); break;
        }
        AddPiece(pos, to, MakePiece(sideToMove, KING), true);
        goto done;
    }
    {

    // Remove captured piece if any
    Piece capt = capturing(move);
    if (capt != EMPTY) {
        assert(ValidCapture(capt));
        ClearPiece(pos, to, true);
        pos->rule50 = 0;
    }

    // Move the piece
    MovePiece(pos, from, to, true);

    // Pawn move specifics
    if (pieceTypeOn(to) == PAWN) {

        pos->rule50 = 0;
        Piece promo = promotion(move);

        // Set en passant square if applicable
        if (moveIsPStart(move)) {
            if (  PawnAttackBB(sideToMove, to ^ 8)
                & colorPieceBB(!sideToMove, PAWN)) {

                pos->epSquare = to ^ 8;
                HASH_EP;
            }

        // Remove pawn captured by en passant
        } else if (moveIsEnPas(move))
            ClearPiece(pos, to ^ 8, true);

        // Replace promoting pawn with new piece
        else if (promo != EMPTY) {
            assert(ValidPromotion(promo));
            ClearPiece(pos, to, true);
            AddPiece(pos, to, promo, true);
        }
    }
    }

done:

    // Change turn to play
    sideToMove ^= 1;
    HASH_SIDE;

    pos->checkers = Checkers(pos);
    pos->nodes++;

    assert(PositionOk(pos));
}

// Pass the turn without moving
void MakeNullMove(Position *pos) {

    // Save misc info for takeback
    history(0).key            = pos->key;
    history(0).move           = NOMOVE;
    history(0).epSquare       = pos->epSquare;
    history(0).rule50         = pos->rule50;
    history(0).castlingRights = pos->castlingRights;

    // Incremental updates
    pos->histPly++;
    pos->rule50 = 0;
    sideToMove ^= 1;
    HASH_SIDE;

    // Hash out en passant if there was one, and unset it
    HASH_EP;
    pos->epSquare = 0;

    TTPrefetch(pos->key);

    assert(PositionOk(pos));
}

// Take back a null move
void TakeNullMove(Position *pos) {

    // Incremental updates
    pos->histPly--;
    sideToMove ^= 1;

    // Get info from history
    pos->key      = history(0).key;
    pos->epSquare = history(0).epSquare;
    pos->rule50   = history(0).rule50;

    assert(PositionOk(pos));
}
