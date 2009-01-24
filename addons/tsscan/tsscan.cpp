/***************************************************************************
  Small scanner for mpeg TS stream
    
            
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
sync_byte 8 bslbf 
transport_error_indicator 1 bslbf       1: Error present
payload_unit_start_indicator 1 bslbf    If 1 a unit start in this packet, there is an offset later
transport_priority 1 bslbf              x
PID 13 uimsbf 
transport_scrambling_control 2 bslbf  00 not scrambled
adaptation_field_control 2 bslbf      00 01:Payload only 10:adap only 11: payload + adap
continuity_counter 4 uimsbf 
if(adaptation_field_control=='10' || adaptation_field_control=='11
'){ adaptation_field() }
if(adaptation_field_control=='01' || adaptation_field_control=='11') 
{ for (i=0;i<N;i++){ data_byte 8 bslbf } } } bslbf Bit string, 

pcr is pts in us
*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define uint8_t unsigned char
#define uint32_t unsigned long int

#define TS_PACKET 188
#define VERSION 0.1
int sync( FILE *fd);
int parse(FILE *fd,FILE *fo,int pid);
int usage(void );
int push(int pid, int streamid);
int decodeDescriptor(uint8_t *data, uint32_t len);

uint32_t stack[100];
uint32_t stacked=0;

uint32_t nbPid=0;
uint32_t pidTab[256];

uint8_t buffer[TS_PACKET*4];

int isPMT(uint32_t pid);
int decodeSection(uint8_t *data, uint32_t len,uint32_t *remaining);
void decodePat(void);
void decodePMT(void);

//_____________________________
//_____________________________
int main(int argc, char **argv)
{
FILE *fd=NULL;
FILE *fo=NULL;
int pid=0;
	usage();
	if(!argv[1]) exit(0);
	
	fd=fopen(argv[1],"rb");
	if(!fd)
	{
		
			printf("Cannot open %s\n",argv[1]);
	}
	if(argc>3)
	{
		pid=atoi(argv[2]);
		if(!pid)
		{
			printf("pid must be non null\n");
			exit(0);
		}
		fo=fopen(argv[3],"wb");
		if(!fo)
		{
			printf("Cannot output to file\n");
			exit(0);
		
		}
	}
	if(!sync(fd)) { printf("Cannot find sync\n");exit(-1);}
	parse(fd,fo,pid);
	fclose(fd);
	if(fo) fclose(fo);
	printf("\nDone.\n");

}

int sync( FILE *fd)
{
int n=0;

	fread(buffer,TS_PACKET,3,fd);
	while(n<TS_PACKET*2)
	{
		if(buffer[n]==0x47 && buffer[n+TS_PACKET]==0x47)
		{
			fseek(fd,n,SEEK_SET);
			return 1;
		}
		n++;
	}
	return 0;
}
// Parse until we find a PES header to identify the stream
//
int parse(FILE *fd,FILE *fo, int cpid)
{
int loop=0;
int pid,adapt,start,len;

	while(loop<350000)
	{
		loop++;		
		if(1!=fread(buffer,TS_PACKET,1,fd))
		{
			printf("Read error\n");
			 return 0;
		}
		if(buffer[0]!=0x47)
		{
			printf("Sync lost\n");
			return 0;
		}
		pid=((buffer[1]<<8)+buffer[2]) & 0x1FFF;
		//if(pid<0x10) continue; // PAT etc...
		if(!pid)
		{
			decodePat();
			continue;
		}
		if(isPMT(pid))
			{
				printf("PMT Id: 0x%x\n",pid);
				decodePMT();
				continue;
			}
		if((pid==cpid ) && fo)
		{
			fwrite(buffer,TS_PACKET,1,fo);
		}
		adapt=(buffer[3]&0x30)>>4;
		if(!(adapt&1)) continue; // no payload
		
		start=4;
		len=184;
		if(adapt==3) //payload + adapt
		{			
			start+=1+buffer[4];
			len-=1+buffer[4];
		}
		
		// We got a packet of the correct pid
		if(!(buffer[1]&0x40)) // PES header ?
			continue;
		
		
		if(buffer[start] || buffer[start+1] || buffer[start+2]!=01)
		{
			printf("Weird...%x :%x %x %X\n",pid,buffer[start],buffer[start+1],buffer[start+2]);
			continue;		
		}
		push(pid,buffer[start+3]);
	}	
	return 1;
}
int isPMT(uint32_t pid)
{
	for(uint32_t i=0;i<nbPid;i++)
		if(pid==pidTab[i]) return 1;
	return 0;
}
void decodePMT(void)
{
	int index=0;
	int adapt,len,l;
	uint32_t sid,pid,r;

	if(!(buffer[1]&0x40)) // Start
	{
		printf("No start indicator in pat ?\n");
		return;
	}
	index=5+buffer[4];
	len=TS_PACKET-5-buffer[4];
	// Now we got the section for PAT
	printf("PMT found:\n");
	l=decodeSection(buffer+index,len,&r);
	len=r;
	index+=l;
	// now body
	// PCR
	uint32_t pcr,stream,pcrlen,streamlen;

	pcr=((buffer[index]&0x1f)<<8)+buffer[index+1];
	index+=2;
	len-=2;
	pcrlen=((buffer[index]&0xf)<<8)+buffer[index+1];
	index+=2;
	len-=2;
	printf("\t**PMT infos**\n");
	printf("\tPcr pid lock :%x\n",pcr);
	printf("\tPcr info len :%x\n",pcrlen);
	
	while(len>=5)
	{
		char *type="unknown";
		switch(buffer[index])
		{
			case 0x2: type="mpeg video";break;
			case 0x3: type="mpeg1 audio";break;
			case 0x4: type="mpeg2 audio";break;
			case 0x1b: type="h264 video";break;
                        case 0x6: type="Private Stream";break;
			default:;
		}
		printf("\tStream Type    :0x%x (%s)\n",buffer[index],type);
		index++;

		stream=((buffer[index]&0x1f)<<8)+buffer[index+1];
		index+=2;
		
		streamlen=((buffer[index]&0xf)<<8)+buffer[index+1];
		index+=2;
		len-=5;
		
		printf("\tStream Pid     :0x%x\n",stream);
		printf("\tStream info len:0x%x\n",streamlen);
		printf("\t\t");
		if(streamlen)
		{
			uint32_t len2=streamlen, idx2=index,taglen;
			for(uint32_t i=0;i<streamlen;i++) printf(" %02x",buffer[index+i]);
			while(len2)
			{
				taglen=decodeDescriptor(buffer+idx2,len2);
				len2=len2-taglen;
				idx2+=taglen;
			}
			printf("\n");
		
		}
		index+=streamlen;len-=streamlen;
		
	}
	printf("\n\t Remaining datas:%d\n",len);
}
void decodePat(void)
{
	int index=0;
	int adapt,len,l;
	uint32_t sid,pid,r;

	if(!(buffer[1]&0x40)) // Start
	{
		printf("No start indicator in pat ?\n");
		return;
	}
	index=5+buffer[4];
	len=TS_PACKET-5-buffer[4];
	// Now we got the section for PAT
	printf("Pat found:\n");
	l=decodeSection(buffer+index,len,&r);
	len=r;
	index+=l;
	// Here is the pat data itself
	printf("\t**PMT**\n");
	while(len>=4)
	{
		sid=buffer[index+1]+((buffer[index]&0xff)<<8);
		pid=buffer[index+3]+((buffer[index+2]&0xf)<<8);
		printf("\tsid:%0x pid:%x\n",sid,pid);
		pidTab[nbPid++]=pid;
		printf("\n");
		len-=4;
		index+=4;
	}

}	
int decodeSection(uint8_t *data, uint32_t len,uint32_t *remaining)
{
uint32_t val,slen;
	printf("\t**Section Header**\n");
	slen=((data[1]&0xf)<<8)+data[2];
	printf("\tTable Id      :0x%x\n",data[0]);
	printf("\tSection Len   :%lu\n",slen);
	val=((data[3]&0xf)<<8)+data[4];
	// 5 skipped
	printf("\tSection Id    :%lu\n",val);
	printf("\tSection No    :%lu\n",data[6]);
	printf("\tSection Max   :%lu\n",data[7]);
	*remaining=slen-5-4;
    	return 8;

}	
int usage(void)
{
	printf("\nSimple Mpeg Ts stream scanner\n");
	printf("\nVersion %1.2f by mean\n",VERSION);
	printf("\n\ntsscan filein [pid in decimal fileout]\n");
	return 1;
}
int push(int pid, int streamid)
{
uint32_t key;
	key=(pid<<16)+streamid;
	for(int i=0;i<stacked;i++)
	{
		if(stack[i]==key) return 1;
	}
	stack[stacked++]=key;
	printf("Found pid: %x with stream : %x\n",pid,streamid);
	return 1;

}
int decodeDescriptor(uint8_t *data, uint32_t len)
{
	uint32_t tag,taglen,tagid;
	tag=data[0];
	taglen=data[1];
	tagid=data[2];
	printf("\n\t\t\t Tag :%x len:%d id:%x =",tag,taglen,tagid);
	switch(tag)
	{
		case 0x52: printf("Stream identifier");break;
                case 0x59: printf("DVB subtitles");break;
                case 0x56: printf("Teletext");break;
		case 0x0a: printf("Language descriptor");break;
		default : printf("unknown");break;
	}
	return 2+taglen;

}
