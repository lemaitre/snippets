#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stddef.h>
#include "illife.h"

void* allocate(size_t size, size_t align) {
  void* p = NULL;
  if (posix_memalign(&p, align, size)) {
    return NULL;
  }
  return p;
}

void deallocate(void* ptr) {
  free(ptr);
}

#define ROUND_UP(a, align) (((a) + ((align)-1)) & -(align))

static size_t ptr_size(size_t n_dim, struct illife_dim_property *dims) {
  if (n_dim <= 1) return 0;
  size_t s = ptr_size(n_dim - 1, dims + 1);
  size_t ps = ROUND_UP(dims->size * sizeof(void*), dims->ptr_align);
  return dims->size * s + ps;
}

static size_t data_size(size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  if (n_dim == 0) return type_size;
  size_t s = data_size(n_dim - 1, dims + 1, type_size);
  return ROUND_UP(s * dims->size, dims->align);
}

static size_t whole_size(size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  size_t ps = ptr_size(n_dim, dims);
  size_t ds = data_size(n_dim, dims, type_size);
  size_t ws = ROUND_UP(ps, dims->align) + ds;
  return ws;
}

static int _init_illife(char** ptrs, char* data, size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  size_t n = dims->size;
  if (n_dim <= 1) return 1;
  size_t ds = data_size(n_dim-1, dims+1, type_size);
  size_t ps = ptr_size(n_dim-1, dims+1);
  char* ptr2 = (char*) ptrs;
  ptr2 += ptr_size(2, dims);
  if (n_dim == 2) {
    for (size_t i = 0; i < n; i++) {
      ptrs[i] = data + i * ds;
    }
    return 1;
  }
  for (size_t i = 0; i < n; i++) {
    ptrs[i] = ptr2 + i * ps;
    _init_illife((char**) ptrs[i], data + i * ds, n_dim - 1, dims + 1, type_size);
  }
  return 1;
}

void* alloc_illife(size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  size_t align = ROUND_UP(dims->ptr_align, dims->align);
  size_t s = whole_size(n_dim, dims, type_size);
  return allocate(s, align);
}

int init_illife(void* ptr, size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  init_illife_omp(ptr, n_dim, dims, type_size);
  return 1;
}

int init_illife_omp(void* ptr, size_t n_dim, struct illife_dim_property *dims, size_t type_size) {
  if (n_dim <= 1) return 1;
  char **ptrs = (char**) ptr;
  char *data = (char*) ptr;
  {
    size_t ps = ptr_size(n_dim, dims);
    size_t ds = data_size(n_dim, dims, type_size);
    size_t ws = whole_size(n_dim, dims, type_size);
    data += ws - ds;
  }
  size_t n = dims->size;
  size_t ds = data_size(n_dim-1, dims+1, type_size);
  size_t ps = ptr_size(n_dim-1, dims+1);
  char* ptr2 = (char*) ptrs;
  ptr2 += ptr_size(2, dims);
  if (n_dim == 2) {
#pragma omp parallel for
    for (size_t i = 0; i < n; i++) {
      ptrs[i] = data + i * ds;
    }
    return 1;
  }
#pragma omp parallel for
  for (size_t i = 0; i < n; i++) {
    ptrs[i] = ptr2 + i * ps;
    _init_illife((char**) ptrs[i], data + i * ds, n_dim - 1, dims + 1, type_size);
  }
  return 1;
}

int free_illife(void* ptr) {
  deallocate(ptr);
  return 1;
}
