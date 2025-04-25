#include <stdio.h>
#include "../loader.h"
#include "../linked_list.h"

int main(int argc, char *argv[])
{
	size_t i;
	struct Binary bin;
	struct Symbol *sym;
	struct Section *sec;

	if (argc<2)
	{
		printf("Usage: %s <binary>\n", argv[0]);
		return -1;
	}

	binary_init(&bin);

	if (load_binary(argv[1], &bin, BIN_TYPE_AUTO) < 0)
		return -2;

	printf("loaded binary '%s' %s/%s (%u bits) entry@0x%016jx\n", bin.filename, bin.type_str, bin.arch_str, bin.bits, bin.entry);

	for (i=0; i<list_len(&bin.sections); i++)
	{
		sec = &list_get(&bin.sections, i)->el.sec_el;
		printf("  0x%016jx %-8ju %-20s %s\n", sec->vma, sec->size, sec->name, sec->type==SEC_TYPE_CODE ? "CODE" : "DATA");
	}

	if (list_len(&bin.symbols) > 0)
	{
		printf("scanned symbol tables\n");
		for (i=0; i<list_len(&bin.symbols); i++)
		{
			sym=&list_get(&bin.symbols, i)->el.sym_el;
			printf("  %-40s 0x%016jx %s\n", sym->name, sym->addr, (sym->type & SYM_TYPE_FUNC) ? "FUNC" : "");
		}
	}

	unload_binary(&bin);

	return 0;
}
