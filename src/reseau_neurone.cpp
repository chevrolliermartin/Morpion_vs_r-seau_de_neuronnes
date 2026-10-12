#include <Arduino.h>
#include "reseau_neurone.h"
#include "reseau_neurone_weights.h"

static void dense(const float *in, int n_in, int n_out,
                  const float *weights, const float *bias, float *out, bool relu)
{
    for (int o = 0; o < n_out; o++) {
        float s = bias[o];
        for (int i = 0; i < n_in; i++) s += weights[o * n_in + i] * in[i];
        out[o] = (relu && s < 0.0f) ? 0.0f : s;
    }
}

// b : format interne 0..8, 1 = réseau, -1 = adversaire, 0 = vide
// retourne 0..8, ou -1 si plein
int nn_core_best_move(const int8_t b[9])
{
    float x[9], h1[NN_H], h2[NN_H], y[9];
    for (int i = 0; i < 9; i++) x[i] = b[i];

    dense(x,  9,    NN_H, &NN_W1[0][0], NN_B1, h1, true);
    dense(h1, NN_H, NN_H, &NN_W2[0][0], NN_B2, h2, true);
    dense(h2, NN_H, 9,    &NN_W3[0][0], NN_B3, y,  false);

    int best = -1;
    for (int i = 0; i < 9; i++)
        if (b[i] == 0 && (best < 0 || y[i] > y[best])) best = i;
    return best;
}

int nn_core_move_level(const int8_t b[9], int err_pct)
{
    if (random(100) < err_pct) {
        int libres[9], n = 0;
        for (int i = 0; i < 9; i++)
            if (b[i] == 0) libres[n++] = i;
        if (n > 0) return libres[random(n)];
    }
    return nn_core_best_move(b);
}
