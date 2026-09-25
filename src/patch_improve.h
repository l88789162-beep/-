#ifndef PATCH_IMPROVE_H
#define PATCH_IMPROVE_H

#include "types.h"
#include "search.h"
#include "position.h"
#include "movepicker.h"
#include "tt.h"

//优化1：历史表权重动态自适应，深度越高权重放大，浅搜索降低噪声
inline int dynamic_history_weight(int depth, int baseScore)
{
    int scale = std::clamp(depth * 12, 32, 128);
    return (baseScore * scale) / 64;
}

//优化2：兵/卒过河加分，中局河界子力增益
inline int quiet_pawn_bonus(const Position& pos, Square sq, Color c)
{
    int bonus = 0;
    int r = rank_of(sq);
    if(c == RED)
    {
        if(r >= RANK_5) bonus += 18;
    }else{
        if(r <= RANK_4) bonus +=18;
    }
    return bonus;
}

//优化3：空窗剪枝自适应阈值，子力少时关闭空窗防止漏杀
inline bool adaptive_nullmove_threshold(const Position& pos, int depth, Value beta)
{
    Value material = pos.material();
    Value matLimit = (depth > 4) ? 280 : 160;
    if(material < matLimit) return false;
    return true;
}

//优化4：LMR晚动归约自适应调整，避免过度归约丢失杀线
inline int adaptive_lmr_reduction(const Position& pos, int depth, int moveNum, Value score)
{
  int baseRed = 1;
  if(depth >= 3 && moveNum >= 4)
  {
    baseRed = 1 + (depth / 3);
    if(score < VALUE_MATE / 2) baseRed += 1;
  }
  return std::min(baseRed, 3);
}

//优化5：置换表条目替换策略，优先保留高深度记录
inline bool tt_should_replace(const TTEntry* entry, int newDepth)
{
  if(!entry->key()) return true;
  int oldDepth = entry->depth();
  if(newDepth > oldDepth + 1) return true;
  return false;
}

//优化6：将军招额外加分，提升杀线搜索优先级
inline int check_move_bonus(bool givesCheck)
{
  return givesCheck ? 60 : 0;
}

//优化7：杀手棋分值动态衰减，清除旧杀手棋对浅层搜索的干扰
inline int killer_score_decay(int depth, int baseScore)
{
    float factor = 1.0f / (1 + depth * 0.12f);
    return static_cast<int>(baseScore * factor);
}


//优化8：SEE静态交换评估残局权重微调，残局对子判断更精准
inline int see_endgame_weight(Value value, const Position& pos)
{
    if(pos.material() < 1200)
    {
        return static_cast<int>(value * 0.92);
    }
    return static_cast<int>(value);
}

#ifndef PATCH_IMPROVE_H
#define PATCH_IMPROVE_H
#include "pikafish.h"

//1. dynamic_history_weight：历史表权重动态自适应
inline int dynamic_history_weight(Piece piece, Square to, int depth, int baseScore)
{
    int scale = std::clamp(depth,1,16);
    float factor = 0.4f + 0.6f * (scale / 16.0f);
    return static_cast<int>(baseScore * factor);
}

//2. quiet_pawn_bonus：过河兵卒加分项
inline int quiet_pawn_bonus(Square sq, Color side)
{
    int r = rank_of(sq);
    if(side == WHITE) r = 7 - r;
    if(r >= 4) return 12 * (r - 3);
    return 0;
}

//3. adaptive_nullmove_threshold：空窗剪枝自适应阈值
inline int adaptive_nullmove_threshold(int depth, int staticEval)
{
    int base = 280;
    int depthAdj = depth * 22;
    int evalAdj = staticEval / 12;
    return base + depthAdj + evalAdj;
}

//4. adaptive_lmr_reduction：LMR晚动归约自适应
inline int adaptive_lmr_reduction(int depth, int moveIdx, bool isCapture)
{
    if(isCapture) return 0;
    int red = 1 + int(log2(depth)) + int(log2(moveIdx + 1));
    return std::max(1, red);
}

//5. tt_should_replace：置换表替换策略
inline bool tt_should_replace(TTEntry entry, int depth, uint64_t key)
{
    if(entry.key != key) return true;
    return entry.depth < depth - 2;
}

//6. check_move_bonus：将军招加分
inline int check_move_bonus(bool givesCheck, int base)
{
    return givesCheck ? base + 65 : base;
}

//7. killer_score_decay：杀手棋分值动态衰减
inline int killer_score_decay(int age, int baseScore)
{
    float decay = pow(0.82f, age);
    return static_cast<int>(baseScore * decay);
}

//8. see_endgame_weight：SEE残局权重修正
inline int see_endgame_weight(int material, int seeVal)
{
    if(material < 1400) return seeVal * 11 / 10;
    return seeVal;
}

#endif



#endif
