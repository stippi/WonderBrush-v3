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

#ifndef AGG_PIXFMT_BRUSH_INCLUDED
#define AGG_PIXFMT_BRUSH_INCLUDED

#include <stdio.h>
#include <string.h>

#include "agg_basics.h"
#include "agg_color_gray.h"
#include "agg_rendering_buffer.h"

namespace agg
{

	//======================================================pixfmt_brush
	template<class ColorT, class RenBuf>
	class pixfmt_brush
	{
	public:
        typedef RenBuf                               rbuf_type;
        typedef typename rbuf_type::row_data         row_data;
		typedef ColorT                               color_type;
        typedef int                                  order_type; // A fake one
        typedef typename color_type::value_type      value_type;
        typedef typename color_type::calc_type       calc_type;
        enum base_scale_e 
        {
            base_shift = color_type::base_shift,
            base_scale = color_type::base_scale,
            base_mask  = color_type::base_mask,
            pix_width  = sizeof(value_type)
        };

		//--------------------------------------------------------------------
		pixfmt_brush(rendering_buffer& rb)
			: m_rbuf(&rb),
			  m_cover_scale(base_mask),
			  m_cover_scale_half(base_mask / 2),
			  m_solid(false)
		{
		}

		//--------------------------------------------------------------------
		unsigned width()  const { return m_rbuf->width();  }
		unsigned height() const { return m_rbuf->height(); }

        const int8u* pix_ptr(int x, int y) const
        {
            return m_rbuf->row_ptr(y) + x;
        }

        int8u* pix_ptr(int x, int y)
        {
            return m_rbuf->row_ptr(y) + x;
        }

        //--------------------------------------------------------------------
        AGG_INLINE static void make_pix(int8u* p, const color_type& c)
        {
            *(value_type*)p = c.v;
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type pixel(int x, int y) const
        {
            value_type* p = (value_type*)m_rbuf->row_ptr(y) + x;
            return color_type(*p);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
        {
            *((value_type*)m_rbuf->row_ptr(x, y, 1) + x) = c.v;
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
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x;
			if(m_solid)
			{
				do 
				{
					int alpha = color_type::int_mult_cover(c.v, *covers);
					alpha = color_type::int_mult_cover(alpha, m_cover_scale);
					if (alpha > m_cover_scale_half)
						*p = max_c(*p, m_cover_scale);

					++p;
					++covers;
				} while(--len);
			}
			else
			{
				do
				{
					int alpha = color_type::int_mult_cover(c.v, *covers);
					alpha = color_type::int_mult_cover(alpha, m_cover_scale);

//					if (alpha > *p)
//						*p = alpha;
					if (*p < m_cover_scale)
						*p += (alpha * (m_cover_scale - *p)) / m_cover_scale;

					++p;
					++covers;
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
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x;
			if(covers)
			{
				if(m_solid)
				{
					do
					{
						int alpha = color_type::int_mult_cover(colors->v, *covers);
						alpha = color_type::int_mult_cover(alpha, m_cover_scale);

						if (alpha > m_cover_scale_half)
							*p = max_c(*p, m_cover_scale);
	
						++p;
						++colors;
						++covers;
					} while(--len);
				}
				else
				{
					do
					{
						int alpha = color_type::int_mult_cover(colors->v, *covers);
						alpha = color_type::int_mult_cover(alpha, m_cover_scale);
	
//						if (alpha > *p)
//							*p = alpha;
						if (*p < m_cover_scale)
							*p += (alpha * (m_cover_scale - *p)) / m_cover_scale;
	
						++p;
						++colors;
						++covers;
					} while(--len);
				}
			}
			else
			{
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

		void cover_scale(value_type scale)
		{
			m_cover_scale = scale;
			m_cover_scale_half = scale / 2;
		}

		void solid(bool solid)
		{
			m_solid = solid;
		}

	private:
		rbuf_type*          m_rbuf;
		value_type			m_cover_scale;
		value_type			m_cover_scale_half;
		bool				m_solid;
	};
}

#endif // AGG_PIXFMT_BRUSH_INCLUDED

