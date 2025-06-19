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

#include <thread>
#include <condition_variable>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "movegen.h"
#include "threads.h"


std::vector<Thread> Threads;


// Allocates memory for thread structs
void InitThreads(int count) 
{
    Threads.clear();
    Threads.resize(count);

    // Each thread knows its own index and total thread count
    for (int i = 0; i < count; ++i)
    {
        Threads[i].index = i,
        Threads[i].count = count;
    }
}

// Sorts all rootmoves beginning from the given index
void SortRootMoves(Thread *thread, int begin) {
    RootMove *rootMoves = thread->rootMoves;

    for (int i = begin + 1; i < thread->rootMoveCount; ++i) {
        RootMove temp = rootMoves[i];
        int j = i - 1;
        while (j >= 0 && rootMoves[j].score < temp.score) {
            rootMoves[j+1] = rootMoves[j];
            --j;
        }
        rootMoves[j+1] = temp;
    }
}

// Tallies the nodes searched by all threads
uint64_t TotalNodes() {
    uint64_t total = 0;
    for (const auto& t: Threads)
        total += t.pos.nodes;
    return total;
}

// Tallies the tbhits of all threads
uint64_t TotalTBHits() {
    uint64_t total = 0;
    for (const auto& t: Threads)
        total += t.tbhits;
    return total;
}

// Setup threads for a new search
void PrepareSearch(Position *pos, Move searchmoves[]) 
{
    MoveList legalMoves;
    legalMoves.count = legalMoves.next = 0;
    GenLegalMoves(pos, &legalMoves);

    RootMove rootMoves[256] = { 0 };
    int rootMoveCount = 0;

    // Add legal searchmoves to the root moves by checking if it is in the legalMoves list
    for (Move *move = searchmoves; *move; ++move)
        for (int i = 0; i < legalMoves.count; ++i)
            if (legalMoves.moves[i].move == *move)
                rootMoves[rootMoveCount++].move = *move;

    // If no searchmoves are provided, add all legal moves to the root moves
    if (!rootMoveCount)
        for (int i = 0; i < legalMoves.count; ++i)
            rootMoves[rootMoveCount++].move = legalMoves.moves[i].move;

    for (auto& t : Threads) {
        memset(&t, 0, offsetof(Thread, pos));
        memcpy(&t.pos, pos, sizeof(Position));
        memcpy(t.rootMoves, rootMoves, sizeof(rootMoves));
        t.rootMoveCount = rootMoveCount;
        for (Depth d = 0; d <= MAX_PLY; ++d)
            (t.ss+SS_OFFSET+d)->ply = d;
        for (Depth d = -7; d < 0; ++d)
            (t.ss+SS_OFFSET+d)->continuation = &t.continuation[0][0][EMPTY][0],
            (t.ss+SS_OFFSET+d)->contCorr = &t.contCorrHistory[EMPTY][0];
    }
}

static bool helpersActive = false;

// Start helper threads running the provided function
void StartHelpers(void (*func)(Thread*), std::vector<std::thread>* tasks) 
{
    helpersActive = true;
    tasks->clear();
    for (size_t i = 1; i < Threads.size(); i++) {
        tasks->push_back(std::thread(func, &Threads[i]));
    }
}

// Wait for helper threads to finish
void WaitForHelpers(std::vector<std::thread>* tasks) 
{
    if (helpersActive)
    {
        for (auto& t : *tasks)
            t.join();
        tasks->clear();
        helpersActive = false;
    }
}

// Reset all data that isn't reset each turn
void ResetThreads() 
{
    for (auto& t : Threads)
        memset(t.pawnCache,       0, sizeof(PawnCache)),
        memset(t.history,         0, sizeof(t.history)),
        memset(t.pawnHistory,     0, sizeof(t.pawnHistory)),
        memset(t.captureHistory,  0, sizeof(t.captureHistory)),
        memset(t.continuation,    0, sizeof(t.continuation)),
        memset(t.pawnCorrHistory, 0, sizeof(t.pawnCorrHistory)),
        memset(t.minorCorrHistory,0, sizeof(t.minorCorrHistory)),
        memset(t.majorCorrHistory,0, sizeof(t.majorCorrHistory)),
        memset(t.contCorrHistory, 0, sizeof(t.contCorrHistory));
}

// Run the given function once in each thread
void RunWithAllThreads(void (*func)(Thread *)) 
{
    std::vector<std::thread> tasks;
    for (auto& t : Threads)
        tasks.push_back(std::thread(func, &t));
    for (auto& task : tasks)
        task.join();
}

// Thread sleeps until it is woken up

std::condition_variable sleepCond;
std::mutex sleepMutex;

void Wait(std::atomic_bool* condition) {
    std::unique_lock<std::mutex> lock(sleepMutex);
    sleepCond.wait(lock, [&]{ return bool(*condition); });
}

// Wakes up a sleeping thread
void Wake() {
    std::lock_guard<std::mutex> lock(sleepMutex);
    sleepCond.notify_one();
}
