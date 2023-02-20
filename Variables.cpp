#include "Variables.h"

static float vertices[] = {


    -10.0f, -10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
     10.0f, -10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
     10.0f,  10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
     10.0f,  10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    -10.0f,  10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
    -10.0f, -10.0f,  10.0f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,

    -10.0f,  10.0f,  10.0f,     -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
    -10.0f,  10.0f, -1000.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 40.0f,
    -10.0f, -10.0f, -1000.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 40.0f,
    -10.0f, -10.0f, -1000.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 40.0f,
    -10.0f, -10.0f,  10.0f,     -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
    -10.0f,  10.0f,  10.0f,     -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

     10.0f,  10.0f,  10.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     10.0f,  10.0f, -1000.0f, 1.0f,  0.0f,  0.0f,   1.0f, 40.0f,
     10.0f, -10.0f, -1000.0f, 1.0f,  0.0f,  0.0f,   0.0f, 40.0f,
     10.0f, -10.0f, -1000.0f, 1.0f,  0.0f,  0.0f,   0.0f, 40.0f,
     10.0f, -10.0f,  10.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
     10.0f,  10.0f,  10.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

    -10.0f, -10.0f, -1000.0f, 0.0f, -1.0f,  0.0f, 0.0f, 40.0f,
     10.0f, -10.0f, -1000.0f, 0.0f, -1.0f,  0.0f, 1.0f, 40.0f,
     10.0f, -10.0f,  10.0f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     10.0f, -10.0f,  10.0f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -10.0f, -10.0f,  10.0f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -10.0f, -10.0f, -1000.0f, 0.0f, -1.0f,  0.0f, 0.0f, 40.0f,

    -10.0f,  10.0f, -1000.0f,    0.0f, 1.0f,  0.0f, 0.0f, 40.0f,
     10.0f,  10.0f, -1000.0f,    0.0f, 1.0f,  0.0f, 1.0f, 40.0f,
     10.0f,  10.0f,  10.0f,      0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     10.0f,  10.0f,  10.0f,     0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    -10.0f,  10.0f,  10.0f,     0.0f, 1.0f,  0.0f, 0.0f, 0.0f,
    -10.0f,  10.0f, -1000.0f,   0.0f, 1.0f,  0.0f, 0.0f, 40.0f
};

static float cube[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

static float hearth[] = {

    -0.5f, 0.5f,
    0.5f, 0.5f,
    0.5f, -0.5f,
    -0.5f, -0.5f,
    -0.5f, 0.5f,
    0.5f, -0.5f,

};
