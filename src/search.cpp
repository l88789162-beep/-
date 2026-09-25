src/search.cpp
#include "search.h"
// Singular Extension & LMR modified code
// Singular margin reduced from 3*ONE_PLY to 2*ONE_PLY
bool singular(Position& pos, Stack* ss, Value beta, Depth depth, Move skipMove)
{
        const Depth singularReduction = depth > 6 ? 2 * ONE_PLY : ONE_PLY;
            const Value singularMargin = 2 * ONE_PLY;
                Value rBeta = beta - singularMargin;
                    Depth rDepth = depth - singularReduction;
                        Value rValue = search<NON_PV>(pos, ss+1, rBeta-1, rDepth, false);
                            return rValue < rBeta;
}

Depth lmr_reduction(bool isPv, bool isCapture, bool isCheck, Depth depth, int moveCnt)
{
        if (isPv || isCapture || isCheck) return 0;
            double base = 0.5 + 0.12 * log(depth) * log(moveCnt);
                return static_cast<Depth>(base * ONE_PLY);
}

}