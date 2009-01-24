class baseRT
{
public:
		baseRT(void ) ;
virtual	~baseRT();
	
virtual	int 		SetSize(int *w, int *h) ;
virtual	int 		SetFormat(int *format) ;
virtual	int 		InitLong(char  *data, int w, int h)  ;

virtual	 void	 Decompress(int8_t *sp, uint8_t **planes) ;


};
class baseRTold : public baseRT
{
public:
		baseRTold(void) ;
virtual	~baseRTold();
	
virtual	int 		SetSize(int *w, int *h);
virtual	 int 		SetFormat(int *format);
virtual	 int 		InitLong(char  *data, int w, int h) ;

virtual	  void	 Decompress(int8_t *sp, uint8_t **planes);


};
