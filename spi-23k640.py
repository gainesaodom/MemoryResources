# module for interacting with the 23k640 sram chip 
# supports reading and writing in "Byte Operation"
# supports reading status register
# does not support "Page Operation" or "Sequential Operation"
# does not support writing to status register
# Author: Amaar Ebrahim
# Email: aae0008@auburn.edu

import spidev 

# 23k640 instruction bytes - don't touch
# these variables are named according to the "instruction name" they correspond 
# to in the 23k640 datasheet 
READ = 0x03
WRITE = 0x02
RDSR = 0x05


# spi config
bus = 0
device = 0

spi = spidev.SpiDev()
spi.open(bus, device)


spi.mode = 0b00
spi.max_speed_hz = 5000
spi.bits_per_word = 8

# functions

def _transfer(message):
	return spi.xfer3(message)


# Reads the status of the chip
def get_status():
	response = _transfer([RDSR, 0x00])
	return response[-1]


# Reads the byte at 16-bit address `address`
def read(address):

	# if address has bits beyond bit 15, it means address is wider than 16-bits
	if (address >> 16 != 0):
		raise Error("Argument #1 `address` needs to be a 16-bit value at most")

	# first 8 bits 		- 00000101
	upper_half_of_addr = (address >> 8) & 0xFF
	lower_half_of_addr = (address & 0xFF)
	message = [READ, upper_half_of_addr, lower_half_of_addr, 0x00]

	response = _transfer(message)
	return response[-1]


# Writes a byte `value` to 16-bit address `address`
# Writing does not return anything.
def write(address, value):
	if (address >> 16 != 0):
		raise Error("Argument #1 `address` needs to be a 16-bit value at most")

	if (value >> 8 != 0):
		raise Error("Argument #2 `value` needs to be an 8-bit value at most")

	upper_half_of_addr = (address >> 8) & 0xFF
	lower_half_of_addr = (address & 0xFF)

	message = [WRITE, upper_half_of_addr, lower_half_of_addr, value, 0x00]

	_transfer(message)