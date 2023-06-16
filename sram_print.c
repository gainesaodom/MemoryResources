#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>

void sramprint_add_to_file64(FILE *f, uint64_t addr, uint32_t data) {

	fprintf(f, "Address: %016" PRIx64 " | ", addr);
	fprintf(f, "Word: %08" PRIx32 "\n", data);
}

void sramprint_add_to_file32(FILE *f, uint32_t addr, uint32_t data) {

	fprintf(f, "Address: %08" PRIx32 " | ", addr);
	fprintf(f, "Word: %08" PRIx32 "\n", data);
}

void sramprint_add_to_file16(FILE *f, uint16_t addr, uint32_t data) {

	fprintf(f, "Address: %04" PRIx16 " | ", addr);
	fprintf(f, "Word: %08" PRIx32 "\n", data);
}
