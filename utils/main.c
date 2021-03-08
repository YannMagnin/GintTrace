#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

struct callnode {
	struct callnode *parent;
	struct callnode *sibling;
	struct callnode *child;
};

static uint32_t get_lword(uint8_t *file)
{
	return (file[0] << 24 | file[1] << 16 | file[2] << 8 | file[3]);
}

static void generate_binary_tree(struct callnode **node, uint8_t *file)
{
	*node = calloc(sizeof(struct callnode), 1);
	if (*node == NULL) {
		fprintf(stderr, "ENOMEM\n");
		exit(84);
	}
	size_t size = get_lword(file);
	printf("size: %lx\n", size);
	generate_binary_tree(node, &file[size]);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return (84);
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "unable to open '%s'\n", argv[1]);
		return (84);
	}
	uint8_t *map = calloc(512 * 1024, 1);
	if (map == NULL) {
		fprintf(stderr, "ENOMEM\n");
		return (84);
	}
	size_t size = read(fd, map, 512 * 1024);
	printf("file size: %ld\n", size);
	printf("lword = %x\n", get_lword(map));

	for (size_t i = 0; i < size; i++) {
		if (i != 0 && (i & 7) == 0)
			printf("\n");
		printf("%.2x", map[i]);
	}
	return (0);
}
