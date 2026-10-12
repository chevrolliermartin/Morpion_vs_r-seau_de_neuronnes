#pragma once
#include <stdint.h>

// ---------------------------------------------------------------------------
// Convention de VOTRE tableau (indices 1..9, board[0] ignoré -> déclarer 10 cases) :
//    0  = case vide
//    1  = case jouée par le joueur humain
//   -1  = case jouée par le réseau
//
// Cases :  1 2 3 / 4 5 6 / 7 8 9
//
// Les fonctions retournent la case 1..9 à jouer, ou 0 si le plateau est plein.
// Le tableau n'est jamais modifié : faites board[coup] = -1.
// Accepte n'importe quel type de tableau (int, int8_t, char, float...),
// const ou non.
// ---------------------------------------------------------------------------

// --- Fonctions internes (0..8, 1 = "moi", -1 = adversaire), voir nn.cpp ---
int nn_core_best_move(const int8_t b[9]);
int nn_core_move_level(const int8_t b[9], int err_pct);

// Conversion : votre tableau 1..9 -> format interne 0..8 vu par le réseau
// (le réseau se voit lui-même comme +1, donc on inverse le signe)
template <typename T>
static inline void nn_convert(const T *board, int8_t out[9])
{
    for (int i = 0; i < 9; i++) {
        T v = board[i + 1];
        out[i] = (v == 0) ? 0 : (v > 0 ? -1 : 1);
    }
}

// Meilleur coup du réseau (case 1..9, 0 si plateau plein)
template <typename T>
int nn_best_move(const T *board)
{
    int8_t b[9];
    nn_convert(board, b);
    return nn_core_best_move(b) + 1;   // -1 (plein) devient 0
}

// Idem avec err_pct % de coups au hasard (0 = parfait, 100 = tout au hasard)
template <typename T>
int nn_move_level(const T *board, int err_pct)
{
    int8_t b[9];
    nn_convert(board, b);
    return nn_core_move_level(b, err_pct) + 1;
}
