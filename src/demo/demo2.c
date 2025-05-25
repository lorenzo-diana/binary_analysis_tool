/*
* Demo2 takes the name of an executable file and a name of a section as inputs
* and print the content of the specified section.
*/

#include <stdio.h>
#include <string.h>
#include "../loader.h"
#include "../linked_list.h"

int main(int argc, char *argv[])
{
	size_t i, j;
	struct Binary bin;
	struct Section *sec=nullptr;
	bool section_found=false;
	const uint8_t *sec_content=nullptr;

	if (argc<3)
	{
		printf("Usage: %s <binary> <section_name>\n", argv[0]);
		return -1;
	}

	binary_init(&bin);

	if (load_binary(argv[1], &bin, BIN_TYPE_AUTO) < 0)
		return -2;

	printf("loaded binary '%s' %s/%s (%u bits) entry@0x%016jx\n", bin.filename, bin.type_str, bin.arch_str, bin.bits, bin.entry);

	for (i=0; i<list_len(&bin.sections); i++)
	{
		sec = &list_get(&bin.sections, i)->el.sec_el;
		if (strcmp(sec->name, argv[2]) == 0)
		{
			section_found=true;
			printf("Section name: %s\nSection type: %s\nSection size: %zu\nSection data:\n\t", sec->name, sec->type==SEC_TYPE_CODE ? "CODE" : "DATA", sec->size);
			// Print the content of selected section
			sec_content = sec->bytes;
			for (j=1; j<=sec->size; j++)
			{
				printf("%02x", *(sec_content++));
				if (j%4 == 0)
					printf(" ");
				else
					if (j%16 == 0)
						printf("\n");
			}
			printf("\n");
			break; // exit the for loop
		}
	}

	if (!section_found)
		printf("Section \"%s\" not found!\n", argv[2]);

	unload_binary(&bin);

	return 0;
}
