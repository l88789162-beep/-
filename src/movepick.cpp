#include "movepick.h"
int MovePicker::score_quiet(Move m) const {
  int score = history[piece(m)][to_sq(m)] / 8;
  score += (*contHist)[piece(m)][to_sq(m)] / 8;
  return score;
}

// search.cpp
#include "patch_improve.h"
//空窗剪枝
int nullThreshold = adaptive_nullmove_threshold(depth, staticEval);
if (staticEval >= beta + nullThreshold)
    return staticEval;
//LMR晚动归约
int reduction = adaptive_lmr_reduction(depth, moveIdx, isCapture);
//TT置换表替换判断
if (tt_should_replace(entry, depth, key))
    saveTT(key, score, depth, bound, move);
//SEE残局分数修正
int seeScore = see_endgame_weight(material, rawSee);
//将军招加分
score = check_move_bonus(givesCheck, score);

// movepick.cpp
#include "patch_improve.h"
//历史表动态权重
int histScore = dynamic_history_weight(piece, to, depth, baseHist);
//杀手棋分值衰减
int killerScore = killer_score_decay(killerAge, baseKillerScore);
//过河兵评估加分
bonus += quiet_pawn_bonus(sq, side);

//patch_improve.h
#ifndef PATCH_IMPROVE_H
#define PATCH_IMPROVE_H
#include "类型.h"
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

//PATCH_README.md
# Pikafish 8项优化补丁 patch_improve
## 文件清单
来源/patch_improve.h
来源/search.cpp（修改）
来源/movepick.cpp（修改）
来源/类型.h（原有依赖）
## 接入步骤
1. 确认patch_improve.h放置在源码根目录（来源文件夹）
2. search.cpp、movepick.cpp头部添加#include "patch_improve.h"
3. 替换原有对应逻辑为本套代码片段
4. 编译命令：make profile-build
## 优化项列表
1. dynamic_history_weight 历史表动态权重自适应，随搜索深度缩放历史分
2. quiet_pawn_bonus 过河兵阶梯加分，越靠近底线增益越高
3. adaptive_nullmove_threshold 空窗剪枝动态阈值，深度+静态评估双变量调节
4. adaptive_lmr_reduction LMR晚动归约，根据深度、移动序号自动调整归约层数，吃子不做归约
5. tt_should_replace 置换表条目覆盖策略，仅当新搜索深度显著更深时才覆盖旧条目
6. check_move_bonus 将军着额外打分，提升逼将路线优先度
7. killer_score_decay 杀手棋分值随迭代轮次衰减，淘汰过期杀手招
8. see_endgame_weight 残局SEE分数放大，残局阶段交换评估权重提升
## 测试方案
基准：原版Pikafish
指标：NPS每秒节点数、固定深度对弈Elo增益、短对局/长对局胜率拆分
bench命令：./pikafish bench
