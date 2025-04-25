#ifndef LOADR_HEADER
#define LOADR_HEADER

#include <stdint.h>
#include "inc/data_structure.h"

void symbol_init(struct Symbol *sym);
void symbol_destroy(struct Symbol *sym);
void section_init(struct Section *sec);
void section_destroy(struct Section *sec);
void binary_init(struct Binary *bin);

/*bool contains(struct Section *sec, uint64_t addr);
struct Section *get_text_section(struct Binary *bin);*/

int load_binary(const char *fname, struct Binary *bin, enum BinaryType type);
void unload_binary(struct Binary *bin);

#endif /* LOADR_HEADER */
