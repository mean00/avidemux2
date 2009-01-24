//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: Mean (fixounet@free.fr)
//
// Copyright: See COPYING file that comes with this distribution GPL.
//
// FIXME : We need total frame # to do only pass2
//
#ifndef ADM_RTCL
#define ADM_RTCL
typedef enum 
{
	RF_I=1,
	RF_P=2,
	RF_B=3,
}ADM_rframe;

typedef enum
{
	RS_IDLE,
	RS_PASS1,
	RS_PASS2
}ADM_rstate;

typedef struct
{
	uint32_t quant;
	uint32_t size;
	ADM_rframe type;
}ADM_pass_stat;
#define FORBIDDEN {ADM_assert(0);return 0;}

class ADM_ratecontrol
{
protected:
		uint32_t _nbFrames;
		uint32_t _fps1000;
		char 	*_logname;
		ADM_rstate _state;
public:
			ADM_ratecontrol(uint32_t fps1000, char *logname);
	virtual 	~ADM_ratecontrol();
	/** Maxbr & minbr in Bps, vbvsize in kBytes); Default is none */
	virtual 	uint8_t setVBVInfo(uint32_t maxbr,uint32_t minbr, uint32_t vbvsize) FORBIDDEN ;
	virtual		uint8_t startPass1( void )FORBIDDEN;
	virtual		uint8_t logPass1(uint32_t qz, ADM_rframe ftype,uint32_t size)FORBIDDEN;
	virtual		uint8_t startPass2( uint32_t size ,uint32_t nbFrame)FORBIDDEN;
	virtual		uint8_t getQz( uint32_t *qz, ADM_rframe *type )FORBIDDEN;
	virtual		uint8_t logPass2( uint32_t qz, ADM_rframe ftype,uint32_t size)FORBIDDEN;

};

class ADM_newXvidRc :public ADM_ratecontrol
{
protected:
                        uint32_t _totalFrame;
public:
                        ADM_newXvidRc(uint32_t fps1000, char *logname);
        virtual 	~ADM_newXvidRc() ;
                        /** Maxbr & minbr in kbps, vbvsize in kBytes); Default is none */
        virtual 	uint8_t setVBVInfo(uint32_t maxbr,uint32_t minbr, uint32_t vbvsize);
        virtual		uint8_t startPass1( void );
        virtual		uint8_t logPass1(uint32_t qz, ADM_rframe ftype,uint32_t size);
        virtual		uint8_t startPass2( uint32_t size,uint32_t nbFrame );
        virtual		uint8_t getQz( uint32_t *qz, ADM_rframe *type );
        virtual		uint8_t logPass2( uint32_t qz, ADM_rframe ftype,uint32_t size);
                        // Used for VBV
                        uint8_t getInfo(uint32_t framenum, uint32_t *qz, uint32_t *size,ADM_rframe *type );

};
#define AVG_LOOKUP 5
class ADM_newXvidRcVBV :public ADM_ratecontrol
{
protected:
                        ADM_newXvidRc	*rc;
                        uint32_t	_minbr,_maxbr,_vbvsize;
                        ADM_pass_stat  *_stat;
                        uint32_t	*_lastSize;
                        uint32_t	_roundup;
                        uint32_t	_frame;
                        uint32_t	_vbv_fullness;
                        uint32_t	_byte_per_image;
                        double		_compr[3][AVG_LOOKUP];  
                        uint32_t   _idxI,_idxP,_idxB;
                        
                        uint8_t 	project(uint32_t framenum, uint32_t q, ADM_rframe frame);
                        uint8_t 	checkVBV(uint32_t framenum, uint32_t q, ADM_rframe frame);
                        float 		getRatio(uint32_t newq, uint32_t oldq, float alpha);
                        float 		getComp(int oldbits, int qporg, int newbits, int qpused);
			
public:
                        ADM_newXvidRcVBV(uint32_t fps1000, char *logname);
        virtual 	~ADM_newXvidRcVBV() ;
        /** Maxbr & minbr in kbps, vbvsize in kBytes); Default is none */
        virtual 	uint8_t setVBVInfo(uint32_t maxbr,uint32_t minbr, uint32_t vbvsize);
        virtual		uint8_t startPass1( void );
        virtual		uint8_t logPass1(uint32_t qz, ADM_rframe ftype,uint32_t size);
        virtual		uint8_t startPass2( uint32_t size,uint32_t nbFrame );
        virtual		uint8_t getQz( uint32_t *qz, ADM_rframe *type );
        virtual		uint8_t logPass2( uint32_t qz, ADM_rframe ftype,uint32_t size);
        static          uint8_t verifyLog(const char *file,uint32_t nbFrame);
};

#endif
//EOF
