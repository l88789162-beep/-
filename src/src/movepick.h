#ifndef MOVEPICK_H
#define MOVEPICK_H
#include "types.h"
struct MovePicker{
  int score_quiet(Move m) const;
  HistoryTable* history;
  ContinuationHistoryTable* contHist;
};
#endif
