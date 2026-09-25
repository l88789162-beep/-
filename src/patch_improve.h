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
//分层迭代历史衰减
inline int iter_hist_decay(int iterAge, int depth, int baseScore);
//延续历史置换过滤
inline int cont_hist_filter(uint64_t posKey, int rawContHist);
//空窗残局开关判定
inline bool nullmove_enable(int material);
//吃子分类LMR归约
inline int capture_lmr_reduction(Move m, int depth, int moveIdx);
//TT年龄置信计算
inline float tt_age_weight(int entryAge, int currIter);
//将军延伸层数限制
inline int check_ext_limit(int depth, int checkCnt);
//通路兵动态加权
inline int passed_pawn_dynamic(Square pawnSq, Square ourKing, Square oppKing);
//自适应SEE阈值
inline int see_cutoff_thresh(int materialLeft);
//LMP动态剪枝阈值
inline int lmp_dynamic_cut(int depth, int staticEval);

//8. see_endgame_weight：SEE残局权重修正
inline int see_endgame_weight(int material, int seeVal)
{
    if(material < 1400) return seeVal * 11 / 10;
    return seeVal;
}

#endif
inline bool nullmove_verify_degrade(int depth, int eval);
inline int hist_normalize(std::vector<int>& scores);
inline int capture_hist_decay(int age);
inline int branch_penalty_propagate(bool failHigh);
inline bool singular_prune_enable(int depth, int evalDiff);
inline int eval_smooth_filter(int oldEval, int newEval);
inline int transfer_hist(Board &b, uint64_t pawnKey);
inline bool sparse_move_sample(int moveIdx, int totalQuiet);
inline int king_side_weight(bool kingside, int mat);
inline uint64_t tt_prefetch_key(uint64_t posKey);
inline bool trade_lock_detect(Board &b);
inline void move_order_warmup(MovePicker &mp);
inline int negative_hist_amplify(int badScore);
inline int lmp_multi_stage(int depth, int staticEval);
inline void precompute_threats(Board &b);
inline int passed_pawn_cover_bonus(Square pawn, Board &b);
inline int window_restrict_limit(int nodeDepth);
inline bool tt_collision_store(uint64_t key, TTEntry entry);
inline uint8_t move_order_mask(Move m);
inline int endgame_king_mobility(Square kingSq, int materialLeft);

//51-78 轻量patch
inline float iter_hist_forget(int iterAge);
inline int move_hit_weight(bool pruneSuccess);
inline void pv_cache_share(MovePicker &mp);
inline bool nullmove_material_switch(int matLeft);
inline int lmp_index_correct(int moveIdx, int depth);
inline bool eval_outlier_detect(int eval, int mat);
inline void killer_group_store(Move killer, bool isCheckKiller);
inline int hist_peak_suppress(int score, int avgScore);
inline int cont_hist_depth_scale(int depth, int rawCont);
inline int see_recursion_cap(int depth);
inline int repeat_move_penalty(Move lastMove, Move currMove);
inline int branch_depth_reward(bool failHigh);
inline int window_dilate(int evalDelta);
inline bool tt_retry_read(TTEntry entry);
inline bool prefilter_capture_see(Move m);
inline bool iter_early_stop(int evalDelta);
inline int threat_quiet_bonus(Move m, Board &b);
inline int neg_hist_material_scale(int mat);
inline bool singular_ext_material_switch(int mat);
inline int sparse_sample_min_keep(int totalQuiet);
inline void pawn_hash_increment(Board &b, Move m);
inline int king_threat_decay(int threatAge);
inline int trade_depth_reduce(int mat);
inline int root_move_bias(Move m);
inline int nonpv_hist_decay(int rawHist);
inline int promo_see_precheck(Move m);
inline int prune_confidence_tag(bool isLmp, int depth);
//79-95评估重构
inline int passed_pawn_nonlinear(Square pawn, Square oppKing);
inline int isolated_pawn_dynamic(Square pawn, Square oppKing);
inline int doubled_pawn_discount(int mat);
inline int king_zone_threat(Square king, Board &b);
inline int piece_mobility_scale(Piece pc, int mat);
inline int bishop_color_penalty(Board &b);
inline int knight_terrain_bonus(Square knight, Board &b);
inline int rook_openline_scale(int mat);
inline int queen_development_penalty(Square qSq);
inline int activity_normalize(int totalAct, int mat);
inline int fortress_detect(Board &b);
inline int stalemate_risk_adjust(Board &b, int eval);
inline int pawn_break_bonus(Board &b);
inline int pinned_piece_value(Piece pc, Square sq, Board &b);
inline int semiopen_line_gradient(Square rook, Square king);
inline int king_opposition_eval(Board &b);
inline int endgame_template_lookup(Board &b);
//96-102底层架构
void lazy_smp_steal_priority();
void multi_level_tt_store(uint64_t key, TTEntry entry);
void root_depth_budget_alloc(std::vector<RootMove> &moves);
int hybrid_nn_eval(Board &b);
NodePool* node_pool_init();
void tt_async_prefetch(uint64_t posKey);
void multi_pv_parallel_search(std::vector<RootMove> &pvList);
inline int move_age_weight(int repeatCount);
inline float eval_variance(Node* node);
inline int check_quiet_hist_scale(int rawScore);
inline bool lmp_keep_backup(Move m);
inline int tt_bound_interpolate(TTEntry entry, int alpha, int beta);
inline int hist_saturation_damp(int rawScore);
inline bool root_iter_smooth_stop(int evalDelta, int iterCnt);
inline bool capture_see_reject(Move m);
inline int cont_hist_pv_separate(bool isPv, int rawCont);
inline int nullmove_margin_depth_scale(int depth);
inline void killer_expire_clean(int missIter);
inline int eval_material_baseline(int mat);
inline int neg_hist_severity(Piece captured);
inline bool singular_node_prune_lock(int depth);
inline int fork_bonus(Board &b);
inline int doubled_rook_scale(int mat);
inline int weak_pawn_penalty(Square sq, inline int eval_momentum_filter(int prevEval, int currEval);
inline int movecat_hist_decay(MoveType mt, int age);
inline int rfp_confidence_adjust(int lastError);
inline int nullmove_margin_tier(int depth);
inline int lmr_hist_reduction_correct(int histScore, int baseReduction);
inline int tt_move_priority_bonus(bool ttHit);
inline int neg_hist_cooldown(int iterAge);
inline int move_prune_stat_bonus(int pruneHitCnt);
inline int singular_ext_mat_cap(int matLeft);
inline void partial_hist_reset(Board &b);
inline bool killer_quick_verify(Move m, Board &b);
inline int window_min_gap(int depth);
inline bool pv_prune_lock(bool isPvNode);
inline int hanging_pawn_penalty(Square sq, Board &b);
inline int bishop_color_control_bonus(Board &b);
inline int trapped_knight_penalty(Square knSq, Board &b);
inline int rook_semiopen_king_scale(Square rSq, Square oppKing);
inline bool king_safety_enable(int enemyPieceCnt);
inline int multi_pawn_synergy(Board &b);
inline bool stalemate_static_check(Board &b);
inline bool deep_node_sample_skip(int depth);
inline int l1_l2_hist_lookup(uint64_t posKey, Move m);
inline int incremental_see(Board &b, Move m);
inline void thread_local_hist_init();
inline void root_prune_preheat(std::vector<RootMove> &moves);
Board &b);
inline int bishop_chain_bonus(Board &b);
inline int king_trap_bonus(Square oppKing, Board &b);
inline int block_pawn_activity_deduct(Square blocker, Square pawn);
inline int trade_eval_compensate(Board &b, int deltaMat);
inline int backrank_threat_penalty(Board &b);
inline int space_advantage(Board &b);
Move lazy_generate_move(Board &b, int idx);
uint64_t tt_entry_compress(TTEntry e);
int coarse_eval(Board &b);
void root_move_cluster(std::vector<RootMove> &moves);
int precomputed_see_lookup(Move m, Board &b);
inline int move_confidence_hist(int rawScore, bool verified);
inline int lmr_partial_rollback(int reduction, int failEval);
inline int nullmove_margin_sign_bias(int staticEval);
inline void hist_global_normalize(std::vector<int>& allHist);
inline void killer_depth_pool_store(int depth, Move killer);
inline bool rfp_safety_lock(Board &b);
inline int neg_hist_floor(int rawNegScore);
inline int tt_bound_smooth(int eval, int boundType);
inline bool singular_ext_material_floor(int mat);
inline bool cont_hist_move_valid(Board &b, Move m);
inline int failhigh_move_reprioritize(Move m);
inline int knight_pawn_support_bonus(Square kn, Square pawn, Board &b);
inline int pinned_queen_penalty(Square qSq, Board &b);
inline int kingside_space(Board &b);
inline int knnkp_endgame_correct(Board &b);
inline int pawn_gap_penalty(Board &b);
Move lazy_priority_gen(Board &b);
uint16_t hist_compress_encode(int score);
bool smp_task_priority_steal(NodeTask t);
void tt_hitrate_adjust_policy(float hitRate);
inline int hist_board_eval_damp(int rawHist, int staticDelta);
inline int lmr_reduction_floor(bool isCheckMove, int histScore);
inline int nullmargin_fail_counter_adjust(int failCnt);
inline int cont_hist_checknode_mask(bool inCheck);
inline int tt_entry_age_by_depth(int depth);
inline int killer_tt_merge_bonus(Move killer, Move ttMove);
inline bool rfp_noise_lock(int evalNoise);
inline bool singular_node_keep_all_moves();
inline void hist_baseline_pv_calibrate(std::vector<int>& histTable, Move pvMove);
inline int knight_semiopen_bonus(Square knSq, Board &b);
inline int rook_blocked_by_own_pawn(Square rSq, Board &b);
inline int pawn_promotion_countdown(Square pawnSq);
inline int bishop_color_total_control(Board &b);
void thread_local_tt_cache_init();
MoveList* cached_movelist_fetch(Board &b);


#endif
inline int iter_budget_alloc(int nodeType, int baseDepth);
inline int history_penalty(bool pruneFailed);
inline int pv_hist_scale(bool isPvNode, int rawHist);
inline bool rfp_allow(int depth, int evalDelta);
inline int king_safety_weight(int materialLeft);
inline bool early_draw_detect(uint64_t posKey, int repCount);
inline int promo_move_score(Move promoM);
inline int hist_clamp(int rawScore, int maxBound);
inline bool killer_valid_stat(int hitCount, int iterAge);
inline float eval_uncertainty(int material);
inline int nonpv_window_shrink(int depth);
inline uint64_t pawn_hash(Board &b);
inline int king_approach_penalty(Square pawn, Square oppKing);
inline bool asymptotic_see_stop(int gain, int cutoff);
