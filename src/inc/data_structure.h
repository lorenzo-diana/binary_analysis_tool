#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES

#include <stdint.h>

enum SectionType : unsigned int
{
	SEC_TYPE_NONE = 0,
	SEC_TYPE_CODE,
	SEC_TYPE_DATA
};
struct Section
{
	struct Binary *binary;
	char * name;
	enum SectionType type;
	uint64_t vma;
	uint64_t size;
	uint8_t *bytes;
};

enum SymbolType : unsigned int
{
	SYM_TYPE_UKN = 0,
	SYM_TYPE_FUNC
};
struct Symbol
{
	enum SymbolType type;
	char * name;
	uint64_t addr;
};

union Element
{
    struct Section sec_el;
    struct Symbol sym_el;
};
enum ListType : unsigned int
{
    SECTION_LIST = 0,
    SYMBOL_LIST
};
struct Node
{
    union Element el;
    struct Node *next;
};
struct List
{
	struct Node *head, *tail;
	enum ListType list_type;
	uint32_t len; // if the type of len is updated, update list_append() accordignly
};

enum BinaryType : unsigned int
{
	BIN_TYPE_AUTO = 0,
	BIN_TYPE_ELF,
	BIN_TYPE_PE
};
enum BinaryArch : unsigned int
{
	ARCH_NONE = 0,
	ARCH_X86
};
struct Binary
{
	const char *filename;
	enum BinaryType type;
	const char * type_str;
	enum BinaryArch arch;
	const char *arch_str;
	unsigned int bits;
	uint64_t entry;
	struct List sections;
	struct List symbols;
};

#endif /* DATA_STRUCTURES */
