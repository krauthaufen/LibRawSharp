#define strnlen_s(data,cap) strnlen(data,cap)

#ifdef __APPLE__
#include "LibRaw.h"
#define DllExport(t) extern "C" t
#elif __GNUC__
#include <libraw.h>
#define DllExport(t) extern "C" t
#else
#include "pch.h"
#include "LibRaw.h"
#endif

//
//bool getSensorSize(int typ, float& w, float& h)
//{
//	switch (typ) {
//	case LIBRAW_FORMAT_APSC: w = 
//	case LIBRAW_FORMAT_FF: return 1.123;
//	case LIBRAW_FORMAT_MF: return 1.123;
//	case LIBRAW_FORMAT_APSH: return 1.123;
//	case LIBRAW_FORMAT_1INCH: return 1.123;
//	case LIBRAW_FORMAT_FT: return 1.123;
//	}
//
//}



//! Byte swap unsigned short
static uint16_t swap_uint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

//! Byte swap short
static int16_t swap_int16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
static uint32_t swap_uint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

//! Byte swap int
static int32_t swap_int32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF);
}


#define FORCC for (c = 0; c < colors && c < 4; c++)
#define SQR(x) ((x) * (x))

static char* allocString(const char* const data, size_t cap)
{
	auto len = strnlen_s(data, cap);
	char* res = new char[len+1];
	memcpy(res, data, len + 1);
	return res;
}

DllExport(void) lrFree(RawImage image)
{
	delete image.Make;
	delete image.Model;
	delete image.LensMake;
	delete image.LensModel;
	delete image.Data;
}

static unsigned sget4(ushort order, uchar* s)
{
	if (order == 0x4949)
		return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
	else
		return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}


typedef struct {
	ushort Order;
	float* ExposureBias;
} CallbackData;

static void callback(void* context, int tag, int type,
	int len, unsigned int ord, void* ifp,
	INT64 base)
{
	auto fp = (LibRaw_file_datastream*)ifp;
	uchar data[128];
	float value = 0.0;
	uint32_t a, b;

	auto ctx = (CallbackData*)context;

	if (tag == 0x9204) {
		switch (type) {
		case LIBRAW_EXIFTAG_TYPE_RATIONAL:
			fp->read((void*)data, 8, 1);
			a = (uint32_t)sget4(ctx->Order, data); 
			b = (uint32_t)sget4(ctx->Order, data + 4);
			value = (float)a / (float)b;
			break;
		case LIBRAW_EXIFTAG_TYPE_SRATIONAL:
			fp->read((void*)data, 8, 1);
			a = sget4(ctx->Order, data);
			b = sget4(ctx->Order, data + 4);
			value = (float)a / (float)b;
			break;
		case LIBRAW_EXIFTAG_TYPE_DOUBLE:
			fp->read((void*)data, 8, 1);
			value = (float)((double*)data)[0];
			break;
		case LIBRAW_EXIFTAG_TYPE_FLOAT:
			fp->read((void*)data, 4, 1);
			value = ((float*)data)[0];
			break;
		}
		//printf("got: %f\n", value);
		*ctx->ExposureBias = value;
	}

}

DllExport(RawImage) lrLoad(const char* filename)
{
	LibRaw* proc = new LibRaw();
	auto byteOrder = proc->get_internal_data_pointer()->unpacker_data.order;
	
	float exposure = 0.0;
	CallbackData cbd;
	cbd.Order = byteOrder;
	cbd.ExposureBias = &exposure;
	
	proc->set_exifparser_handler(callback, (void*)&cbd);
	proc->open_file(filename);
	proc->unpack();
	auto corr = (float)pow(2.0, (double)exposure);
	proc->imgdata.params.use_camera_wb = 1;
	//proc->imgdata.params.no_auto_bright = 0;
	//proc->imgdata.params.highlight = 1;
	//proc->imgdata.params.gamm[0] = 0.45;
	//proc->imgdata.params.gamm[1] = 4.5;

	proc->imgdata.params.exp_correc = 1;
	proc->imgdata.params.exp_shift = corr;
	proc->imgdata.params.exp_preser = 0.5;
	//proc->imgdata.params.user_qual = 12;
	proc->dcraw_process();

	size_t pixels = (size_t)proc->imgdata.sizes.width * (size_t)proc->imgdata.sizes.height;
	char* data = new char[pixels * (size_t)3];
	proc->copy_mem_image(data, proc->imgdata.sizes.width * 3, 0);
	proc->free_image();

	RawImage result;
	result.Make = allocString(proc->imgdata.idata.make, 64);
	result.Model = allocString(proc->imgdata.idata.model, 64);
	result.LensMake = allocString(proc->imgdata.lens.LensMake, 128);
	result.LensModel = allocString(proc->imgdata.lens.makernotes.Lens, 128);
	result.FocalLength = proc->imgdata.lens.makernotes.CurFocal;
	result.Aperture = proc->imgdata.lens.makernotes.CurAp;
	result.Width = proc->imgdata.sizes.width;
	result.Height = proc->imgdata.sizes.height;
	result.Data = data;

	proc->recycle();
	delete proc;
	return result;
}





