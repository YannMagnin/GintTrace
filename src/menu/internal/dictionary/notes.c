#include "./src/menu/internal/dictionary.h"

#include <gint/std/string.h>
#include <gint/std/stdio.h>

/* internal note table, set by the consturctor at runtime */
static struct osnote *osnote_table = NULL;

/* internal struct used to store OS-specific note */
extern struct osnote osnote_table_03502202[];

struct {
	const char *os_version;
	struct osnote *table;
} osnote_db[] = {
	{.os_version = "03.50.2202", .table = osnote_table_03502202},
	{.os_version = NULL, .table = NULL},
};

/* dictionary_notes_init(): Constructor, used to get the OS-specific note */
__attribute__((constructor))
static void dictionary_notes_init(void)
{
	char os_version[11];

	/* get the internal OS string */
#if FXCG50
	void (*GetOSVersion)(char *buffer) = (*(void ***)0x8002007c)[0x1406];
#endif
#if FX9860G
	void (*GetOSVersion)(char *buffer) = (*(void ***)0x8001007c)[0x2ee];
#endif

	/* get OS version */
	GetOSVersion(os_version);
	os_version[10] = '\0';

	/* try to get the appropriate table */
	osnote_table = NULL;
	for (int i = 0; osnote_db[i].os_version != NULL; ++i) {
		if (strcmp(os_version, osnote_db[i].os_version) == 0) {
			osnote_table = osnote_db[i].table;
			return;
		}
	}
}

/* dictionary_notes_check(): Check OS specific notes */
const char *dictionary_notes_check(void *address)
{
	if (address == NULL || osnote_table == NULL)
		return (NULL);
	for (int i = 0; osnote_table[i].name != NULL; ++i) {
		if ((uintptr_t)address == osnote_table[i].address)
			return (osnote_table[i].name);
	}
	return (NULL);
}

//---
// Notes
//---
struct osnote osnote_table_03502202[] = {
	{.address = 0x801815a6, .name = "uint32_t atomic_start(void)"},

	{.address = 0x802de518, .name = "int fugue_strlen_secure(const char *str)"},
	{.address = 0x8037a31c, .name = "int fugue_strlen(const char *str)"},
	{.address = 0x8017233e, .name = "int fugue_dump_volume_info(const char *volume_name, void *buffer)"},
	{.address = 0x8014f808, .name = "int fugue_open(struct bpb *bios, uint8_t *pathname, int mode)"},
	{.address = 0x8014f2ac, .name = "int fugue_dump_info(strcuct casio_fs *fs, void *buffer)"},
	{.address = 0x8015443e, .name = "int fugue_check_file_validity(strcuct bpb *bios, void *file, void *previous)"},
	{.address = 0x00000000, .name = NULL},
};
