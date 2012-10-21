//----------------------------------------------------------------------------
// Copyright (color) 2012 Stephan Aßmus
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: superstippi@gmx.de
//----------------------------------------------------------------------------

#ifndef AGG_PIXFMT_LCD_BGRA_16_INCLUDED
#define AGG_PIXFMT_LCD_BGRA_16_INCLUDED

#include <string.h>

#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "agg_primary_weights.h"
#include "agg_rendering_buffer.h"


namespace agg
{

	//=======================================================pixfmt_lcd_bgra16
	class pixfmt_lcd_bgra16
	{
	public:
		typedef rgba16 color_type;
		typedef rendering_buffer::row_data row_data;
		typedef color_type::value_type value_type;
		typedef color_type::calc_type calc_type;


		pixfmt_lcd_bgra16(rendering_buffer& buffer,
				const primary_weights& weights)
			:
			m_buffer(&buffer),
			m_weights(&weights)
		{
		}

		inline unsigned width() const
		{
			return m_buffer->width() * 3;
		}

		inline unsigned height() const
		{
			return m_buffer->height();
		}

		void blend_hline(int x, int y, unsigned length, const color_type& color,
			int8u cover)
		{
			int16u* p = ((int16u*) m_buffer->row_ptr(y)) + x + x + x + x;
			int alpha = int(cover) * color.a;
			do
			{
				p[0] = (int8u)((((color.b - p[0]) * alpha)
					+ (p[0] << 16)) >> 16);
				p[1] = (int8u)((((color.g - p[1]) * alpha)
					+ (p[1] << 16)) >> 16);
				p[2] = (int8u)((((color.r - p[2]) * alpha)
					+ (p[2] << 16)) >> 16);

				p += 4;
				--length;
			}
			while (length != 0);
		}

		void blend_solid_hspan(int x, int y, unsigned length,
			const color_type& color, const int8u* covers)
		{
			// How the padding needs to work due to swapping:

			// x = 2, length = 7
			// __ __ B0 | R1 G1 B1 | R2 G2 B2 |
			// r0 g0 B0 | r1 G1 B1 | R2 G2 B2 | r3 g3 __ |
			//->
			// B0 g0 r0 | B1 G1 r1 | B2 G2 R2 | __ g3 r3 |

			// x = 3, length = 8
			// __ __ __ | R1 G1 B1 | R2 G2 B2 | R3 G3 __ | __ __ __
			// __ g0 b0 | R1 G1 B1 | R2 G2 B2 | R3 G3 b3 | r4 __ __
			//->
			// b0 g0 __ | B1 G1 R1 | B2 G2 R2 | b3 G3 R3 | __ __ r4

			// x = 4, length = 6
			// __ __ __ | __ G1 B1 | R2 G2 B2 | R3 __ __ |
			// __ __ b0 | r1 G1 B1 | R2 G2 B2 | R3 g3 b3 |
			//->
			// b0 __ __ | B1 G1 r1 | B2 G2 R2 | b3 g3 R3 |

			int startPadding = 0;
			if (x % 3 == 0)
				startPadding = 1;
			else if (x % 3 == 1)
				startPadding = 2;
			else if (x % 3 == 2)
				startPadding = 0;

			int endPadding = 0;
			if ((x + length) % 3 == 0)
				endPadding = 1;
			else if ((x + length) % 3 == 1)
				endPadding = 0;
			else if ((x + length) % 3 == 2)
				endPadding = 2;

			// Filter the cover values according to the weights distribution
			// into a new cover buffer that starts 2 components earlier and ends
			// 2 components later.
			memset(m_filtered_covers, 0,
				length + 4 + startPadding + endPadding);

			for(int i = startPadding; i < int(length + startPadding); i++)
			{
				int j = i - startPadding;
				m_filtered_covers[i + 0] += m_weights->weight_c(covers[j]);
				m_filtered_covers[i + 1] += m_weights->weight_b(covers[j]);
				m_filtered_covers[i + 2] += m_weights->weight_a(covers[j]);
				m_filtered_covers[i + 3] += m_weights->weight_b(covers[j]);
				m_filtered_covers[i + 4] += m_weights->weight_c(covers[j]);
			}

			x -= 2 + startPadding;
			length += 4 + startPadding + endPadding;

			// TODO: Optimize this loop!
			for (int i = 0; i < int(length - 2); i++)
			{
				if ((i + x) % 3 == 0)
				{
					int8u t = m_filtered_covers[i];
					m_filtered_covers[i] = m_filtered_covers[i + 2];
					m_filtered_covers[i + 2] = t;
				}
			}

			covers = m_filtered_covers;

			if (x < 0)
			{
				if ((int) length > -x)
					length += x;
				else
					length = 0;
				covers = covers + (-x);
				x = 0;
			}

			if (x + length > width())
			{
				int maxLength = length - ((x + length) - width());
				if (maxLength <= 0)
					return;
				length = maxLength;
			}

			// x points to any of the B, G, or R component
			int component = x % 3;

			// Cache the swapped components
			int16u components[3] = { color.b, color.g, color.r };

			int16u* p = ((int16u*) m_buffer->row_ptr(y)) + x + x / 3;

			do
			{
				int alpha = int(*covers++) * color.a;
				if (alpha != 0)
				{
					if (alpha == 255 * 255)
					{
						*p = (int16u)components[component];
					}
					else
					{
						*p = (int16u)((((components[component] - *p) * alpha)
							+ (*p << 16)) >> 16);
					}
				}

				++p;
				++component;
				--length;

				if (component == 3)
				{
					component = 0;
					// Skip alpha channel
					++p;
				}
			}
			while (length != 0);
		}

	private:
		rendering_buffer* m_buffer;
		const primary_weights* m_weights;
		int8u m_filtered_covers[8192];
	};


}

#endif // AGG_PIXFMT_LCD_BGRA_16_INCLUDED

