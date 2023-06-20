#include <stdio.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <assert.h>

// must keep `test_trx` outside the function. For whatever reason, if this is
// declared inside of spi_test, this SPI transfer will fail when this
// function is callespi_ioc_transfer test_trx; 
// void spi_test(int fd_) {

uint8_t test_tx_buffer[32];
uint8_t test_rx_buffer[32];


test_trx.tx_buf = (unsigned long) test_tx_buffer;
test_trx.rx_buf = (unsigned long) test_rx_buffer;
test_trx.bits_per_word = SPI_NUMBER_OF_BITS;
test_trx.speed_hz = SPI_SPEED_HZ;
test_trx.delay_usecs = SPI_DELAY_US;
test_trx.len = 32;

uint8_t looper;
for (looper = 0; looper < 32; ++looper) {
    test_tx_buffer[looper] = looper;
    test_rx_buffer[looper] = 0xFF;
}

uint32_t ret = ioctl(fd_, SPI_IOC_MESSAGE(1), &test_trx);
if (ret != 0) {
    printf("SPI transfer returned %d... \r\n", ret);
}

printf("Received SPI buffer...\r\n");
for (looper = 0; looper < 32; ++looper) {
    printf("%02x ", test_rx_buffer[looper]);
}
printf("\n");
}d.
