/*
 * Copyright 2008-2012 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef GRAPHICS_DEFS_H
#define GRAPHICS_DEFS_H


#include <string.h>

#include <SupportDefs.h>

#include <QColor>


// Pattern
typedef struct pattern {
	uint8 data[8];
} pattern;


#ifdef __cplusplus
inline bool
operator==(const pattern& a, const pattern& b)
{
	return memcmp(a.data, b.data, sizeof(a.data)) == 0;
}


inline bool
operator!=(const pattern& a, const pattern& b)
{
	return !(a == b);
}
#endif // __cplusplus


extern const pattern B_SOLID_HIGH;
extern const pattern B_MIXED_COLORS;
extern const pattern B_SOLID_LOW;


// rgb_color
typedef struct rgb_color {
	uint8		red;
	uint8		green;
	uint8		blue;
	uint8		alpha;

#if defined(__cplusplus)
	// some convenient additions
	inline rgb_color&
	set_to(uint8 r, uint8 g, uint8 b, uint8 a = 255)
	{
		red = r;
		green = g;
		blue = b;
		alpha = a;
		return *this;
	}

	inline bool
	operator==(const rgb_color& other) const
	{
		return *(const uint32 *)this == *(const uint32 *)&other;
	}

	inline bool
	operator!=(const rgb_color& other) const
	{
		return *(const uint32 *)this != *(const uint32 *)&other;
	}

	inline rgb_color&
	operator=(const rgb_color& other)
	{
		return set_to(other.red, other.green, other.blue, other.alpha);
	}

	operator QColor() const
	{
		return QColor(red, green, blue, alpha);
	}
#endif
} rgb_color;


#if defined(__cplusplus)
inline rgb_color
make_color(uint8 red, uint8 green, uint8 blue, uint8 alpha = 255)
{
	rgb_color color = {red, green, blue, alpha};
	return color;
}
#endif


extern const rgb_color 	B_TRANSPARENT_COLOR;
extern const uint8		B_TRANSPARENT_MAGIC_CMAP8;
extern const uint16		B_TRANSPARENT_MAGIC_RGBA15;
extern const uint16		B_TRANSPARENT_MAGIC_RGBA15_BIG;
extern const uint32		B_TRANSPARENT_MAGIC_RGBA32;
extern const uint32		B_TRANSPARENT_MAGIC_RGBA32_BIG;
extern const uint8 		B_TRANSPARENT_8_BIT;
extern const rgb_color	B_TRANSPARENT_32_BIT;


// overlay
typedef struct overlay_rect_limits {
	uint16				horizontal_alignment;
	uint16				vertical_alignment;
	uint16				width_alignment;
	uint16				height_alignment;
	uint16				min_width;
	uint16				max_width;
	uint16				min_height;
	uint16				max_height;
	uint32				reserved[8];
} overlay_rect_limits;


typedef struct overlay_restrictions {
	overlay_rect_limits	source;
	overlay_rect_limits	destination;
	float				min_width_scale;
	float				max_width_scale;
	float				min_height_scale;
	float				max_height_scale;
	uint32				reserved[8];
} overlay_restrictions;


// Screen ID
struct screen_id { int32 id; };
extern const struct screen_id B_MAIN_SCREEN_ID;


// Color spaces
typedef enum {
	B_NO_COLOR_SPACE	= 0x0000,

	// linear color space (little endian)
	B_RGB32				= 0x0008,	// BGR-		-RGB 8:8:8:8
	B_RGBA32			= 0x2008,	// BGRA		ARGB 8:8:8:8
	B_RGB24				= 0x0003,	// BGR		 RGB 8:8:8
	B_RGB16				= 0x0005,	// BGR		 RGB 5:6:5
	B_RGB15				= 0x0010,	// BGR-		-RGB 1:5:5:5
	B_RGBA15			= 0x2010,	// BGRA		ARGB 1:5:5:5
	B_CMAP8				= 0x0004,	// 256 color index table
	B_GRAY8				= 0x0002,	// 256 greyscale table
	B_GRAY1				= 0x0001,	// Each bit represents a single pixel

	// linear color space (big endian)
	B_RGB32_BIG			= 0x1008,	// -RGB		BGR- 8:8:8:8
	B_RGBA32_BIG		= 0x3008,	// ARGB		BGRA 8:8:8:8
	B_RGB24_BIG			= 0x1003,	//  RGB		BGR  8:8:8
	B_RGB16_BIG			= 0x1005,	//  RGB		BGR  5:6:5
	B_RGB15_BIG			= 0x1010,	// -RGB		BGR- 5:5:5:1
	B_RGBA15_BIG		= 0x3010,	// ARGB		BGRA 5:5:5:1

	// linear color space (little endian, for completeness)
	B_RGB32_LITTLE		= B_RGB32,
	B_RGBA32_LITTLE		= B_RGBA32,
	B_RGB24_LITTLE		= B_RGB24,
	B_RGB16_LITTLE		= B_RGB16,
	B_RGB15_LITTLE		= B_RGB15,
	B_RGBA15_LITTLE		= B_RGBA15,

	// non linear color space -- incidently, all with 8 bits per value
	// Note, BBitmap and BView do not support all of these!

	// Loss / saturation points:
	//  Y		16 - 235 (absolute)
	//  Cb/Cr	16 - 240 (center 128)

	B_YCbCr422			= 0x4000,	// Y0  Cb0 Y1  Cr0
									// Y2  Cb2 Y3  Cr4
	B_YCbCr411			= 0x4001,	// Cb0 Y0  Cr0 Y1
									// Cb4 Y2  Cr4 Y3
									// Y4  Y5  Y6  Y7
	B_YCbCr444			= 0x4003,	// Y   Cb  Cr
	B_YCbCr420			= 0x4004,	// Non-interlaced only
		// on even scan lines: Cb0  Y0  Y1  Cb2 Y2  Y3
		// on odd scan lines:  Cr0  Y0  Y1  Cr2 Y2  Y3

	// Extrema points are:
	//  Y 0 - 207 (absolute)
	//  U -91 - 91 (offset 128)
	//  V -127 - 127 (offset 128)

	// Note that YUV byte order is different from YCbCr; use YCbCr, not YUV,
	// when that's what you mean!
	B_YUV422			= 0x4020,	// U0  Y0  V0  Y1
									// U2  Y2  V2  Y3
	B_YUV411			= 0x4021,	// U0  Y0  Y1  V0  Y2  Y3
									// U4  Y4  Y5  V4  Y6  Y7
	B_YUV444			= 0x4023,	// U0  Y0  V0  U1  Y1  V1
	B_YUV420			= 0x4024,	// Non-interlaced only
		// on even scan lines: U0  Y0  Y1  U2 Y2  Y3
		// on odd scan lines:  V0  Y0  Y1  V2 Y2  Y3
	B_YUV9				= 0x402C,
	B_YUV12				= 0x402D,

	B_UVL24				= 0x4030,	// UVL
	B_UVL32				= 0x4031,	// UVL-
	B_UVLA32			= 0x6031,	// UVLA

	// L lightness, a/b color-opponent dimensions
	B_LAB24				= 0x4032,	// Lab
	B_LAB32				= 0x4033,	// Lab-
	B_LABA32			= 0x6033,	// LabA

	// Red is at hue 0
	B_HSI24				= 0x4040,	// HSI
	B_HSI32				= 0x4041,	// HSI-
	B_HSIA32			= 0x6041,	// HSIA

	B_HSV24				= 0x4042,	// HSV
	B_HSV32				= 0x4043,	// HSV-
	B_HSVA32			= 0x6043,	// HSVA

	B_HLS24				= 0x4044,	// HLS
	B_HLS32				= 0x4045,	// HLS-
	B_HLSA32			= 0x6045,	// HLSA

	B_CMY24				= 0xC001,	// CMY
	B_CMY32				= 0xC002,	// CMY-
	B_CMYA32			= 0xE002,	// CMYA
	B_CMYK32			= 0xC003,	// CMYK

	// Compatibility declarations
	B_MONOCHROME_1_BIT	= B_GRAY1,
	B_GRAYSCALE_8_BIT	= B_GRAY8,
	B_COLOR_8_BIT		= B_CMAP8,
	B_RGB_32_BIT		= B_RGB32,
	B_RGB_16_BIT		= B_RGB15,
	B_BIG_RGB_32_BIT	= B_RGB32_BIG,
	B_BIG_RGB_16_BIT	= B_RGB15_BIG
} color_space;



#endif // GRAPHICS_DEFS_H
