#include "mbed.h"

#define SDA_HIGH    sda = 1;
#define SDA_LOW     sda = 0;

#define SCL_HIGH    scl = 1;
#define SCL_LOW     scl = 0;

//  for "mbed LPC1768"
#if 0
DigitalInOut sda( p27 );
DigitalInOut scl( p28 );
#endif

//  for "FRDM-K64F"
//DigitalInOut sda( A4 );   //  for PCT2075-ARD
//DigitalInOut scl( A5 );   //  for PCT2075-ARD
DigitalInOut sda( I2C_SDA );    //  for P3T1085-ARD
DigitalInOut scl( I2C_SCL );    //  for P3T1085-ARD

void    initialize( void );
int     i2c_send( uint8_t address, uint8_t *data, int length );
int     i2c_receive( uint8_t address, uint8_t *data, int length );
void    start_condition( void );
void    stop_condition( void );
int     send_a_byte( uint8_t data );
uint8_t receive_a_byte( int last_byte );
void    bus_clear( void );

int     i2c_receive_short( uint8_t address, uint8_t *data, int length, int n_pulse = 3 );
uint8_t receive_short( int last_byte, int n_pulse = 3  );

#define TARGET_ADDRESS  0x90    //  for P3T1085-ARD

int main() {
	initialize();

//    uint8_t data[]  = { 0xAA, 0x55 };
	uint8_t data[ 2 ];

	while( 1 ) {
		//  Normal receive transfer
		i2c_receive( TARGET_ADDRESS, data, sizeof( data ) );
	
		printf( "%f\r\n", (data[ 0 ] << 8 | data[ 1 ]) / 256.0 );

		//  In next function call, SDA stuck may occur.
        //  The bus-clear will be performed when the SDA stuck detected after stop-condition generated.
        //  The 4th argument is number of pulses (including SCL pulse on ACK) in second byte read. 
		i2c_receive_short( TARGET_ADDRESS, data, sizeof( data ), 5 );
	
		wait( 1 );
	}
}

void initialize( void )
{
	sda.mode( PullUp );
	scl.mode( PullUp );

	sda.output();
	scl.output();

	sda = 1;
	scl = 1;
}

int i2c_send( uint8_t address, uint8_t *data, int length )
{
	int nak = 0;

	start_condition();
	nak = send_a_byte( address );

	if ( nak )
	{
		stop_condition();        
		return 1;
	}

	for ( int i = 0; i < length; i++ )
	{
		nak = send_a_byte( data[ i ] );

		if ( nak )
			break;
	}

	stop_condition();

	return nak;
}

int i2c_receive( uint8_t address, uint8_t *data, int length )
{
	int nak = 0;

	start_condition();
	nak = send_a_byte( address | 0x1 );

	if ( nak )
	{
		stop_condition();        
		return 1;
	}

	for ( int i = 0; i < length; i++ )
	{
		data[ i ]   = receive_a_byte( ( i == (length - 1)) ? 1 : 0 );
	}

	stop_condition();

	return 0;
}

int i2c_receive_short( uint8_t address, uint8_t *data, int length, int n_pulse )
{
	int nak = 0;

	start_condition();
	nak = send_a_byte( address | 0x1 );

	if ( nak )
	{
		stop_condition();        
		return 1;
	}

	for ( int i = 0; i < length; i++ )
	{
		if (i == (length - 1))
			data[ i ]   = receive_short( 1, n_pulse );
		else
			data[ i ]   = receive_a_byte( 0 );
	}

	stop_condition();

    wait_ms( 10 );
    bus_clear();


	return 0;
}

void start_condition( void )
{
	sda = 1;
	scl = 1;
	sda = 0;
	scl = 0;
}

void stop_condition( void )
{
	scl = 0;
	sda = 0;
	scl = 1;
	sda = 1;

#if 0
	sda.input();

    if ( !sda ) //  SDA stuck is happening!
        bus_clear();

    sda.output();
#endif
}

int send_a_byte( uint8_t data )
{
	for ( int i = 7; i >= 0; i-- )
	{
		sda = (data >> i) & 0x1;
		scl = 1;
		scl = 0;
	}

	sda.input();
	scl = 1;
	int val = sda;
	scl = 0;

	sda.output();

	return val;
}

uint8_t receive_a_byte( int last_byte )
{
	uint8_t data    = 0;
	sda.input();

	for ( int i = 7; i >= 0; i-- )
	{
		scl = 1;
		data |= (sda & 0x1) << i;
		scl = 0;
	}

	sda.output();
	sda = last_byte;
	scl = 1;
	scl = 0;

	return data;
}

uint8_t receive_short( int last_byte, int n_pulse )
{
	uint8_t data    = 0;
	sda.input();

	for ( int i = (n_pulse - 2); i >= 0; i-- )
	{
		scl = 1;
		data |= (sda & 0x1) << i;
		scl = 0;
	}

    if ( n_pulse )
    {
        sda.output();
        sda = last_byte;
        scl = 1;
        scl = 0;
    }

	return data;
}

void bus_clear( void )
{
    scl = 0;
	receive_a_byte( 1 );
	stop_condition();
}
