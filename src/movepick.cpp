#include "movepick.h"
int MovePicker::score_quiet(Move m) const {
  int score = history[piece(m)][to_sq(m)] / 8;
  score += (*contHist)[piece(m)][to_sq(m)] / 8;
  return score;
}

