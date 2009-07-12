
/***************************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include <libxml/tree.h>
#include <libxml/parser.h>

#include "ADM_default.h"
#include "ADM_quota.h"

#include <unistd.h>	/* access(), R_OK */
#include <errno.h>	/* errno, ENOENT */
#include <dirent.h>

#undef free
extern char *ADM_getBaseDir(void);
#define CONFIG "config"
extern int  qxmlSaveFormatFile(const char *filename, xmlDocPtr cur, int format);
static char *checkDirAccess(char *home);
extern char *ADM_escape(const ADM_filename *incoming);
class preferences *prefs;
// <prefs_gen>
typedef enum {
	UINT,
	INT,
	ULONG,
	LONG,
	FLOAT,
	STRING,
	FILENAME
} types;

typedef struct {
	const char *name;
	types type;
	const char *default_val;
	char *current_val;
	const char *minimum;
	const char *maximum;
} opt_def;

static opt_def opt_defs [] = {
	{"codecs.svcd.enctype",		UINT,	"0",	NULL,	"0",	"99999"},
	{"codecs.svcd.bitrate",		UINT,	"1500000",NULL,	"16000","2748000"},
	{"codecs.svcd.quantizer",		UINT,	"4",	NULL,	"2",	"31"	},
	{"codecs.svcd.finalsize",	UINT,	"700",	NULL,	"0",	"99999"},
	{"codecs.svcd.interlaced",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.svcd.bff",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.svcd.widescreen",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.svcd.matrix",		UINT,	"0",	NULL,	"0",	"99999"},
	{"codecs.svcd.gopsize",		UINT,	"12",	NULL,	"0",	"99999"},
	{"codecs.svcd.maxbitrate",		UINT,	"2500",	NULL,	"0",	"2748"	},
	{"codecs.dvd.enctype",		UINT,	"0",	NULL,	"0",	"99999"},
	{"codecs.dvd.bitrate",		UINT,	"1500000",NULL,	"16000","9900000"},
	{"codecs.dvd.quantizer",		UINT,	"4",	NULL,	"2",	"31"	},
	{"codecs.dvd.finalsize",	UINT,	"700",	NULL,	"0",	"99999"},
	{"codecs.dvd.interlaced",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.dvd.bff",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.dvd.widescreen",		UINT,	"0",	NULL,	"0",	"1"	},
	{"codecs.dvd.matrix",		UINT,	"0",	NULL,	"0",	"99999"},
	{"codecs.dvd.gopsize",		UINT,	"12",	NULL,	"0",	"99999"},
	{"codecs.dvd.maxbitrate",		UINT,	"9000",	NULL,	"0",	"9900"	},
	{"codecs.xvid.enctype",		UINT,	"0",	NULL,	"0",	"99999"},
	{"codecs.xvid.quantizer",		UINT,	"4",	NULL,	"2",	"32"	},
	{"codecs.xvid.bitrate",		UINT,	"1500000",NULL,	"17",	"5900000"},
	{"codecs.xvid.finalsize",	UINT,	"700",	NULL,	"0",	"3999"	},
	{"codecs.preferredcodec",		STRING,"FFmpeg4",NULL, NULL, NULL },
	{"filters.subtitle.fontname",		FILENAME,"/usr/share/fonts/truetype/DejaVuSans.ttf",NULL, NULL, NULL },
	{"filters.subtitle.charset",		STRING,"ISO-8859-1",NULL, NULL, NULL },
	{"filters.subtitle.fontsize",		UINT,	"24",	NULL,	"1",	"576"	},
	{"filters.subtitle.ypercent",	INT,	"255",	NULL,	"0",	"255"	},
	{"filters.subtitle.upercent",		INT,	"0",	NULL,	"0",	"255"	},
	{"filters.subtitle.vpercent",		INT,	"0",	NULL,	"0",	"255"	},
	{"filters.subtitle.selfadjustable",		UINT,	"0",	NULL,	"0",	"1"	},
	{"filters.subtitle.usebackgroundcolor",		UINT,	"0",	NULL,	"0",	"1"	},
	{"settings.mpegsplit",	UINT,	"790",	NULL,	"400",	"5000"	},
	{"device.audiodevice",		STRING,"ALSA",	NULL, NULL, NULL },
	{"device.audio.alsa_device",		STRING,"dmix",	NULL, NULL, NULL },
	{"device.videodevice",		UINT,	"0",	NULL,	"0",	"10"	},
	{"default.postproc_type",		UINT,	"3",	NULL,	"0",	"7"	},
	{"default.postproc_value",		UINT,	"3",	NULL,	"0",	"5"	},
	{"lastfiles.file1",		FILENAME,"",	NULL, NULL, NULL },
	{"lastfiles.file2",		FILENAME,"",	NULL, NULL, NULL },
	{"lastfiles.file3",		FILENAME,"",	NULL, NULL, NULL },
	{"lastfiles.file4",		FILENAME,"",	NULL, NULL, NULL },
	{"lastdir_read",		FILENAME,"",	NULL, NULL, NULL },
	{"lastdir_write",		FILENAME,"",	NULL, NULL, NULL },
	{"lame_cli",		STRING,"",	NULL, NULL, NULL },
	{"pipe_cmd",		STRING,"",	NULL, NULL, NULL },
	{"pipe_param",		STRING,"",	NULL, NULL, NULL },
	{"lame_path",		STRING,"",	NULL, NULL, NULL },
	{"toolame_path",		STRING,"",	NULL, NULL, NULL },
	{"lvemux_path",		STRING,"",	NULL, NULL, NULL },
	{"requant_path",		STRING,"",	NULL, NULL, NULL },
	{"message_level",		UINT,	"2",	NULL,	"0",	"2"	},
	{"feature.swap_if_A_greater_than_B",		UINT,	"1",	NULL,	"0",	"1"	},
	{"feature.svcdres.preferedsourceratio",		STRING,"1:1",	NULL, NULL, NULL },
	{"feature.saveprefsonexit",		UINT,	"1",	NULL,	"0",	"1"	},
	{"feature.ignoresavedmarkers",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.disable_nuv_resync",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.tryautoidx",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.use_odml",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.use_systray",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.reuse_2pass_log",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.audiobar_uses_master",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.threading.lavc",		UINT,	"0",	NULL,	"0",	"32"	},
	{"feature.cpu_caps",		UINT,	"4294967295",NULL,	"0",	"4294967295"},
	{"feature.mpeg_no_limit",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.auto_buildmap",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.auto_rebuildindex",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.auto_unpack",		UINT,	"0",	NULL,	"0",	"1"	},
	{"downmixing.prologic",		UINT,	"2",	NULL,	"0",	"2"	},
	{"filters.autoload.path",		STRING,"/tmp/",NULL, NULL, NULL },
	{"filters.autoload.active",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.alternate_mp3_tag",		UINT,	"1",	NULL,	"0",	"1"	},
	{"feature.vdpau",		UINT,	"0",	NULL,	"0",	"1"	},
	{"feature.global_glyph.active",		UINT,	"1",	NULL,	"0",	"1"	},
	{"feature.global_glyph.name",		STRING,"",	NULL, NULL, NULL },
	{"priority.encoding",		UINT,	"3",	NULL,	"0",	"4"	},
	{"priority.indexing",		UINT,	"3",	NULL,	"0",	"4"	},
	{"priority.playback",		UINT,	"0",	NULL,	"0",	"4"	}
};

int num_opts = 79;
// </prefs_gen>


/*
** we cannot put this into the header file, cause libxml headers
** are not reachable in all directories/Makefiles
** way 1: make xml headers reachable
** way 2: write my own (extended) class definition here
** way 3: use an external static variable only here
**
** i've choosed way 3 ;-)
*/
static xmlDocPtr xdoc;
#if 0
int xmlSaveNoEmptyTags  = 1;      /* save empty tags as <empty></empty> */
int xmlIndentTreeOutput = 1;      /* try to indent the tree dumps */
#endif
void erase_blank_nodes(xmlNodePtr cur){
  /* erase all blank-nodes recursive; they deny IndentTreeOutput !!! */
  xmlNodePtr run = cur;
  xmlNodePtr erase;
   while( run ){
      if( xmlIsBlankNode(run) ){
         erase = run;
         run = run->next;
         xmlUnlinkNode(erase);
         xmlFreeNode(erase);
         continue;
      }
      if( run->children )
         erase_blank_nodes(run->children); /* Blank nodes has no childs ;-) */
      run = run->next;
   }
}

xmlNodePtr goto_node(xmlNodePtr cur, const char *str){
  xmlNodePtr ret;
   ADM_assert(cur);
   ret = cur->children;
   while( ret ){
      if( !strcmp((char *)ret->name,str) )
         return ret;
      ret = ret->next;
   }
   return ret;
}

xmlNodePtr goto_node_with_create(xmlNodePtr cur, const char *str){
  xmlNodePtr nn = goto_node(cur, str);
	if( ! nn ){
		if( !(nn=xmlNewChild(cur,NULL,(xmlChar *)str,NULL)) ){
			fprintf(stderr,"xmlNewChild() failed\n");
			return NULL;
		}
	}
	return nn;
}

void set_content(const char *option, xmlNodePtr x){
   int idx = -1;
   char *str,*str2;
	for( int i=0; i < num_opts; i++ ){
		if( !strcmp(opt_defs[i].name,option) ){
			idx = i;
			break;
		}
	}
	if( idx == -1 ){
		fprintf(stderr,"option \"%s\" not defined.\n",option);
		return;
	}
	if( !(str = (char*)xmlNodeGetContent(x)) ){
		fprintf(stderr,"no content in xmlNode for option \"%s\".\n",option);
		return;
	}
	
	str2=(char *)ADM_alloc(strlen(str)+1);
	strcpy(str2,str);
	free(str);
	
	if( opt_defs[idx].current_val )
		ADM_dealloc(opt_defs[idx].current_val);
	opt_defs[idx].current_val = str2;
	#ifdef DEBUG_PREFS
	fprintf(stderr,"Prefs: %s => %s\n",opt_defs[idx].name,opt_defs[idx].current_val);
	#endif
}


#include "prefs.h"

preferences::preferences(){
	internal_lastfiles[0] = internal_lastfiles[1] = NULL;
	internal_lastfiles[2] = internal_lastfiles[3] = NULL;
	internal_lastfiles[4] = NULL;
	
	xdoc = NULL;
	
}

preferences::~preferences(){
  unsigned int idx;
	for( idx=0; idx < 4; idx++ ){
		if( internal_lastfiles[idx] )
			ADM_dealloc(internal_lastfiles[idx]);
	}
	
	if( xdoc )
		xmlFreeDoc(xdoc);
	
}

int preferences::load(){
   xmlNodePtr p;
   char *rcfile;
   char *home;
   char *dir_adm;
   
   char buf[1024];
   DIR   *dir;

        dir_adm=ADM_getBaseDir();
        if(!dir_adm) return RC_FAILED;

        rcfile=new char[strlen(dir_adm)+4+strlen(CONFIG)];
        strcpy(rcfile,dir_adm);
        strcat(rcfile,"/");
        strcat(rcfile,CONFIG);
        delete [] dir_adm;
        
        // Now build the filename
	if( access(rcfile,R_OK) == -1 ){
		if( errno != ENOENT )
			fprintf(stderr,"can't read(%s): %d (%s)\n",
				rcfile, errno, strerror(errno) );
                delete [] rcfile;
		return RC_FAILED;
	}
	if( !(xdoc = xmlParseFile(rcfile)) ){
		fprintf(stderr,"can't parse \"rcfile\".\n");
                delete [] rcfile;
		return RC_FAILED;
	}
        delete [] rcfile;
	erase_blank_nodes(xdoc->children);
	p = xdoc->children; // ->avidemux
	buf[0] = '\0';
	if( p )
		p = p->children; // ->avidemux->???
	while( p ){
		if( strlen(buf) ){
			strncpy(&buf[strlen(buf)],".",1024-strlen(buf));
			buf[1023] = '\0';
		}
		strncpy(&buf[strlen(buf)],(char*)p->name,1024-strlen(buf));
		buf[1023] = '\0';
		if( p->content ){
			set_content(buf, p);
		}else if( p->children ){
		   xmlNodePtr c = p->children;
			if( c->type == XML_TEXT_NODE && ! c->children && ! c->next && ! c->prev ){
				set_content(buf, c);
				// the routine below will go to c->parent->next (p->next) if we do:
				p = c;
				strncpy(&buf[strlen(buf)],".DUMMY",1024-strlen(buf));
				buf[1023] = '\0';
			}
		}
		if( p->children ){                               // go down first
			p = p->children;
		}else if( p->next ){                             // than go next
		   char *t = rindex(buf,'.');
			if( t )
				*t = '\0';
			else
				buf[0] = '\0';
			p = p->next;
		}else{                                           // and last go up AND next
			do{
				if( p->parent == xdoc->children ){
					p = NULL;
				}else{
				   char *t = rindex(buf,'.');
					if( t )
						*t = '\0';
					else
						buf[0] = '\0';
					p = p->parent;
				}
			}while( p && ! p->next );
			if( p ){
			   char *t = rindex(buf,'.');
				if( t )
					*t = '\0';
				else
					buf[0] = '\0';
				p = p->next;
			}
		}
	}
	// load xml to preferences
	//    check ranges foreach val
	//       set to min if  <min or to max if >max - generate warning
	//    warn about unused options
	printf("Preferences found and loaded\n");
	return RC_OK;
}


int preferences::save(){
   xmlNodePtr n;
   char buf[1024];
   char *p;
   char *q;
	if( xdoc )
		xmlFreeDoc(xdoc);
	if( ! (xdoc = xmlNewDoc((const xmlChar *)"1.0")) )
		return RC_FAILED;
	if( ! (xdoc->children = xmlNewNode(NULL,(xmlChar *)"avidemux")) ){
		xmlFreeDoc(xdoc); xdoc = NULL;
		return RC_FAILED;
	}
	for( int i=0; i < num_opts; i++ ){
	   xmlNodePtr nn;
	   const char *val = opt_defs[i].current_val;
		if( ! opt_defs[i].current_val )
			val = opt_defs[i].default_val;
		//	continue; // no value set - yet
		// if( ! strcmp(opt_defs[i].default_val,opt_defs[i].current_val) )
		//	continue; // current_val == default_val
		strncpy(buf,opt_defs[i].name,1024);
		buf[1023] = '\0';
		p = buf;
		n = xdoc->children;
		while( (q=index(p,'.')) ){
			*q = '\0'; q++;
			nn = goto_node_with_create(n, p);
			if( !nn )
				return RC_FAILED;
			p = q;
			n = nn;
		}
		nn = goto_node_with_create(n, p);
		if( !nn )
			return RC_FAILED;
		xmlNodeSetContent( nn, (xmlChar*)val );
	}
	return save_xml_to_file();
}

int preferences::save_xml_to_file(){
   char *dir_adm;
   char *rcfile;
   char *rcfilenew;

        dir_adm=ADM_getBaseDir();
        if(!dir_adm) return RC_FAILED;

        rcfile=new char[strlen(dir_adm)+4+strlen(CONFIG)];
        strcpy(rcfile,dir_adm);
        strcat(rcfile,"/");
        strcat(rcfile,CONFIG);


#if defined(__MINGW32__)
	xmlSetDocCompressMode(xdoc,9);
	if( xmlSaveFormatFile(rcfile,xdoc,1) == -1 ){
           fprintf(stderr,"\ncan't save xml tree to file. Filesystem full?\n\n");
           delete [] rcfile;
	   return RC_FAILED;
	}
        delete [] rcfile;
	return RC_OK;

#else
	
        
         rcfilenew=new char[strlen(rcfile)+5];
        strcpy(rcfilenew,rcfile);
        strcat(rcfilenew,".new");
        if( unlink(rcfilenew) == -1 && errno != ENOENT ){
           fprintf(stderr,"can't unlink(%s): %d (%s)\n",
                   rcfilenew, errno, strerror(errno));
           delete [] rcfilenew;
           return RC_FAILED;
        }
	xmlSetDocCompressMode(xdoc,9);
	if( qxmlSaveFormatFile(rcfilenew,xdoc,1) == -1 ){
           delete [] rcfilenew;
           delete [] rcfile;
	   return RC_FAILED;
	}

        if( unlink(rcfile) == -1 && errno != ENOENT ){
           fprintf(stderr,"can't unlink(%s): %d (%s)\n",
                   rcfile, errno, strerror(errno));
           delete [] rcfilenew;
           delete [] rcfile;
           return RC_FAILED;
        }
        if( link(rcfilenew,rcfile) == -1 ){
           fprintf(stderr,"can't create \"%s\": %d (%s)\n",
                   rcfile, errno, strerror(errno));
           delete [] rcfilenew;
           delete [] rcfile;
           return RC_FAILED;
        }
        unlink(rcfilenew); // rc/errno handling is done on next call ;-)
        delete [] rcfilenew;
        delete [] rcfile;
	return RC_OK;
#endif
}

/*
int preferences::get(options option, uint8_t *val){
   unsigned int x;
	if( get(option,&x) == RC_OK ){
		if( x <= 255 ){
			*val = (uint8_t)x;
			return RC_OK;
		}
	}
	return RC_FAILED;
}
*/
int preferences::get(options option, uint16_t *val){
   unsigned int x;
	if( get(option,&x) == RC_OK ){
		if( x <= 65535 ){
			*val = (uint16_t)x;
			return RC_OK;
		}
	}
	return RC_FAILED;
}

int preferences::get(options option, unsigned int *val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	if( opt_defs[option].type != UINT ){
		fprintf(stderr,"preferences::get(%s,uint) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	if( sscanf(p,"%u",val) == 1 )
		return RC_OK;
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option,          int *val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	if( opt_defs[option].type != INT ){
		fprintf(stderr,"preferences::get(%s,int) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	if( sscanf(p,"%d",val) == 1 )
		return RC_OK;
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, unsigned long *val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	if( opt_defs[option].type != ULONG ){
		fprintf(stderr,"preferences::get(%s,ulong) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	if( sscanf(p,"%lu",val) == 1 )
		return RC_OK;
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, long *val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	if( opt_defs[option].type != LONG ){
		fprintf(stderr,"preferences::get(%s,long) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	if( sscanf(p,"%ld",val) == 1 )
		return RC_OK;
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, float *val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	if( opt_defs[option].type != FLOAT ){
		fprintf(stderr,"preferences::get(%s,float) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	if( sscanf(p,"%f",val) == 1 )
		return RC_OK;
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, char **val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	// no type check : every value can be represented by a string
	// not an error -> it's a magic feature
	if( (*val = ADM_strdup(p) ) )
		return RC_OK;
	return RC_FAILED; // strdup() out of memory
}
#warning incorrect!
int preferences::get(options option, ADM_filename **val){
   const char *p = opt_defs[option].current_val;
	if( !p )
		p = opt_defs[option].default_val;
	// no type check : every value can be represented by a string
	// not an error -> it's a magic feature
	if( (*val = (ADM_filename *)ADM_strdup(p) ) )
		return RC_OK;
	return RC_FAILED; // strdup() out of memory
}
int preferences::set(options option, const unsigned int val){
   unsigned int l,r;
   char buf[1024];
   unsigned int v = val;
	// check type of option
	if( opt_defs[option].type != UINT ){
		fprintf(stderr,"preferences::set(%s,uint) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check range
	if( sscanf(opt_defs[option].minimum,"%u",&l) != 1){
		fprintf(stderr,"error reading opt_defs[option].minimum\n");
		return RC_FAILED;
	}
	if( sscanf(opt_defs[option].maximum,"%u",&r) != 1){
		fprintf(stderr,"error reading opt_defs[option].maximum\n");
		return RC_FAILED;
	}
	if( v < l ){
		fprintf(stderr,"%s : value < min : %u < %u\n", opt_defs[option].name, v, l);
		v = l;
		fprintf(stderr,"   using %u as value instead.\n", v);
	}
	if( v > r ){
		fprintf(stderr,"%s : value > max : %u > %u\n", opt_defs[option].name, v, r);
		v = r;
		fprintf(stderr,"   using %u as value instead.\n", v);
	}
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	snprintf(buf,1024,"%u",v);
	buf[1023] = '\0';
	opt_defs[option].current_val = ADM_strdup(buf);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

int preferences::set(options option, const int val){
   int l,r;
   char buf[1024];
   int v = val;
	// check type of option
	if( opt_defs[option].type != INT ){
		fprintf(stderr,"preferences::set(%s,int) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
	}
	// check range
	if( sscanf(opt_defs[option].minimum,"%d",&l) != 1){
		fprintf(stderr,"error reading opt_defs[option].minimum\n");
		return RC_FAILED;
	}
	if( sscanf(opt_defs[option].maximum,"%d",&r) != 1){
		fprintf(stderr,"error reading opt_defs[option].maximum\n");
		return RC_FAILED;
	}
	if( v < l ){
		fprintf(stderr,"%s : value < min : %d < %d\n", opt_defs[option].name, v, l);
		v = l;
		fprintf(stderr,"   using %d as value instead.\n", v);
	}
	if( v > r ){
		fprintf(stderr,"%s : value > max : %d > %d\n", opt_defs[option].name, v, r);
		v = r;
		fprintf(stderr,"   using %d as value instead.\n", v);
	}
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	snprintf(buf,1024,"%d",v);
	buf[1023] = '\0';
	opt_defs[option].current_val = ADM_strdup(buf);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

int preferences::set(options option, const unsigned long val){
   unsigned long l,r;
   char buf[1024];
   unsigned long v = val;
	// check type of option
	if( opt_defs[option].type != ULONG ){
		fprintf(stderr,"preferences::set(%s,ulong) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check range
	if( sscanf(opt_defs[option].minimum,"%lu",&l) != 1){
		fprintf(stderr,"error reading opt_defs[option].minimum\n");
		return RC_FAILED;
	}
	if( sscanf(opt_defs[option].maximum,"%lu",&r) != 1){
		fprintf(stderr,"error reading opt_defs[option].maximum\n");
		return RC_FAILED;
	}
	if( v < l ){
		fprintf(stderr,"%s : value < min : %lu < %lu\n", opt_defs[option].name, v, l);
		v = l;
		fprintf(stderr,"   using %lu as value instead.\n", v);
	}
	if( v > r ){
		fprintf(stderr,"%s : value > max : %lu > %lu\n", opt_defs[option].name, v, r);
		v = r;
		fprintf(stderr,"   using %lu as value instead.\n", v);
	}
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	snprintf(buf,1024,"%lu",v);
	buf[1023] = '\0';
	opt_defs[option].current_val = ADM_strdup(buf);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

int preferences::set(options option, const long val){
   long l,r;
   char buf[1024];
   long v = val;
	// check type of option
	if( opt_defs[option].type != LONG ){
		fprintf(stderr,"preferences::set(%s,long) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check range
	if( sscanf(opt_defs[option].minimum,"%ld",&l) != 1){
		fprintf(stderr,"error reading opt_defs[option].minimum\n");
		return RC_FAILED;
	}
	if( sscanf(opt_defs[option].maximum,"%ld",&r) != 1){
		fprintf(stderr,"error reading opt_defs[option].maximum\n");
		return RC_FAILED;
	}
	if( v < l ){
		fprintf(stderr,"%s : value < min : %ld < %ld\n", opt_defs[option].name, v, l);
		v = l;
		fprintf(stderr,"   using %ld as value instead.\n", v);
	}
	if( v > r ){
		fprintf(stderr,"%s : value > max : %ld > %ld\n", opt_defs[option].name, v, r);
		v = r;
		fprintf(stderr,"   using %ld as value instead.\n", v);
	}
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	snprintf(buf,1024,"%ld",v);
	buf[1023] = '\0';
	opt_defs[option].current_val = ADM_strdup(buf);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

int preferences::set(options option, const float val){
   float l,r;
   char buf[1024];
   float v = val;
	// check type of option
	if( opt_defs[option].type != FLOAT ){
		fprintf(stderr,"preferences::set(%s,float) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check range
	if( sscanf(opt_defs[option].minimum,"%f",&l) != 1){
		fprintf(stderr,"error reading opt_defs[option].minimum\n");
		return RC_FAILED;
	}
	if( sscanf(opt_defs[option].maximum,"%f",&r) != 1){
		fprintf(stderr,"error reading opt_defs[option].maximum\n");
		return RC_FAILED;
	}
	if( v < l ){
		fprintf(stderr,"%s : value < min : %f < %f\n", opt_defs[option].name, v, l);
		v = l;
		fprintf(stderr,"   using %f as value instead.\n", v);
	}
	if( v > r ){
		fprintf(stderr,"%s : value > max : %f > %f\n", opt_defs[option].name, v, r);
		v = r;
		fprintf(stderr,"   using %f as value instead.\n", v);
	}
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	snprintf(buf,1024,"%f",v);
	buf[1023] = '\0';
	opt_defs[option].current_val = ADM_strdup(buf);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

int preferences::set(options option, const char * val){
	// check type of option
	if( opt_defs[option].type != STRING ){
		fprintf(stderr,"preferences::set(%s,string) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check val
	if( ! val )
		return RC_FAILED;
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	opt_defs[option].current_val = ADM_strdup(val);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}
#warning incorrect!
int preferences::set(options option, const ADM_filename * val){
	// check type of option
	if( opt_defs[option].type != FILENAME ){
		fprintf(stderr,"preferences::set(%s,string) called for type %d\n",
			opt_defs[option].name,opt_defs[option].type);
		ADM_assert(0);
	}
	// check val
	if( ! val )
		return RC_FAILED;
	// set value
	if( opt_defs[option].current_val )
		ADM_dealloc(opt_defs[option].current_val);
	opt_defs[option].current_val = ADM_strdup((char *)val);
	if( ! opt_defs[option].current_val )
		return RC_FAILED;
	return RC_OK;
}

const char * preferences::get_str_min(options option){
	return( opt_defs[option].minimum );
}

const char * preferences::get_str_max(options option){
	return( opt_defs[option].maximum );
}

#define PRT_LAFI(x,y,z) fprintf(stderr,"Prefs: %s%u %s\n",x,y,(z?z:"NULL"))

int preferences::set_lastfile(const char* file){
   char *internal_file;
	if( ! file ){
		fprintf(stderr,"Prefs: set_lastfile(NULL) called\n");
		return RC_FAILED;
	}
	internal_file = ADM_PathCanonize(file);
	if( !internal_file ){
		fprintf(stderr,"Prefs: set_lastfile(): PathCanonize() returns NULL\n");
		return RC_FAILED;
        }
#ifdef DEBUG_PREFS
	fprintf(stderr,"Prefs: set_lastfile(%s)\n",file);
	if( strcmp(file,internal_file) )
		fprintf(stderr,"Prefs: set_lastfile(%s) (with appended current dir)\n",internal_file);
	PRT_LAFI("<= LASTFILES_",1,opt_defs[LASTFILES_FILE1].current_val);
	PRT_LAFI("<= LASTFILES_",2,opt_defs[LASTFILES_FILE2].current_val);
	PRT_LAFI("<= LASTFILES_",3,opt_defs[LASTFILES_FILE3].current_val);
	PRT_LAFI("<= LASTFILES_",4,opt_defs[LASTFILES_FILE4].current_val);
#endif
	// change opt_defs array
	//
	// ToDo:
	// * a call with a file already in lastfiles will resort lastfiles with
	//   the actual argument on top
	// * a call with a file new to lastfiles will drop LASTFILE_4, move all
	//   one step down and add the file as LASTFILE_1
	if( opt_defs[LASTFILES_FILE4].current_val &&
	    !strncmp(opt_defs[LASTFILES_FILE4].current_val,internal_file,strlen(opt_defs[LASTFILES_FILE4].current_val)) ){
	  char *x = opt_defs[LASTFILES_FILE4].current_val;
		opt_defs[LASTFILES_FILE4].current_val = opt_defs[LASTFILES_FILE3].current_val;
		opt_defs[LASTFILES_FILE3].current_val = opt_defs[LASTFILES_FILE2].current_val;
		opt_defs[LASTFILES_FILE2].current_val = opt_defs[LASTFILES_FILE1].current_val;
		opt_defs[LASTFILES_FILE1].current_val = x;
	}else if( opt_defs[LASTFILES_FILE3].current_val &&
            !strncmp(opt_defs[LASTFILES_FILE3].current_val,internal_file,strlen(opt_defs[LASTFILES_FILE3].current_val)) ){
          char *x = opt_defs[LASTFILES_FILE3].current_val;
		opt_defs[LASTFILES_FILE3].current_val = opt_defs[LASTFILES_FILE2].current_val;
                opt_defs[LASTFILES_FILE2].current_val = opt_defs[LASTFILES_FILE1].current_val;
                opt_defs[LASTFILES_FILE1].current_val = x;
        }else if( opt_defs[LASTFILES_FILE2].current_val &&
            !strncmp(opt_defs[LASTFILES_FILE2].current_val,internal_file,strlen(opt_defs[LASTFILES_FILE2].current_val)) ){
          char *x = opt_defs[LASTFILES_FILE2].current_val;
		opt_defs[LASTFILES_FILE2].current_val = opt_defs[LASTFILES_FILE1].current_val;
		opt_defs[LASTFILES_FILE1].current_val = x;
	}else if( opt_defs[LASTFILES_FILE1].current_val &&
            !strncmp(opt_defs[LASTFILES_FILE1].current_val,internal_file,strlen(opt_defs[LASTFILES_FILE1].current_val)) ){
		; // nothing to do - always on top
	}else{
		if( opt_defs[LASTFILES_FILE4].current_val )
			ADM_dealloc(opt_defs[LASTFILES_FILE4].current_val);
		opt_defs[LASTFILES_FILE4].current_val = opt_defs[LASTFILES_FILE3].current_val;
		opt_defs[LASTFILES_FILE3].current_val = opt_defs[LASTFILES_FILE2].current_val;
		opt_defs[LASTFILES_FILE2].current_val = opt_defs[LASTFILES_FILE1].current_val;
		opt_defs[LASTFILES_FILE1].current_val = ADM_strdup(internal_file);
	}


	// change the xmlDocument
	if( ! xdoc ){
		// no .avidemuxrc file or not loaded yet
		load();          // try to load it
		if( ! xdoc ){    // really: no .avidemuxrc file
			save();  // generate one from internal defaults and actual changes
			if( xdoc )
				erase_blank_nodes(xdoc->children);
		}
	}
	if( ! xdoc ){
		fprintf(stderr,"Prefs: no xml document generated ny load() nor save()\n");
	}else{
	  xmlNodePtr p;
	  xmlNodePtr q;
		// we assume a valid xml document, but maybe an older version
		ADM_assert( xdoc->children );
		p = xdoc->children;				// ->avidemux (should be there)
		p = goto_node_with_create(p, "lastfiles");	// ->avidemux->lastfile
		q = goto_node_with_create(p, "file1");		// ->avidemux->lastfile->1
		xmlNodeSetContent( q,
			(xmlChar*)(opt_defs[LASTFILES_FILE1].current_val?opt_defs[LASTFILES_FILE1].current_val:""));
		q = goto_node_with_create(p, "file2");		// ->avidemux->lastfile->2
		xmlNodeSetContent( q,
			(xmlChar*)(opt_defs[LASTFILES_FILE2].current_val?opt_defs[LASTFILES_FILE2].current_val:""));
		q = goto_node_with_create(p, "file3");		// ->avidemux->lastfile->3
		xmlNodeSetContent( q,
			(xmlChar*)(opt_defs[LASTFILES_FILE3].current_val?opt_defs[LASTFILES_FILE3].current_val:""));
		q = goto_node_with_create(p, "file4");		// ->avidemux->lastfile->4
		xmlNodeSetContent( q,
			(xmlChar*)(opt_defs[LASTFILES_FILE4].current_val?opt_defs[LASTFILES_FILE4].current_val:""));
		save_xml_to_file();
	}


#ifdef DEBUG_PREFS
	PRT_LAFI("=> LASTFILES_",1,opt_defs[LASTFILES_FILE1].current_val);
	PRT_LAFI("=> LASTFILES_",2,opt_defs[LASTFILES_FILE2].current_val);
	PRT_LAFI("=> LASTFILES_",3,opt_defs[LASTFILES_FILE3].current_val);
	PRT_LAFI("=> LASTFILES_",4,opt_defs[LASTFILES_FILE4].current_val);
#endif
	delete[] internal_file;
	return RC_OK;
}

#undef PRT_LAFI
#define PRT_LAFI(y,z) fprintf(stderr,"Prefs: ret idx[%u] %s\n",y,(z?z:"NULL"))

const char **preferences::get_lastfiles(void){
  unsigned int idx;
#ifdef DEBUG_PREFS
	fprintf(stderr,"Prefs: get_lastfile()\n");
#endif
	for( idx=0; idx < 4; idx++ ){
		if( internal_lastfiles[idx] ){
			ADM_dealloc(internal_lastfiles[idx]);
			internal_lastfiles[idx] = NULL;
		}
	}
	if( opt_defs[LASTFILES_FILE1].current_val )
		internal_lastfiles[0] = ADM_strdup(opt_defs[LASTFILES_FILE1].current_val);
	if( opt_defs[LASTFILES_FILE2].current_val )
		internal_lastfiles[1] = ADM_strdup(opt_defs[LASTFILES_FILE2].current_val);
	if( opt_defs[LASTFILES_FILE3].current_val )
		internal_lastfiles[2] = ADM_strdup(opt_defs[LASTFILES_FILE3].current_val);
	if( opt_defs[LASTFILES_FILE4].current_val )
		internal_lastfiles[3] = ADM_strdup(opt_defs[LASTFILES_FILE4].current_val);
	internal_lastfiles[4] = NULL;

#ifdef DEBUG_PREFS
	PRT_LAFI(0,internal_lastfiles[0]);
	PRT_LAFI(1,internal_lastfiles[1]);
	PRT_LAFI(2,internal_lastfiles[2]);
	PRT_LAFI(3,internal_lastfiles[3]);
	PRT_LAFI(4,internal_lastfiles[4]);
#endif
	return (const char**)internal_lastfiles;
}
int initPrefs(  void )
{
  prefs = new preferences();
  return 1;
}

int destroyPrefs(void)
{
	for (int i = 0; i < num_opts; i++)
	{
		if (opt_defs[i].current_val)
		{
			ADM_dealloc(opt_defs[i].current_val);
			opt_defs[i].current_val=NULL;
		}
	}

	delete prefs;
	prefs = NULL;
	return 1;
}

// EOF
