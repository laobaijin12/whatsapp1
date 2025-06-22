#ifndef FISHHOOK_H
#define FISHHOOK_H
#include <stddef.h> 
struct rebinding {
  const char *name;
  void *replacement;
  void **replaced;
};
int rebind_symbols(struct rebinding rebindings[], size_t rebindings_nel);
#endif
