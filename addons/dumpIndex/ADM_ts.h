
class tsHeader
{
public:
    bool readIndex(indexFile *index);
    bool processVideoIndex(char *buffer);
    bool readVideo(indexFile *index);
};

class dmxFrame
{
public:
    int startAt;
    int index;
    uint64_t pts,dts;
    int type;
    int len;
    
};