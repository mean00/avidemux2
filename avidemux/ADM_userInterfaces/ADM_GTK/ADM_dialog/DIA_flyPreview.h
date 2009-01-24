#ifndef FLY_PREVIEW_H
#define FLY_PREVIEW_H
class flyPreview : public ADM_flyDialogGtk
{
public:
	uint8_t process(void) {return 1;}
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
	uint8_t setData(uint8_t *buffer) {_rgb->scale(buffer, _rgbBufferOut); return 1;}

	flyPreview(uint32_t width, uint32_t height, void *canvas) : 
	  ADM_flyDialogGtk(width, height, NULL, canvas, NULL, 0, RESIZE_NONE) {delete[] _rgbBuffer; _rgbBuffer = NULL;};
	virtual ~flyPreview(void) {_rgbBuffer = NULL;};
};

class flySeekablePreview : public ADM_flyDialogGtk
{
public:
	uint8_t process(void) {_rgbBufferOut = _rgbBuffer; return 1;}
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}

	flySeekablePreview(uint32_t width, uint32_t height, AVDMGenericVideoStream *videoStream, void *canvas, void *slider) : 
	  ADM_flyDialogGtk(width, height, videoStream, canvas, slider, 0, RESIZE_AUTO) {delete[] _rgbBufferOut; _rgbBufferOut = NULL;};
	virtual ~flySeekablePreview(void) {_rgbBufferOut = NULL;};
};
#endif
