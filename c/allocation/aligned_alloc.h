#ifdef __cplusplus
extern "C" {
#endif

void* aligned_alloc(size_t size, size_t align);
void aligned_free(void* p);

#ifdef __cplusplus
}
#endif
