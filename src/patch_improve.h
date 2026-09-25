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

#endif
