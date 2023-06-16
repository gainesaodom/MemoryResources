/*
SPI API for the 23K640 SRAM chip

HEY - IF YOU GET "Could not write SPI mode, ret = -1" THEN MAKE SURE YOU RUN
IT AS sudo <executable-path>
For example:
	sudo ./a.out
*/

#include <stdint.h>
#include <stdbool.h>
#include "spi23x640.c"


// must keep `test_trx` outside the function. For whatever reason, if this is
// declared inside of spi_test, this SPI transfer will fail when this
// function is callespi_ioc_transfer test_trx; 
// void spi_test(int fd_) {

// 	uint8_t test_tx_buffer[32];
// 	uint8_t test_rx_buffer[32];


// 	test_trx.tx_buf = (unsigned long) test_tx_buffer;
// 	test_trx.rx_buf = (unsigned long) test_rx_buffer;
// 	test_trx.bits_per_word = SPI_NUMBER_OF_BITS;
// 	test_trx.speed_hz = SPI_SPEED_HZ;
// 	test_trx.delay_usecs = SPI_DELAY_US;
// 	test_trx.len = 32;

// 	uint8_t looper;
// 	for (looper = 0; looper < 32; ++looper) {
// 		test_tx_buffer[looper] = looper;
// 		test_rx_buffer[looper] = 0xFF;
// 	}
	
// 	uint32_t ret = ioctl(fd_, SPI_IOC_MESSAGE(1), &test_trx);
// 	if (ret != 0) {
// 		printf("SPI transfer returned %d... \r\n", ret);
// 	}

// 	printf("Received SPI buffer...\r\n");
// 	for (looper = 0; looper < 32; ++looper) {
// 		printf("%02x ", test_rx_buffer[looper]);
// 	}
// 	printf("\n");
// }d.
// 

uint8_t checkerboard_compute_byte_value_at_address(uint8_t *address, bool checkerboard_starts_with_a_zero_bit) {
	bool address_is_even = ((uint32_t) address) % 2 == 0;

	bool first_condition_for_0xAA = address_is_even && checkerboard_starts_with_a_zero_bit == false;
	bool second_condition_for_0xAA = address_is_even == false && checkerboard_starts_with_a_zero_bit;

	if (first_condition_for_0xAA || second_condition_for_0xAA) {
		return 0xAA;
	} else {
		return 0x55;
	}
}

void checkerboard_test(bool first_bit_is_zero) {

	uint16_t number_of_faults = 0;

	// address_idx is a pointer to a uint8_t, so "++" increases its value by just 1
	uint8_t *address_idx;
	uint32_t max_address = 65536;
	for (address_idx = (uint8_t *) 0; address_idx < max_address; address_idx++) {
		//printf("%x\n", address_idx);
		uint8_t next_value = checkerboard_compute_byte_value_at_address(address_idx, first_bit_is_zero);
		spi23x640_write_byte(address_idx, next_value);

		printf("Writing value %04" PRIx16 "\n", next_value);

	}	

	for (address_idx = (uint8_t *) 0; address_idx < max_address; address_idx++) {
		
		uint8_t expected_value = checkerboard_compute_byte_value_at_address(address_idx, first_bit_is_zero);
		uint8_t actual_value = spi23x640_read_byte(address_idx);	

		printf("Read value %04" PRIx16 "\n", actual_value);

		if (expected_value != actual_value) {
			number_of_faults++;
			printf("Fault -- %02" PRIx8 " at %04" PRIx16 " when it should be %02" PRIx16 "\n", actual_value, address_idx, expected_value);
		}	
	}

	printf("Number of faults: %d\n", number_of_faults);
}

void reset_memory_to_zeroes() {
	uint8_t *address_pointer;
	for (address_pointer = 0; address_pointer < (uint8_t *)SPI23X640_NUMBER_OF_BYTES; address_pointer++) {
		spi23x640_write_byte(address_pointer, 0x00);
	}

	// only prints first 1024 bytes
	for (address_pointer = 0; address_pointer < (uint8_t *)1024; address_pointer++) {
		uint8_t actual_value = spi23x640_read_byte(address_pointer);
		printf("value: %x\n", actual_value);
	}
}

int main() {

	spi23x640_init(5000000);

	//reset_memory_to_zeroes();

	checkerboard_test(false);

	spi23x640_close();

	printf("done\n");



}
