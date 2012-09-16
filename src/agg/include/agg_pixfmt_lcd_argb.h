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

#ifndef AGG_PIXFMT_LCD_ARGB_INCLUDED
#define AGG_PIXFMT_LCD_ARGB_INCLUDED

#include <string.h>

#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "agg_primary_weights.h"
#include "agg_rendering_buffer.h"


namespace agg
{

	//==========================================================pixfmt_lcd_argb
	class pixfmt_lcd_argb
	{
	public:
		typedef rgba8 color_type;
		typedef rendering_buffer::row_data row_data;
		typedef color_type::value_type value_type;
		typedef color_type::calc_type calc_type;


		pixfmt_lcd_argb(rendering_buffer& buffer,
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
			int8u* p = m_buffer->row_ptr(y) + x + x + x + x;
			int alpha = int(cover) * color.a;
			do
			{
				p[0] = (int8u)((((color.r - p[1]) * alpha)
					+ (p[0] << 16)) >> 16);
				p[1] = (int8u)((((color.g - p[2]) * alpha)
					+ (p[1] << 16)) >> 16);
				p[2] = (int8u)((((color.b - p[3]) * alpha)
					+ (p[2] << 16)) >> 16);

				p += 4;
				--length;
			}
			while (length != 0);
		}

		void blend_solid_hspan(int x, int y, unsigned length,
			const color_type& color, const int8u* covers)
		{
			// Filter the cover values according to the weights distribution
			// into a new cover buffer that starts 2 components earlier and ends
			// 2 components later.
			memset(m_filtered_covers, 0, length + 4);

			for(int i = 0; i < int(length); i++)
			{
				m_filtered_covers[i + 0] += m_weights->weight_c(covers[i]);
				m_filtered_covers[i + 1] += m_weights->weight_b(covers[i]);
				m_filtered_covers[i + 2] += m_weights->weight_a(covers[i]);
				m_filtered_covers[i + 3] += m_weights->weight_b(covers[i]);
				m_filtered_covers[i + 4] += m_weights->weight_c(covers[i]);
			}

			x -= 2;
			length += 4;

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
			int8u components[3] = { color.r, color.g, color.b };

			int8u* p = m_buffer->row_ptr(y) + x + x / 3 + 1;

			do
			{
				int alpha = int(*covers++) * color.a;
				if (alpha != 0)
				{
					if (alpha == 255 * 255)
					{
						*p = (int8u)components[component];
					}
					else
					{
						*p = (int8u)((((components[component] - *p) * alpha)
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

#endif // AGG_PIXFMT_LCD_ARGB_INCLUDED

