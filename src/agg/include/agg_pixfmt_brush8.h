//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.1
// Copyright (C) 2002-2004 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//		  mcseemagg@yahoo.com
//		  http://www.antigrain.com
//----------------------------------------------------------------------------

#ifndef AGG_PIXFMT_BRUSH8_INCLUDED
#define AGG_PIXFMT_BRUSH8_INCLUDED

#include <stdio.h>
#include <string.h>

#include <agg_basics.h>
#include <agg_color_gray.h>
#include <agg_rendering_buffer.h>

#include "blending.h"

namespace agg
{

	//======================================================pixfmt_brush
	template<class ColorT>
	class pixfmt_brush
	{
	public:
		typedef ColorT                               color_type;
        typedef int                                  order_type; // A fake one
        typedef typename color_type::value_type      value_type;
        typedef typename color_type::calc_type       calc_type;
        typedef typename rendering_buffer::row_data  row_data;
        typedef typename rendering_buffer::span_data span_data;

		//--------------------------------------------------------------------
		pixfmt_brush(rendering_buffer& rb)
			: m_rbuf(&rb),
			  m_cover_scale(255),
			  m_cover_scale_half(127),
			  m_solid(false)
		{
		}

		//--------------------------------------------------------------------
		unsigned width()  const { return m_rbuf->width();  }
		unsigned height() const { return m_rbuf->height(); }

		//--------------------------------------------------------------------
		color_type pixel(int x, int y)
		{
			return color_type(m_rbuf->row(y)[x]);
		}

		//--------------------------------------------------------------------
		void copy_pixel(int x, int y, const color_type& c)
		{
printf("copy_pixel()\n");
		}

		//--------------------------------------------------------------------
		void blend_pixel(int x, int y, const color_type& c, int8u cover)
		{
printf("blend_pixel()\n");
		}


		//--------------------------------------------------------------------
		void copy_hline(int x, int y, unsigned len, const color_type& c)
		{
printf("copy_hline()\n");
		}


		//--------------------------------------------------------------------
		void copy_vline(int x, int y, unsigned len, const color_type& c)
		{
printf("copy_vline()\n");
		}


		//--------------------------------------------------------------------
		void blend_hline(int x, int y, unsigned len, 
						 const color_type& c, int8u cover)
		{
printf("blend_hline()\n");
		}


		//--------------------------------------------------------------------
		void blend_vline(int x, int y, unsigned len, 
						 const color_type& c, int8u cover)
		{
printf("blend_vline()\n");
		}


		//--------------------------------------------------------------------
		void copy_from(const rendering_buffer& from, 
					   int xdst, int ydst,
					   int xsrc, int ysrc,
					   unsigned len)
		{
printf("copy_from()\n");
		}


		//--------------------------------------------------------------------
		void blend_solid_hspan(int x, int y, unsigned len, 
							   const color_type& c, const int8u* covers)
		{
			int8u* p = m_rbuf->row(y) + x;
			if(m_solid) {
				do 
				{
					int alpha = color_type::int_mult_cover(c.v, *covers++);
					if (alpha > m_cover_scale_half)
						*p = max_c(*p, m_cover_scale);

					p++;
				} while(--len);
			} else {
				do {
					int alpha = INT_MULT(*covers++, c.v, t);

					*p = max_c(*p, alpha);
					p++;
				} while(--len);
			}
		}


		//--------------------------------------------------------------------
		void blend_solid_vspan(int x, int y, unsigned len, 
							   const color_type& c, const int8u* covers)
		{
printf("blend_solid_vspan()\n");
		}


		//--------------------------------------------------------------------
		void blend_color_hspan(int x, int y, unsigned len, 
							   const color_type* colors,
							   const int8u* covers, int8u cover)
		{
			int8u* p = m_rbuf->row(y) + x;
			if(covers) {
				int t;
				if(m_solid) {
					do {
						int alpha = color_type::int_mult_cover(colors->v, *covers++);
						alpha = color_type::int_mult_cover(alpha, m_cover_scale);

						if (alpha > m_cover_scale_half)
							*p = max_c(*p, m_cover_scale);
	
						++p;
						++colors;
					} while(--len);
				} else {
					do {
						int alpha = color_type::int_mult_cover(colors->v, *covers++);
						alpha = color_type::int_mult_cover(alpha, m_cover_scale);
	
						if (*p < m_cover_scale)
							*p += (alpha * (m_cover_scale - *p)) / m_cover_scale;
	
						++p;
						++colors;
					} while(--len);
				}
			} else {
printf("blend_color_hspan() - no covers\n");
			}
		}



		//--------------------------------------------------------------------
		void blend_color_vspan(int x, int y, unsigned len, 
							   const color_type* colors,
							   const int8u* covers, int8u cover)
		{
printf("blend_color_vspan()\n");
		}

		void cover_scale(int8u scale)
		{
			m_cover_scale = scale;
			m_cover_scale_half = scale / 2;
		}

		void solid(bool solid)
		{
			m_solid = solid;
		}

	private:
		rendering_buffer*	m_rbuf;
		int8u				m_cover_scale;
		int8u				m_cover_scale_half;
		bool				m_solid;
	};
}

#endif // AGG_PIXFMT_BRUSH8_INCLUDED

