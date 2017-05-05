#ifndef ILLIFE_H
#define ILLIFE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct illife_dim_property {
  size_t size, align, ptr_align;
};

void* alloc_illife(size_t n_dim, struct illife_dim_property *dims, size_t type_size);
int init_illife(void* ptr, size_t n_dim, struct illife_dim_property *dims, size_t type_size);
int init_illife_omp(void* ptr, size_t n_dim, struct illife_dim_property *dims, size_t type_size);
int free_illife(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // ILLIFE_H
