  uint64_t  asfPacket::read64(void)
      {
        uint64_t lo,hi;
        lo=read32();
        hi=read32();
        _offset+=8;
        ADM_assert(_offset<=pakSize);
        return lo+(hi<<32); 
        
      }
      uint32_t   asfPacket::read32(void)
      {
        uint8_t c[4];
        fread(c,4,1,_fd);
        _offset+=4;
        ADM_assert(_offset<=pakSize);
        return c[0]+(c[1]<<8)+(c[2]<<16)+(c[3]<<24);
      }
      uint32_t   asfPacket::read16(void)
      {
        uint8_t c[2];
        fread(c,2,1,_fd);
        _offset+=2;
        ADM_assert(_offset<=pakSize);
        return c[0]+(c[1]<<8);
      }
      
      uint8_t   asfPacket::read8(void)
      {
        uint8_t c[1];
        fread(c,1,1,_fd);
        _offset++;
        ADM_assert(_offset<=pakSize);
        return c[0];
      }
