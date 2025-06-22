
#include "fishhook.h"
#include <stdio.h>

int rebind_symbols(struct rebinding rebindings[], size_t rebindings_nel) {
  // 仅模拟函数定义，用于绕过链接错误
  printf("rebind_symbols called (mock)\n");
  return 0;
}
