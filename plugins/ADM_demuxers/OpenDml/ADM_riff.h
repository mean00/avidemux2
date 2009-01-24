#ifndef __ADM_RIFF_
#define  __ADM_RIFF_
class riffParser
{
public:
	FILE *fd;
	uint64_t startPos;
	uint64_t endPos;
	uint64_t curPos;
	uint8_t  _root;

	riffParser(const char *name);
	riffParser(riffParser *o, uint32_t size);
	~riffParser();
	uint64_t getPos( void );
	uint32_t read32(void );
	uint8_t endReached(void);
	uint8_t skip(uint32_t s);
	uint8_t read(uint32_t len, uint8_t *data);

};
#endif

