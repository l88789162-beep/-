#include <iostream>

typedef int Move;
struct HistoryTable{};
struct ContinuationHistoryTable{};

struct MovePicker{
  int score_quiet(Move m) const;
  HistoryTable* history;
  ContinuationHistoryTable* contHist;
};

int MovePicker::score_quiet(Move m) const{
  return 0;
}

int search(){
  return 0;
}

int main(){
  std::cout << "chess engine start\n";
  return 0;
}
