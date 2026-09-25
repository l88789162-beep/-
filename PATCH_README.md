# Pikafish 一体化算法补丁 patch_improve.h
This patch is built on official Pikafish source code, licensed under GPLv3.
All optimizations are plug-and-play, no modification to original core board/movegen code.

## Optimizations List
1. Dynamic scaling for history table weight, reduce noise for shallow search
2. River pawn/pawn cross river bonus for quiet move scoring
3. Adaptive null-move pruning threshold, disable nullmove when material is low
4. Adaptive LMR late move reduction, prevent over-reduction and missing mate lines
5. Smarter Transposition Table entry replacement rule, keep deeper depth records
6. Extra bonus for checking moves, raise priority of mating lines
7. Dynamic killer move score decay, remove stale killer interference
8. Endgame weighted SEE for capture static exchange evaluation

## How to integrate
1. Put patch_improve.h into src folder of Pikafish source
2. Add `#include "patch_improve.h"` at top of search.cpp, movepicker.cpp, tt.cpp, see.cpp
3. Replace corresponding original function call:
- history score: score = dynamic_history_weight(depth, historyScore);
- pawn eval: val += quiet_pawn_bonus(pos, sq, side);
- nullmove: if(adaptive_nullmove_threshold(pos, depth, beta)){ nullmove search }
- LMR: int r = adaptive_lmr_reduction(pos, depth, moveCount, value);
- TT replace: if(tt_should_replace(ttEntry, depth)) store entry;
- check bonus: score += check_move_bonus(givesCheck);
- killer score: score += killer_score_decay(depth, killerScore);
- SEE result: res = see_endgame_weight(res, pos);
4. Compile: `make -j`
