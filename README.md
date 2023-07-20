# bitbang_I2C_Mbed
A simple sample for I²C emulation by bit-banging

# What is this?
An I²C host operation sample by bit banging. 
This sample was made to perform its protocol abnormal situation called "SDA stuck". 

## SDA stuck
The I²C byte transfer always done with 9 clock (SCL) pulses contains 8 bit data and 1 bit ACK/NACK. 
The "SDA stuck" happens if the number of pulse is less than 9 in read transfer. The clock pulse missing may occured by noise or hardware problem. 
The target device sends data bits to host synchronized to the clock. The target prepare next bit after detecting falling edge of the clock. 
If some clock is missed, the target may keep the data line (SDA) to send the data bit. If the data bit is LOW, the I²C bus cannot go back to free (idle) state. 

## Bus clear
To recover this situation, I²C specification defines "bus clear". 
The bus clear can be done by 9 clock pulse generation followed by stop-condition. WIth these clock pulses, all remaining bits flushed. 
While the bus clear, the host side will keep the SDA line HIGH. That tells target transfer terminated by keep SDA HIGH (NAK) at 9th clock. 

![normal transfer](https://github.com/teddokano/additional_files/blob/main/bitbang_I2C_Mbed/normal%202%20bytes%20read.png) 
_Normal I²C transfer: 2 bytes read from target_

![SDA_Stuck and bus_clear](https://github.com/teddokano/additional_files/blob/main/bitbang_I2C_Mbed/bus_clear.png) 
_SDA_Stuck and bus_clear: The host intentionally send 5 clock pulses at second byte read. Then SDA stuck happend and host performed the bus clear_

# Hardware requirement
This sample code works on FRDM-K64F as an host MCU and P3T1085 (or its equivalent) as a target. 
The bitbanging is performed about 1MHz SCL frequency by DigitalInOut API on FRDM-K64F. 

![Boards](https://github.com/teddokano/additional_files/blob/main/bitbang_I2C_Mbed/boards.jpg) 

# Limitation
This is not intended to implement complete I²C features. 
Clock stretching and multi-master are not supported. 
