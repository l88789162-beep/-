#ifndef TYPES_H
#define TYPES_H
#include <cstdint>
using Move = uint16_t;
using Square = uint8_t;
using Piece = uint8_t;
using HistoryTable = int[16][64];
using ContinuationHistoryTable = int[16][64];
inline Piece piece(Move m){return (m >> 12) & 0x0F;}
inline Square to_sq(Move m){return m & 0x3F;}
#endif
