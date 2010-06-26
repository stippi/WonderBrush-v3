#include "StackBlurFilter.h"

#include <stdio.h>

#include <Bitmap.h>

#include "agg_pod_vector.h"

#include "RenderBuffer.h"

template<class T> struct stack_blur_tables
{
	static uint16 const g_stack_blur8_mul[255];
	static uint8  const g_stack_blur8_shr[255];
};

template<class T> 
uint16 const stack_blur_tables<T>::g_stack_blur8_mul[255] = 
{
	512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
	454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
	482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
	437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
	497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
	320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
	446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
	329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
	505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
	399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
	324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
	268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
	451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
	385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
	332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
	289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

template<class T> 
uint8 const stack_blur_tables<T>::g_stack_blur8_shr[255] = 
{
	  9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
	 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
	 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
	 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
	 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
	 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
	 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
	 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
	 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};


#if __GNUC__ < 4
static inline float
roundf(float v)
{
	return floorf(v + 0.5);
}
#endif


StackBlurFilter::StackBlurFilter()
{
}


StackBlurFilter::~StackBlurFilter()
{
}


void
StackBlurFilter::FilterRGBA64(RenderBuffer* buffer, double radius)
{
	if (radius < 1.0) {
		printf("StackBlurFilter::Filter() - radius too small (< 1.0)\n");
		return;
	}

	int32 width = buffer->Width();
	int32 height = buffer->Height();
	uint32 bpr = buffer->BytesPerRow();
	uint16* bits = reinterpret_cast<uint16*>(buffer->Bits());

	unsigned r = (unsigned)roundf(radius);

	_Filter64(bits, width, height, bpr, r, r);
}


void
StackBlurFilter::FilterRGBA32(RenderBuffer* buffer, double radius)
{
	if (radius < 1.0) {
		printf("StackBlurFilter::Filter() - radius too small (< 1.0)\n");
		return;
	}

	int32 width = buffer->Width();
	int32 height = buffer->Height();
	uint32 bpr = buffer->BytesPerRow();
	uint8* bits = buffer->Bits();

	unsigned r = (unsigned)roundf(radius);

	_Filter32(bits, width, height, bpr, r, r);
}


void
StackBlurFilter::FilterGray8(RenderBuffer* buffer, double radius)
{
	if (radius < 1.0) {
		printf("StackBlurFilter::Filter() - radius too small (< 1.0)\n");
		return;
	}

	int32 width = buffer->Width();
	int32 height = buffer->Height();
	uint32 bpr = buffer->BytesPerRow();
	uint8* bits = buffer->Bits();

	unsigned r = (unsigned)roundf(radius);

	_Filter8(bits, width, height, bpr, r, r);
}


void
StackBlurFilter::Filter(BBitmap* bitmap, double radius)
{
	if (radius < 1.0) {
		printf("StackBlurFilter::Filter() - radius too small (< 1.0)\n");
		return;
	}

	int32 width = bitmap->Bounds().IntegerWidth() + 1;
	int32 height = bitmap->Bounds().IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();
	uint8* bits = (uint8*)bitmap->Bits();

	unsigned r = (unsigned)roundf(radius);

	if (bitmap->ColorSpace() == B_RGBA32
		|| bitmap->ColorSpace() == B_RGB32) {

		_Filter32(bits, width, height, bpr, r, r);

	} else if (bitmap->ColorSpace() == B_GRAY8) {

		_Filter8(bits, width, height, bpr, r, r);

	} else {
		printf("StackBlurFilter::Filter() - unsupported color space\n");
	}
}


// #pragma mark -

struct rgba16 {
	uint16	r;
	uint16	g;
	uint16	b;
	uint16	a;
};


void
StackBlurFilter::_Filter64(uint16* buffer,
						   unsigned width, unsigned height,
						   int32 bpr,
						   unsigned rx, unsigned ry) const
{
	typedef rgba16 color_type;

	unsigned x, y, xp, yp, i;
	unsigned stack_ptr;
	unsigned stack_start;

	bpr /= 2;

	const uint16* src_pix_ptr;
		  uint16* dst_pix_ptr;
	color_type* stack_pix_ptr;

	unsigned sum_r;
	unsigned sum_g;
	unsigned sum_b;
	unsigned sum_a;
	unsigned sum_in_r;
	unsigned sum_in_g;
	unsigned sum_in_b;
	unsigned sum_in_a;
	unsigned sum_out_r;
	unsigned sum_out_g;
	unsigned sum_out_b;
	unsigned sum_out_a;

	unsigned w   = width;
	unsigned h   = height;
	unsigned wm  = w - 1;
	unsigned hm  = h - 1;

	unsigned div;
	unsigned mul_sum;
	unsigned shr_sum;

	pod_vector<color_type> stack;

	if (rx > 0) {
		if (rx > 254)
			rx = 254;
		div = rx * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[rx];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[rx];
		stack.allocate(div);

		for (y = 0; y < h; y++) {
			sum_r = 
			sum_g = 
			sum_b = 
			sum_a = 
			sum_in_r = 
			sum_in_g = 
			sum_in_b = 
			sum_in_a = 
			sum_out_r = 
			sum_out_g = 
			sum_out_b = 
			sum_out_a = 0;

			src_pix_ptr = buffer + bpr * y;
			for (i = 0; i <= rx; i++) {
				stack_pix_ptr	= &stack[i];
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
				sum_r		   += stack_pix_ptr->r * (i + 1);
				sum_g		   += stack_pix_ptr->g * (i + 1);
				sum_b		   += stack_pix_ptr->b * (i + 1);
				sum_a		   += stack_pix_ptr->a * (i + 1);
				sum_out_r	   += stack_pix_ptr->r;
				sum_out_g	   += stack_pix_ptr->g;
				sum_out_b	   += stack_pix_ptr->b;
				sum_out_a	   += stack_pix_ptr->a;
			}
			for (i = 1; i <= rx; i++) {
				if (i <= wm)
					src_pix_ptr += 4;
				stack_pix_ptr = &stack[i + rx];
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
				sum_r		   += stack_pix_ptr->r * (rx + 1 - i);
				sum_g		   += stack_pix_ptr->g * (rx + 1 - i);
				sum_b		   += stack_pix_ptr->b * (rx + 1 - i);
				sum_a		   += stack_pix_ptr->a * (rx + 1 - i);
				sum_in_r		+= stack_pix_ptr->r;
				sum_in_g		+= stack_pix_ptr->g;
				sum_in_b		+= stack_pix_ptr->b;
				sum_in_a		+= stack_pix_ptr->a;
			}

			stack_ptr = rx;
			xp = rx;
			if (xp > wm)
				xp = wm;
			src_pix_ptr = buffer + xp * 4 + y * bpr;
			dst_pix_ptr = buffer + y * bpr;
			for (x = 0; x < w; x++) {
				dst_pix_ptr[0] = (sum_b * mul_sum) >> shr_sum;
				dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_pix_ptr[2] = (sum_r * mul_sum) >> shr_sum;
				dst_pix_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_pix_ptr[0] = (dst_pix_ptr[0] << 8) | dst_pix_ptr[0];
				dst_pix_ptr[1] = (dst_pix_ptr[1] << 8) | dst_pix_ptr[1];
				dst_pix_ptr[2] = (dst_pix_ptr[2] << 8) | dst_pix_ptr[2];
				dst_pix_ptr[3] = (dst_pix_ptr[3] << 8) | dst_pix_ptr[3];
				dst_pix_ptr += 4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;
   
				stack_start = stack_ptr + div - rx;
				if(stack_start >= div) stack_start -= div;
				stack_pix_ptr = &stack[stack_start];

				sum_out_r -= stack_pix_ptr->r;
				sum_out_g -= stack_pix_ptr->g;
				sum_out_b -= stack_pix_ptr->b;
				sum_out_a -= stack_pix_ptr->a;

				if (xp < wm) {
					src_pix_ptr += 4;
					++xp;
				}
	
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
	
				sum_in_r += stack_pix_ptr->r;
				sum_in_g += stack_pix_ptr->g;
				sum_in_b += stack_pix_ptr->b;
				sum_in_a += stack_pix_ptr->a;
				sum_r	+= sum_in_r;
				sum_g	+= sum_in_g;
				sum_b	+= sum_in_b;
				sum_a	+= sum_in_a;
	
				++stack_ptr;
				if (stack_ptr >= div)
					stack_ptr = 0;
				stack_pix_ptr = &stack[stack_ptr];

				sum_out_r += stack_pix_ptr->r;
				sum_out_g += stack_pix_ptr->g;
				sum_out_b += stack_pix_ptr->b;
				sum_out_a += stack_pix_ptr->a;
				sum_in_r  -= stack_pix_ptr->r;
				sum_in_g  -= stack_pix_ptr->g;
				sum_in_b  -= stack_pix_ptr->b;
				sum_in_a  -= stack_pix_ptr->a;
			}
		}
	}

	if (ry > 0) {
		if (ry > 254)
			ry = 254;
		div = ry * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[ry];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[ry];
		stack.allocate(div);

		int stride = bpr;
		for(x = 0; x < w; x++) {
			sum_r = 
			sum_g = 
			sum_b = 
			sum_a = 
			sum_in_r = 
			sum_in_g = 
			sum_in_b = 
			sum_in_a = 
			sum_out_r = 
			sum_out_g = 
			sum_out_b = 
			sum_out_a = 0;

			src_pix_ptr = buffer + x * 4;
			for (i = 0; i <= ry; i++) {
				stack_pix_ptr	= &stack[i];
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
				sum_r		   += stack_pix_ptr->r * (i + 1);
				sum_g		   += stack_pix_ptr->g * (i + 1);
				sum_b		   += stack_pix_ptr->b * (i + 1);
				sum_a		   += stack_pix_ptr->a * (i + 1);
				sum_out_r	   += stack_pix_ptr->r;
				sum_out_g	   += stack_pix_ptr->g;
				sum_out_b	   += stack_pix_ptr->b;
				sum_out_a	   += stack_pix_ptr->a;
			}
			for (i = 1; i <= ry; i++) {
				if (i <= hm)
					src_pix_ptr += stride; 
				stack_pix_ptr = &stack[i + ry];
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
				sum_r		   += stack_pix_ptr->r * (ry + 1 - i);
				sum_g		   += stack_pix_ptr->g * (ry + 1 - i);
				sum_b		   += stack_pix_ptr->b * (ry + 1 - i);
				sum_a		   += stack_pix_ptr->a * (ry + 1 - i);
				sum_in_r		+= stack_pix_ptr->r;
				sum_in_g		+= stack_pix_ptr->g;
				sum_in_b		+= stack_pix_ptr->b;
				sum_in_a		+= stack_pix_ptr->a;
			}

			stack_ptr = ry;
			yp = ry;
			if (yp > hm)
				yp = hm;
			src_pix_ptr = buffer + x * 4 + yp * bpr;
			dst_pix_ptr = buffer + x * 4;
			for (y = 0; y < h; y++) {
				dst_pix_ptr[0] = (sum_b * mul_sum) >> shr_sum;
				dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_pix_ptr[2] = (sum_r * mul_sum) >> shr_sum;
				dst_pix_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_pix_ptr[0] = (dst_pix_ptr[0] << 8) | dst_pix_ptr[0];
				dst_pix_ptr[1] = (dst_pix_ptr[1] << 8) | dst_pix_ptr[1];
				dst_pix_ptr[2] = (dst_pix_ptr[2] << 8) | dst_pix_ptr[2];
				dst_pix_ptr[3] = (dst_pix_ptr[3] << 8) | dst_pix_ptr[3];
				dst_pix_ptr += stride;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;
   
				stack_start = stack_ptr + div - ry;
				if (stack_start >= div)
					stack_start -= div;

				stack_pix_ptr = &stack[stack_start];
				sum_out_r -= stack_pix_ptr->r;
				sum_out_g -= stack_pix_ptr->g;
				sum_out_b -= stack_pix_ptr->b;
				sum_out_a -= stack_pix_ptr->a;

				if (yp < hm) {
					src_pix_ptr += stride;
					++yp;
				}
	
				stack_pix_ptr->r = src_pix_ptr[2] >> 8;
				stack_pix_ptr->g = src_pix_ptr[1] >> 8;
				stack_pix_ptr->b = src_pix_ptr[0] >> 8;
				stack_pix_ptr->a = src_pix_ptr[3] >> 8;
	
				sum_in_r += stack_pix_ptr->r;
				sum_in_g += stack_pix_ptr->g;
				sum_in_b += stack_pix_ptr->b;
				sum_in_a += stack_pix_ptr->a;
				sum_r	+= sum_in_r;
				sum_g	+= sum_in_g;
				sum_b	+= sum_in_b;
				sum_a	+= sum_in_a;
	
				++stack_ptr;
				if (stack_ptr >= div)
					stack_ptr = 0;
				stack_pix_ptr = &stack[stack_ptr];

				sum_out_r += stack_pix_ptr->r;
				sum_out_g += stack_pix_ptr->g;
				sum_out_b += stack_pix_ptr->b;
				sum_out_a += stack_pix_ptr->a;
				sum_in_r  -= stack_pix_ptr->r;
				sum_in_g  -= stack_pix_ptr->g;
				sum_in_b  -= stack_pix_ptr->b;
				sum_in_a  -= stack_pix_ptr->a;
			}
		}
	}
}

struct rgba {
	uint8	r;
	uint8	g;
	uint8	b;
	uint8	a;
};

void
StackBlurFilter::_Filter32(uint8* buffer,
						   unsigned width, unsigned height,
						   int32 bpr,
						   unsigned rx, unsigned ry) const
{
	typedef rgba color_type;

	unsigned x, y, xp, yp, i;
	unsigned stack_ptr;
	unsigned stack_start;

	const uint8* src_pix_ptr;
		  uint8* dst_pix_ptr;
	color_type*  stack_pix_ptr;

	unsigned sum_r;
	unsigned sum_g;
	unsigned sum_b;
	unsigned sum_a;
	unsigned sum_in_r;
	unsigned sum_in_g;
	unsigned sum_in_b;
	unsigned sum_in_a;
	unsigned sum_out_r;
	unsigned sum_out_g;
	unsigned sum_out_b;
	unsigned sum_out_a;

	unsigned w   = width;
	unsigned h   = height;
	unsigned wm  = w - 1;
	unsigned hm  = h - 1;

	unsigned div;
	unsigned mul_sum;
	unsigned shr_sum;

	pod_vector<color_type> stack;

	if (rx > 0) {
		if (rx > 254)
			rx = 254;
		div = rx * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[rx];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[rx];
		stack.allocate(div);

		for (y = 0; y < h; y++) {
			sum_r = 
			sum_g = 
			sum_b = 
			sum_a = 
			sum_in_r = 
			sum_in_g = 
			sum_in_b = 
			sum_in_a = 
			sum_out_r = 
			sum_out_g = 
			sum_out_b = 
			sum_out_a = 0;

			src_pix_ptr = buffer + bpr * y;
			for (i = 0; i <= rx; i++) {
				stack_pix_ptr	= &stack[i];
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
				sum_r		   += src_pix_ptr[2] * (i + 1);
				sum_g		   += src_pix_ptr[1] * (i + 1);
				sum_b		   += src_pix_ptr[0] * (i + 1);
				sum_a		   += src_pix_ptr[3] * (i + 1);
				sum_out_r	   += src_pix_ptr[2];
				sum_out_g	   += src_pix_ptr[1];
				sum_out_b	   += src_pix_ptr[0];
				sum_out_a	   += src_pix_ptr[3];
			}
			for (i = 1; i <= rx; i++) {
				if (i <= wm)
					src_pix_ptr += 4;
				stack_pix_ptr = &stack[i + rx];
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
				sum_r		   += src_pix_ptr[2] * (rx + 1 - i);
				sum_g		   += src_pix_ptr[1] * (rx + 1 - i);
				sum_b		   += src_pix_ptr[0] * (rx + 1 - i);
				sum_a		   += src_pix_ptr[3] * (rx + 1 - i);
				sum_in_r		+= src_pix_ptr[2];
				sum_in_g		+= src_pix_ptr[1];
				sum_in_b		+= src_pix_ptr[0];
				sum_in_a		+= src_pix_ptr[3];
			}

			stack_ptr = rx;
			xp = rx;
			if (xp > wm)
				xp = wm;
			src_pix_ptr = buffer + xp * 4 + y * bpr;
			dst_pix_ptr = buffer + y * bpr;
			for (x = 0; x < w; x++) {
				dst_pix_ptr[2] = (sum_r * mul_sum) >> shr_sum;
				dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_pix_ptr[0] = (sum_b * mul_sum) >> shr_sum;
				dst_pix_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_pix_ptr += 4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;
   
				stack_start = stack_ptr + div - rx;
				if(stack_start >= div) stack_start -= div;
				stack_pix_ptr = &stack[stack_start];

				sum_out_r -= stack_pix_ptr->r;
				sum_out_g -= stack_pix_ptr->g;
				sum_out_b -= stack_pix_ptr->b;
				sum_out_a -= stack_pix_ptr->a;

				if (xp < wm) {
					src_pix_ptr += 4;
					++xp;
				}
	
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
	
				sum_in_r += src_pix_ptr[2];
				sum_in_g += src_pix_ptr[1];
				sum_in_b += src_pix_ptr[0];
				sum_in_a += src_pix_ptr[3];
				sum_r	+= sum_in_r;
				sum_g	+= sum_in_g;
				sum_b	+= sum_in_b;
				sum_a	+= sum_in_a;
	
				++stack_ptr;
				if (stack_ptr >= div)
					stack_ptr = 0;
				stack_pix_ptr = &stack[stack_ptr];

				sum_out_r += stack_pix_ptr->r;
				sum_out_g += stack_pix_ptr->g;
				sum_out_b += stack_pix_ptr->b;
				sum_out_a += stack_pix_ptr->a;
				sum_in_r  -= stack_pix_ptr->r;
				sum_in_g  -= stack_pix_ptr->g;
				sum_in_b  -= stack_pix_ptr->b;
				sum_in_a  -= stack_pix_ptr->a;
			}
		}
	}

	if (ry > 0) {
		if (ry > 254)
			ry = 254;
		div = ry * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[ry];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[ry];
		stack.allocate(div);

		int stride = bpr;
		for(x = 0; x < w; x++) {
			sum_r = 
			sum_g = 
			sum_b = 
			sum_a = 
			sum_in_r = 
			sum_in_g = 
			sum_in_b = 
			sum_in_a = 
			sum_out_r = 
			sum_out_g = 
			sum_out_b = 
			sum_out_a = 0;

			src_pix_ptr = buffer + x * 4;
			for (i = 0; i <= ry; i++) {
				stack_pix_ptr	= &stack[i];
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
				sum_r		   += src_pix_ptr[2] * (i + 1);
				sum_g		   += src_pix_ptr[1] * (i + 1);
				sum_b		   += src_pix_ptr[0] * (i + 1);
				sum_a		   += src_pix_ptr[3] * (i + 1);
				sum_out_r	   += src_pix_ptr[2];
				sum_out_g	   += src_pix_ptr[1];
				sum_out_b	   += src_pix_ptr[0];
				sum_out_a	   += src_pix_ptr[3];
			}
			for (i = 1; i <= ry; i++) {
				if (i <= hm)
					src_pix_ptr += stride; 
				stack_pix_ptr = &stack[i + ry];
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
				sum_r		   += src_pix_ptr[2] * (ry + 1 - i);
				sum_g		   += src_pix_ptr[1] * (ry + 1 - i);
				sum_b		   += src_pix_ptr[0] * (ry + 1 - i);
				sum_a		   += src_pix_ptr[3] * (ry + 1 - i);
				sum_in_r		+= src_pix_ptr[2];
				sum_in_g		+= src_pix_ptr[1];
				sum_in_b		+= src_pix_ptr[0];
				sum_in_a		+= src_pix_ptr[3];
			}

			stack_ptr = ry;
			yp = ry;
			if (yp > hm)
				yp = hm;
			src_pix_ptr = buffer + x * 4 + yp * bpr;
			dst_pix_ptr = buffer + x * 4;
			for (y = 0; y < h; y++) {
				dst_pix_ptr[2] = (sum_r * mul_sum) >> shr_sum;
				dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_pix_ptr[0] = (sum_b * mul_sum) >> shr_sum;
				dst_pix_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_pix_ptr += stride;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;
   
				stack_start = stack_ptr + div - ry;
				if (stack_start >= div)
					stack_start -= div;

				stack_pix_ptr = &stack[stack_start];
				sum_out_r -= stack_pix_ptr->r;
				sum_out_g -= stack_pix_ptr->g;
				sum_out_b -= stack_pix_ptr->b;
				sum_out_a -= stack_pix_ptr->a;

				if (yp < hm) {
					src_pix_ptr += stride;
					++yp;
				}
	
				stack_pix_ptr->r = src_pix_ptr[2];
				stack_pix_ptr->g = src_pix_ptr[1];
				stack_pix_ptr->b = src_pix_ptr[0];
				stack_pix_ptr->a = src_pix_ptr[3];
	
				sum_in_r += src_pix_ptr[2];
				sum_in_g += src_pix_ptr[1];
				sum_in_b += src_pix_ptr[0];
				sum_in_a += src_pix_ptr[3];
				sum_r	+= sum_in_r;
				sum_g	+= sum_in_g;
				sum_b	+= sum_in_b;
				sum_a	+= sum_in_a;
	
				++stack_ptr;
				if (stack_ptr >= div)
					stack_ptr = 0;
				stack_pix_ptr = &stack[stack_ptr];

				sum_out_r += stack_pix_ptr->r;
				sum_out_g += stack_pix_ptr->g;
				sum_out_b += stack_pix_ptr->b;
				sum_out_a += stack_pix_ptr->a;
				sum_in_r  -= stack_pix_ptr->r;
				sum_in_g  -= stack_pix_ptr->g;
				sum_in_b  -= stack_pix_ptr->b;
				sum_in_a  -= stack_pix_ptr->a;
			}
		}
	}
}


void
StackBlurFilter::_Filter8(uint8* buffer,
						  unsigned width, unsigned height,
						  int32 bpr,
						  unsigned rx, unsigned ry) const
{
	unsigned x, y, xp, yp, i;
	unsigned stack_ptr;
	unsigned stack_start;

	const uint8* src_pix_ptr;
		  uint8* dst_pix_ptr;
	unsigned pix;
	unsigned stack_pix;
	unsigned sum;
	unsigned sum_in;
	unsigned sum_out;

	unsigned w   = width;
	unsigned h   = height;
	unsigned wm  = w - 1;
	unsigned hm  = h - 1;

	unsigned div;
	unsigned mul_sum;
	unsigned shr_sum;

	pod_vector<uint8> stack;

	if(rx > 0)
	{
		if(rx > 254) rx = 254;
		div = rx * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[rx];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[rx];
		stack.allocate(div);

		for(y = 0; y < h; y++)
		{
			sum = sum_in = sum_out = 0;

			src_pix_ptr = buffer + y * bpr;
			pix = *src_pix_ptr;
			for(i = 0; i <= rx; i++)
			{
				stack[i] = pix;
				sum	 += pix * (i + 1);
				sum_out += pix;
			}
			for(i = 1; i <= rx; i++)
			{
				if(i <= wm) src_pix_ptr += 1; 
				pix = *src_pix_ptr; 
				stack[i + rx] = pix;
				sum	+= pix * (rx + 1 - i);
				sum_in += pix;
			}

			stack_ptr = rx;
			xp = rx;
			if(xp > wm) xp = wm;
			src_pix_ptr = buffer + xp + y * bpr;
			dst_pix_ptr = buffer + y * bpr;
			for(x = 0; x < w; x++)
			{
				*dst_pix_ptr = (sum * mul_sum) >> shr_sum;
				dst_pix_ptr += 1;

				sum -= sum_out;
   
				stack_start = stack_ptr + div - rx;
				if(stack_start >= div) stack_start -= div;
				sum_out -= stack[stack_start];

				if(xp < wm) 
				{
					src_pix_ptr += 1;
					pix = *src_pix_ptr;
					++xp;
				}
	
				stack[stack_start] = pix;
	
				sum_in += pix;
				sum	+= sum_in;
	
				++stack_ptr;
				if(stack_ptr >= div) stack_ptr = 0;
				stack_pix = stack[stack_ptr];

				sum_out += stack_pix;
				sum_in  -= stack_pix;
			}
		}
	}

	if(ry > 0)
	{
		if(ry > 254) ry = 254;
		div = ry * 2 + 1;
		mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[ry];
		shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[ry];
		stack.allocate(div);

		int stride = bpr;
		for(x = 0; x < w; x++)
		{
			sum = sum_in = sum_out = 0;

			src_pix_ptr = buffer + x;
			pix = *src_pix_ptr;
			for(i = 0; i <= ry; i++)
			{
				stack[i] = pix;
				sum	 += pix * (i + 1);
				sum_out += pix;
			}
			for(i = 1; i <= ry; i++)
			{
				if(i <= hm) src_pix_ptr += stride; 
				pix = *src_pix_ptr; 
				stack[i + ry] = pix;
				sum	+= pix * (ry + 1 - i);
				sum_in += pix;
			}

			stack_ptr = ry;
			yp = ry;
			if(yp > hm) yp = hm;
			src_pix_ptr = buffer + x + yp * bpr;
			dst_pix_ptr = buffer + x;
			for(y = 0; y < h; y++)
			{
				*dst_pix_ptr = (sum * mul_sum) >> shr_sum;
				dst_pix_ptr += stride;

				sum -= sum_out;
   
				stack_start = stack_ptr + div - ry;
				if(stack_start >= div) stack_start -= div;
				sum_out -= stack[stack_start];

				if(yp < hm) 
				{
					src_pix_ptr += stride;
					pix = *src_pix_ptr;
					++yp;
				}
	
				stack[stack_start] = pix;
	
				sum_in += pix;
				sum	+= sum_in;
	
				++stack_ptr;
				if(stack_ptr >= div) stack_ptr = 0;
				stack_pix = stack[stack_ptr];

				sum_out += stack_pix;
				sum_in  -= stack_pix;
			}
		}
	}
}

