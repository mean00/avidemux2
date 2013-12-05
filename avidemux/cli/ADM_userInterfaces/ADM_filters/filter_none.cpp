#if defined(__sun__)
/*
  this is to avoid an empty library, with no symbols, raising
 "ld: elf error: file libADM_filtersCli6.a: elf_getarsym"
*/
int Sun_ar_require_a_symbol_here_4_cli = 0;
#endif /* : defined(__sun__) */
