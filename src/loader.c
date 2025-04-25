#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PACKAGE "binary_analysis_tool"
#define PACKAGE_VERSION "0.1"
#include <bfd.h>

#include "loader.h"
#include "linked_list.h"

static int load_symbols_bfd(bfd *bfd_h, struct Binary *bin)
{
	int ret;
	long n, nsyms, i;
	struct Symbol *sym;
	struct Node *sym_el;
	asymbol **bfd_symtab=NULL;

	n=bfd_get_symtab_upper_bound(bfd_h);
	if (n<0)
	{
		fprintf(stderr, "failed to read symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		goto fail;
	}
	else
		if (n)
		{
			bfd_symtab=(asymbol**)malloc(n);
			if (!bfd_symtab)
			{
				fprintf(stderr, "out of memory\n");
				goto fail;
			}
			nsyms=bfd_canonicalize_symtab(bfd_h, bfd_symtab);
			if (nsyms<0)
			{
				fprintf(stderr, "failed to read symtab (%s)\n", bfd_errmsg(bfd_get_error()));
				goto fail;
			}
			for (i=0; i<nsyms; i++)
			{
				if (bfd_symtab[i]->flags & BSF_FUNCTION)
				{
					sym_el=list_append(&bin->symbols);
					if (!sym_el)
					{
						fprintf(stderr, "out of memory\n");
						goto fail;
					}
					sym=&sym_el->el.sym_el;
					
					sym->type=SYM_TYPE_FUNC;
					sym->name=strdup(bfd_symtab[i]->name);
					if (!sym->name)
					{
						fprintf(stderr, "out of memory\n");
						goto fail;
					}
					sym->addr=bfd_asymbol_value(bfd_symtab[i]);
				}
			}
		}
	ret=0;
	goto cleanup;

fail:
	ret=-1;

cleanup:
	if (bfd_symtab)
		free(bfd_symtab);

	return ret;
}

static int load_dynsym_bfd(bfd *bfd_h, struct Binary *bin)
{
	int ret;
	long n, nsyms, i;
	struct Symbol *sym;
	struct Node *sym_el;
	asymbol **bfd_dynsym = NULL;

	n = bfd_get_dynamic_symtab_upper_bound(bfd_h);
	if (n<0)
	{
		fprintf(stderr, "failed to read dynamic symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		goto fail;
	}
	else
		if (n)
		{
			bfd_dynsym = (asymbol**)malloc(n);
			if (!bfd_dynsym)
			{
				fprintf(stderr, "Out of memory\n");
				goto fail;
			}
			nsyms = bfd_canonicalize_dynamic_symtab(bfd_h, bfd_dynsym);
			if (nsyms<0)
			{
				fprintf(stderr, "failed to read dynamic symtab (%s)\n", bfd_errmsg(bfd_get_error()));
				goto fail;
			}
			for (i=0; i<nsyms; i++)
			{
				if (bfd_dynsym[i]->flags & BSF_FUNCTION)
				{
					sym_el=list_append(&bin->symbols);
					if (!sym_el)
					{
						fprintf(stderr, "out of memory\n");
						goto fail;
					}
					sym=&sym_el->el.sym_el;

					sym->type=SYM_TYPE_FUNC;
					sym->name=strdup(bfd_dynsym[i]->name);
					if (!sym->name)
					{
						fprintf(stderr, "out of memory\n");
						goto fail;
					}
					sym->addr=bfd_asymbol_value(bfd_dynsym[i]);
				}
			}
		}

	ret=0;
	goto cleanup;

fail:
	ret=-1;

cleanup:
	if (bfd_dynsym)
		free(bfd_dynsym);

	return ret;

}

static int load_sections_bfd(bfd *bfd_h, struct Binary *bin)
{
	int bfd_flags;
	uint64_t vma, size;
	const char *secname;
	asection* bfd_sec;
	struct Section *sec;
	struct Node *sec_el;
	enum SectionType sectype;
	for (bfd_sec=bfd_h->sections; bfd_sec; bfd_sec=bfd_sec->next)
	{
		bfd_flags = bfd_section_flags(bfd_sec);
		sectype = SEC_TYPE_NONE;
		if (bfd_flags & SEC_CODE)
			sectype = SEC_TYPE_CODE;
		else
			if (bfd_flags & SEC_DATA)
				sectype = SEC_TYPE_DATA;
			else
				continue;
		vma = bfd_section_vma(bfd_sec);
		size = bfd_section_size(bfd_sec);
		secname = bfd_section_name(bfd_sec);

		if (!secname)
			secname = "<unnamed>";

		sec_el=list_append(&bin->sections);
		if (!sec_el)
		{
			fprintf(stderr, "out of memory\n");
			return -1;
		}
		sec=&sec_el->el.sec_el;

		sec->binary = bin;
		sec->name = strdup(secname);
		if (!sec->name)
		{
			fprintf(stderr, "out of memory\n");
			return -1;
		}
		sec->type = sectype;
		sec->vma = vma;
		sec->size = size;
		sec->bytes = (uint8_t*)malloc(size);
		if (!sec->bytes)
		{
			fprintf(stderr, "out of memory\n");
			return -1;
		}

		if (!bfd_get_section_contents(bfd_h, bfd_sec, sec->bytes, 0, size))
		{
			fprintf(stderr, "failed to read section '%s' (%s)\n", secname, bfd_errmsg(bfd_get_error()));
			return -1;
		}
	}

	return 0;
}

static bfd* open_bfd(const char *fname)
{
	static int bfd_inited=0;
	bfd *bfd_h;

	if (!bfd_inited)
	{
		bfd_init();
		bfd_inited=1;
	}

	bfd_h=bfd_openr(fname, NULL);
	if (!bfd_h)
	{
		fprintf(stderr, "failed to open binary '%s' (%s)\n", fname, bfd_errmsg(bfd_get_error()));
		return NULL;
	}

	if (!bfd_check_format(bfd_h, bfd_object))
	{
		fprintf(stderr, "file '%s' does not look like an executable (%s)\n", fname, bfd_errmsg(bfd_get_error()));
		return NULL;
	}

	/* Some version of bfd_check_format pessimistically set a wrong_format
	 * error before detecting the format and then neglect to unset it once
	 * the format has been detected. We unset it manually to prevent problems.
	*/
	bfd_set_error(bfd_error_no_error);

	if (bfd_get_flavour(bfd_h) == bfd_target_unknown_flavour)
	{
		fprintf(stderr, "unrecognized format for binary '%s' (%s)\n", fname, bfd_errmsg(bfd_get_error()));
	}

	return bfd_h;
}

static int load_binary_bfd(const char *fname, struct Binary *bin, enum BinaryType type)
{
	int ret;
	bfd *bfd_h;
	const bfd_arch_info_type *bfd_info;

	bfd_h=nullptr;
	bfd_h=open_bfd(fname);
	if (!bfd_h)
	{
		goto fail;
	}

	bin->filename=fname;
	bin->entry=bfd_get_start_address(bfd_h);

	bin->type_str=bfd_h->xvec->name;
	switch (bfd_h->xvec->flavour)
	{
		case bfd_target_elf_flavour:
			bin->type=BIN_TYPE_ELF;
			break;
		case bfd_target_coff_flavour:
			bin->type=BIN_TYPE_PE;
			break;
		case bfd_target_unknown_flavour:
		default:
			fprintf(stderr, "unsupported binary type (%s)\n", bfd_h->xvec->name);
			goto fail;
	}

	bfd_info=bfd_get_arch_info(bfd_h);
	bin->arch_str=bfd_info->printable_name;
	switch (bfd_info->mach)
	{
		case bfd_mach_i386_i386:
			bin->arch=ARCH_X86;
			bin->bits=32;
			break;
		case bfd_mach_x86_64:
			bin->arch=ARCH_X86;
			bin->bits=64;
			break;
		default:
			fprintf(stderr, "unsupported architecture (%s)\n", bfd_info->printable_name);
			goto fail;
	}

	/* Symbo, handling is best-effort only (they may not even be present) */
	load_symbols_bfd(bfd_h, bin);
	load_dynsym_bfd(bfd_h, bin);

	if (load_sections_bfd(bfd_h, bin)<0)
		goto fail;

	ret=0;
	goto cleanup;

fail:
	ret=-1;

cleanup:
	if (bfd_h)
		bfd_close(bfd_h);

	return ret;
}

int load_binary(const char *fname, struct Binary *bin, enum BinaryType type)
{
	return load_binary_bfd(fname, bin, type);
}

void unload_binary(struct Binary *bin)
{
	list_destroy(&bin->sections);
	list_destroy(&bin->symbols);
}

void symbol_init(struct Symbol *sym)
{
	sym->type=SYM_TYPE_UKN;
	sym->name=nullptr;
	sym->addr=0;
}

void symbol_destroy(struct Symbol *sym)
{
	if (sym->name)
	    free(sym->name);
}

void section_init(struct Section *sec)
{
	sec->binary=nullptr;
	sec->name=nullptr;
	sec->type=SEC_TYPE_NONE;
	sec->vma=0;
	sec->size=0;
	sec->bytes=nullptr;
}

void section_destroy(struct Section *sec)
{
	if (sec->name)
	    free(sec->name);
	if (sec->bytes)
	    free(sec->bytes);
}

void binary_init(struct Binary *bin)
{
	bin->filename=nullptr;
	bin->type=BIN_TYPE_AUTO;
	bin->type_str=nullptr;
	bin->arch=ARCH_NONE;
	bin->arch_str=nullptr;
	bin->bits=0;
	bin->entry=0;
	list_init(&bin->sections, SECTION_LIST);
	list_init(&bin->symbols, SYMBOL_LIST);
}
