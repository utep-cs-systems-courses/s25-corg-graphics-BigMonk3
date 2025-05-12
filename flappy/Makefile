# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:flappy.elf

#additional rules for files
flappy.elf: ${COMMON_OBJECTS} flappy.o wdt_handler.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

load: flappy.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.elf
