import RPi.GPIO as GPIO

# GPIO pin parameters
SerialInput_pin = 8
SerialClock_pin = 7
SerialOutput_pin = 11
Hold_pin = 10
ChipSelect_pin = 13

# application-specific constant parameters
memory_size = 65536      # Number of memory cells
address_bits = 16       # Number of address bits
data_bits = 8           # Number of data bits (8 in this case, for one byte)
instruction_bits = 8    # Number of instruction bits  

def setup():   
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(SerialInput_pin, GPIO.OUT)
    GPIO.setup(SerialClock_pin, GPIO.OUT)
    GPIO.setup(SerialOutput_pin, GPIO.IN)
    GPIO.setup(Hold_pin, GPIO.OUT)
    GPIO.setup(ChipSelect_pin, GPIO.OUT)
    GPIO.output(ChipSelect_pin, GPIO.LOW)


def checkerboard_test(memory_size):
    for address in range(memory_size):
        if address % 2 == 0:
            writeCommand = translate_command(False, address, 0xAA)
            write_byte_to_memory(writeCommand)
        else:
            writeCommand = translate_command(False, address, 0x55)
            write_byte_to_memory(writeCommand)
    for i in range(memory_size):
        readCommand = translate_command(True, i, 0)
        readValue = read_byte_from_memory(readCommand)
        if ~((readValue == 0xAA) & (i % 2 == 0)) or ~((readValue == 0x55) & (i % 2 == 0)):
            print(f"Checkerboard test failed at address {i}")
            return

def sequence_test(memory_size, datum):
    for address in range(memory_size):
        # Write 0xFF
        writeCommand = translate_command(False, address, datum)
        write_byte_to_memory(writeCommand)
    # Verify all cells
    for i in range(memory_size):
        readCommand = translate_command(True, i, 0)
        readValue = read_byte_from_memory(readCommand)
        if ~(readValue == datum):
            print(f"Sequence test failed at address {i}")
            return

def translate_command(RW, address, data):
    if(RW): binaryRW = bin(3)[2:]                       # Create binary string based on byte read/write intention
    else: binaryRW = bin(2)[2:]
    binaryRW = binaryRW.zfill(instruction_bits)         # Pad with zeros to ensure 8 bits

    binaryAddress = bin(address)[2:]                    # Convert address to binary string
    binaryAddress = binaryAddress.zfill(address_bits)   # Pad with zeros to ensure 16 bits

    binaryData = bin(data)[2:]                          # Convert address to binary string
    binaryData = binaryData.zfill(data_bits)            # Pad with zeros to ensure 8 bits

    if(RW): binaryCMD = ''.join(binaryRW,binaryAddress)
    else: binaryCMD = binaryAddress.join(binaryRW,binaryData)
    binaryCMD = binaryCMD[::-1]                         # Reverse the binary string

    # Convert binary string to list of integers
    binaryCMD = [int(bit) for bit in binaryCMD]

    return binaryCMD


def write_byte_to_memory(binaryCommand):
    GPIO.output(Hold_pin, GPIO.LOW)
    for bit in binaryCommand:
        GPIO.output(SerialInput_pin, bit)
        GPIO.output(SerialClock_pin, GPIO.HIGH)
        GPIO.output(SerialClock_pin, GPIO.LOW)
    GPIO.output(Hold_pin, GPIO.HIGH)
    pass

# Simulated memory read/write functions (replace with actual implementation)
def read_byte_from_memory(binaryCommand):
    byte = []
    GPIO.output(Hold_pin, GPIO.LOW)
    for bit in binaryCommand:
        GPIO.output(SerialInput_pin, bit)
        GPIO.output(SerialClock_pin, GPIO.HIGH)
        GPIO.output(SerialClock_pin, GPIO.LOW)
    for i in range(8):
        GPIO.output(SerialClock_pin, GPIO.HIGH)
        outputbit = GPIO.input(SerialOutput_pin)
        GPIO.output(SerialClock_pin, GPIO.LOW)
        byte.append(outputbit)
    GPIO.output(Hold_pin, GPIO.HIGH)
    return byte


setup()
# Perform checkerboard test
checkerboard_test(memory_size)
print("Checkerboard test complete!")

# Perform sequence tests
sequence_test(memory_size,0xEE)
print("Sequence test 1 for 1110 1110 complete!")

sequence_test(memory_size,0x66)
print("Sequence test 2 for 0110 0110 complete!")

sequence_test(memory_size,0x99)
print("Sequence test 3 for 1001 1001 complete!")

sequence_test(memory_size,0x22)
print("Sequence test 4 for 0010 0010 complete!")

sequence_test(memory_size,0x33)
print("Sequence test 5 for 0011 0011 complete!")

sequence_test(memory_size,0xCC)
print("Sequence test 6 for 1100 1100 complete!")