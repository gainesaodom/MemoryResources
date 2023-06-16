import RPi.GPIO as GPIO

# GPIO pin parameters
SerialInput_pin = 10
SerialClock_pin = 11 
SerialOutput_pin = 9
Hold_pin = 8
ChipSelect_pin = 7

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


def checkerboard_test(memory_size,step):
    for address in range(memory_size):
        if ((address % 2 == 0) and (step == 1)) or ((address % 2 == 1) and (step == 2)) :
            writeCommand = translate_command(False, address, 0xAA)
            write_byte_to_memory(writeCommand)
        else:
            writeCommand = translate_command(False, address, 0x55)
            write_byte_to_memory(writeCommand)
    for i in range(memory_size):
        readCommand = translate_command(True, i, 0)
        readValue = read_byte_from_memory(readCommand)
        if ((readValue != 0xAA) and (i % 2 == 0) and (step == 1)) or ((readValue != 0x55) and (i % 2 == 1) and (step == 1)):
            print(f"Checkerboard test part 1 failed at address {i}")
            return
        if ((readValue != 0xAA) and (i % 2 == 1) and (step == 2)) or ((readValue != 0x55) and (i % 2 == 0) and (step == 2)):
            print(f"Checkerboard test part 2 failed at address {i}")
            return
        
def march_A_test(memory_size):
    for address in range(memory_size):
        # M0   
        writeValue = 0x00            
        for i in range(data_bits): 

            writeCommand = translate_command(False, address, writeValue)
            write_byte_to_memory(writeCommand)
    for address in range(memory_size):
        # M1
        readCommand = translate_command(True, address, 0)
        readValue = read_byte_from_memory(readCommand)
        if (readValue != oldWriteValue):
            print(f"March A test failed at address {address}")
            return
        writeValue = 0xFF
        writeCommand1 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand1)
        writeValue = 0x00
        writeCommand0 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand0)
        write_byte_to_memory(writeCommand1)
        oldWriteValue = 0xFF
    for address in range(memory_size):
        # M2
        readCommand = translate_command(True, address, 0)
        readValue = read_byte_from_memory(readCommand)
        if (readValue != oldWriteValue):
            print(f"March A test failed at address {address}")
            return       
        writeValue = 0x00
        writeCommand0 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand0)
        writeValue = 0xFF
        writeCommand1 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand1)
    for address in reversed(range(memory_size)):
        # M3
        readCommand = translate_command(True, address, 0)
        readValue = read_byte_from_memory(readCommand)
        if (readValue != oldWriteValue):
            print(f"March A test failed at address {address}")
            return       
        writeValue = 0x00
        writeCommand0 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand0)
        writeValue = 0xFF
        writeCommand1 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand1)
        write_byte_to_memory(writeCommand0)
        oldWriteValue = 0x00
    for address in reversed(range(memory_size)):
        # M4
        readCommand = translate_command(True, address, 0)
        readValue = read_byte_from_memory(readCommand)
        if (readValue != oldWriteValue):
            print(f"March A test failed at address {address}")
            return       
        writeValue = 0xFF
        writeCommand1 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand1)
        writeValue = 0x00
        writeCommand0 = translate_command(False, address, writeValue)
        write_byte_to_memory(writeCommand0)
    pass

def readZero():
    pass

def readOne():
    pass
'''
def sequence_test(memory_size, datum):
    for address in range(memory_size):
        # Write datum
        writeCommand = translate_command(False, address, datum)
        write_byte_to_memory(writeCommand)
    # Verify all cells
    for i in range(memory_size):
        readCommand = translate_command(True, i, 0)
        readValue = read_byte_from_memory(readCommand)
        if readValue != datum:
            print(f"Sequence test failed at address {i}")
            return
'''
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

# Simulated memory read/write functions
def read_byte_from_memory(binaryCommand):
    byte = []
    GPIO.output(Hold_pin, GPIO.LOW)
    for bit in binaryCommand:
        GPIO.output(SerialInput_pin, bit)
        GPIO.output(SerialClock_pin, GPIO.HIGH)
        GPIO.output(SerialClock_pin, GPIO.LOW)
    for i in range(data_bits):
        GPIO.output(SerialClock_pin, GPIO.HIGH)
        outputbit = GPIO.input(SerialOutput_pin)
        GPIO.output(SerialClock_pin, GPIO.LOW)
        byte.append(outputbit)
    GPIO.output(Hold_pin, GPIO.HIGH)
    return byte

"""
setup()
# Perform checkerboard test
checkerboard_test(memory_size, 1)
print("Checkerboard test step 1 complete!")

checkerboard_test(memory_size, 2)
print("Checkerboard test step 2 complete!")

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
"""