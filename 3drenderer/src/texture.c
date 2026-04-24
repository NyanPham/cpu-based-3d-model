#include "texture.h"

tex2_t tex2_clone(tex2_t* t) {
    tex2_t res = {
        .u = t->u,
        .v = t->v
    };

    return res;
}
