/*
SPI API for the 23K640 SRAM chip

HEY - IF YOU GET "Could not write SPI mode, ret = -1" THEN MAKE SURE YOU RUN
IT AS sudo <executable-path>
For example:
	sudo ./a.out

Authors:
	Gaines Odom
	Amaar Ebrahim
*/

#include <stdint.h>
#include <stdbool.h>
#include "spi23x640.c"


uint8_t checkerboard_compute_byte_value_at_address(uint16_t address, bool checkerboard_starts_with_a_zero_bit) {
	bool address_is_even = ((uint32_t) address) % 2 == 0;

	bool first_condition_for_0xAA = address_is_even && checkerboard_starts_with_a_zero_bit == false;
	bool second_condition_for_0xAA = address_is_even == false && checkerboard_starts_with_a_zero_bit;

	if (first_condition_for_0xAA || second_condition_for_0xAA) {
		return 0xAA;
	} else {
		return 0x55;
	}
}

uint8_t read_bit(uint16_t address, uint8_t bitpos) {

	if (bitpos > 7) {
		errno = 22;
		printf("Argument #2 `bitpos` needs to be between 0 and 7!\n");
		printf("Got %d\n", bitpos);
		exit(EXIT_FAILURE);
	}

	uint8_t byte = spi23x640_read_byte(address);
	return (byte >> bitpos) & 0x01;
}

uint8_t set_bit_in_byte(uint8_t byte, uint8_t bitpos, uint8_t value) {
	if (bitpos > 7) {
		printf("Argument #2 `bitpos` needs to be between 0 and 7!\n");
		exit(EXIT_FAILURE);
	}

	if (value > 1) {
		printf("Argument #3 `value` needs to be '0' or '1'!\n");
		exit(EXIT_FAILURE);
	}
	
	if (value) {
		byte = byte | (0x01 << bitpos);
	} else {
		byte = byte & (~(0x01 << bitpos));
	}	

	return byte;
}

void write_bit(uint16_t address, uint8_t bitpos, uint8_t value) {


	uint8_t byte = spi23x640_read_byte(address);
	byte = set_bit_in_byte(byte, bitpos, value);
	spi23x640_write_byte(address, byte);

}

void reset_memory_to_zeroes() {
	uint16_t address_pointer;
	for (address_pointer = 0; address_pointer <= SPI23X640_MAX_ADDRESS; address_pointer++) {
		spi23x640_write_byte(address_pointer, 0x00);
	}

}

void print_value_at_address(uint16_t address) {
	printf("Address: %04" PRIx16 " \tValue: %02" PRIx8 "\n", address, spi23x640_read_byte(address));
}

void print_entire_address_space() {
	uint16_t address_pointer;
	for (address_pointer = 0; address_pointer <= SPI23X640_MAX_ADDRESS; address_pointer++) {
		print_value_at_address(address_pointer);
	}	
}

uint16_t march_A_test() {

	int i = 0;
	uint16_t number_of_faults = 0;
	
	int16_t address_idx;
	

	printf("March A Test...\n");

	printf("Performing March 0...\n");
	// M0 - write 0 (diff version)
	reset_memory_to_zeroes();
	
	printf("Number of faults after March 0: %d\n", number_of_faults);

	printf("Performing March 1...\n");
	// M1 - read 0, write 1, write 0, write 1 (ascending order)
	for (address_idx = 0; address_idx <= SPI23X640_MAX_ADDRESS; address_idx++) {

		//print_value_at_address(address_idx);
		
		uint8_t expected_value = 0;

		for (i = 0; i < 8; i++) {

			uint8_t actual_value = read_bit(address_idx, i);	

			//printf("%04" PRIx16 ", %d - read value %02" PRIx16 "\n", address_idx, i, actual_value);

			if (expected_value != actual_value) {	// read 0
				number_of_faults++;
				printf("M1 Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
				//exit(EXIT_FAILURE);
			}	

			write_bit(address_idx, i, 1);
			write_bit(address_idx, i, 0);
			write_bit(address_idx, i, 1);
		}
	}
	
	printf("Number of faults after March 1: %d\n", number_of_faults);

	printf("Performing March 2...\n");
	// M2 - read 1, write 0, write 1 (ascending order)
	for (address_idx = 0; address_idx <= SPI23X640_MAX_ADDRESS; address_idx++) {
		
		uint8_t expected_value = 1;

		for (i = 0; i < 8; i++) {

			uint8_t actual_value = read_bit(address_idx, i);	

			//printf("Read value %04" PRIx16 "\n", actual_value);

			if (expected_value != actual_value) {	// read 1
				number_of_faults++;
				printf("M2 Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
				//exit(EXIT_FAILURE);
			}	

			write_bit(address_idx, i, 0);
			write_bit(address_idx, i, 1);

		}
	}
	
	printf("Number of faults after March 2: %d\n", number_of_faults);

	printf("Performing March 3...\n");
	// M3 - read 1, write 0, write 1, write 0 (descending order)
	for (address_idx = SPI23X640_MAX_ADDRESS; address_idx >= 0; address_idx--) {


		uint8_t expected_value = 1;
		for (i = 7; i >= 0; i--) {

			uint8_t actual_value = read_bit(address_idx, i);
			if (actual_value != expected_value) {
				number_of_faults++;
		 		printf("M3 Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
				//exit(EXIT_FAILURE);
			}

			write_bit(address_idx, i, 0);
			write_bit(address_idx, i, 1);
			write_bit(address_idx, i, 0);

		}

	}

	printf("Number of faults after March 3: %d\n", number_of_faults);

	printf("Performing March 4...\n");
	// M4 - read 0, write 1, write 0 (descending order)
	for (address_idx = SPI23X640_MAX_ADDRESS; address_idx >= 0; address_idx--) {

		uint8_t expected_value = 0;

		for (i = 7; i >= 0; i--) {

			uint8_t actual_value = read_bit(address_idx, i);//((spi23x640_read_byte(address_idx) >> i) & 1);

			//printf("Read value %04" PRIx16 "\n", actual_value);

			if (expected_value != actual_value) {	// read 0
				number_of_faults++;
				printf("Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
				//exit(EXIT_FAILURE);
			}	

			write_bit(address_idx, i, 1);
			write_bit(address_idx, i, 0);

		}
	}

	printf("Number of faults after March 4 (total): %d\n", number_of_faults);
	printf("Finished March A Test...\n");


	return number_of_faults;

}

int get_num_of_different_bits_between_two_bytes(uint8_t b1, uint8_t b2) {

	// the bits that are 1 are the differences
	uint8_t xor_of_bytes = b1 ^ b2;

	int num_of_different_bytes = 0;
	int i;

	// do this 8 times because there are 8 bits in a byte
	for (i = 0; i < 8; i++) {

		// This if-statement is only true when the LSB is 1. There is a difference
		// at the LSB if it is 1
		if (0x01 & xor_of_bytes) {
			num_of_different_bytes++;
		}

		// shift down the bits, so the new LSB is the bit to the left of the old LSB
		xor_of_bytes >>= 1;

	}

	return num_of_different_bytes;

}

/*
	Runs a checkerboard test on the 23x640 SRAM chip using SPI

	first_bit_is_zero - if true, the checkerboard will be made such that the
	upperleft bit in the memory space is a logic-0. If false, the upperleft
	bit will be a logic-1.

	Returns the number of bits with an incorrect value identified after scanning through the memory
	space.
*/
uint16_t checkerboard_test(bool first_bit_is_zero) {

	uint16_t number_of_faults = 0;

	uint16_t address_idx;
	for (address_idx = 0; address_idx <= SPI23X640_MAX_ADDRESS; address_idx++) {
		//printf("%x\n", address_idx);
		uint8_t next_value = checkerboard_compute_byte_value_at_address(address_idx, first_bit_is_zero);
		spi23x640_write_byte(address_idx, next_value);

		//printf("Writing value %04" PRIx16 "\n", next_value);

	}	

	for (address_idx = 0; address_idx <= SPI23X640_MAX_ADDRESS; address_idx++) {
		
		uint8_t expected_value = checkerboard_compute_byte_value_at_address(address_idx, first_bit_is_zero);
		uint8_t actual_value = spi23x640_read_byte(address_idx);	

		//printf("Read value %04" PRIx16 "\n", actual_value);

		if (expected_value != actual_value) {
			int num_of_faults_in_byte = get_num_of_different_bits_between_two_bytes(expected_value, actual_value);
			number_of_faults += num_of_faults_in_byte;
			//printf("Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
		}	
	}

	//printf("Number of faults: %d\n", number_of_faults);
	return number_of_faults;
}



struct checkboard_result_summary {
	uint16_t step1_faults;
	uint16_t step2_faults;
	uint32_t total_faults;
};

struct checkboard_result_summary full_checkerboard_test() {

	printf("Checkerboard Test...\n");

	struct checkboard_result_summary result;

	printf("Performing Step 1...\n");
	int step1_faults = checkerboard_test(false);
	printf("Number of faults in Step 1: %d\n", step1_faults);

	printf("Performing Step 2...\n");
	int step2_faults = checkerboard_test(true);
	printf("Number of faults in Step 2: %d\n", step2_faults);

	printf("Finished Checkerboard Test...\n");

	result.step1_faults = step1_faults;
	result.step2_faults = step2_faults;
	result.total_faults = step1_faults + step2_faults;

	return result;
	
}




int main() {

	spi23x640_init(5000000);

	uint16_t num_faults_detected_by_march_A = march_A_test();
	struct checkboard_result_summary res = full_checkerboard_test();

	printf("------------------------------\n");
	printf("Faults detected by March A test:\t %d\n", num_faults_detected_by_march_A);
	printf("Faults detected by Checkerboard test:\t %d\n", res.total_faults);
	printf("------------------------------\n");

	spi23x640_close();

	// printf("done\n");



}
