#ifndef __BITSREADER__
#define __BITSREADER__

#define BR_BUFFER_SIZE 256 
class bitsReader{

	FILE 		*_fd;
	uint8_t 	_buffer[BR_BUFFER_SIZE];
	uint8_t 	_curByte;
	uint32_t	_rdIndex;
	uint32_t	_wrIndex;
	uint8_t		_rdBits;
	uint64_t	_size;
	uint64_t	_pos;

public:
			bitsReader(void);
	uint8_t		open(const char *name);
			~bitsReader();
	uint8_t 	forward(uint32_t nbBits);
	uint8_t 	read(uint32_t nbBits,uint32_t *val);
	uint8_t	sync( void );
	uint8_t	syncMpeg( uint8_t *sc );	
	uint8_t	readByte( void );
	uint8_t	read1bit( void );
	uint32_t	getPos( void );
};

#endif
