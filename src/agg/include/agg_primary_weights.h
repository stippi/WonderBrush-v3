//----------------------------------------------------------------------------
// Copyright (C) 2012 Stephan Aßmus
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: superstippi@gmx.de
//----------------------------------------------------------------------------

#ifndef AGG_PRIMARY_WEIGHTS_INCLUDED
#define AGG_PRIMARY_WEIGHTS_INCLUDED

#include "agg_basics.h"

namespace agg
{

	//===========================================================primary_weights
	class primary_weights
	{
	public:
		primary_weights(double a, double b, double c)
		{
			init(a, b, c);
		}

		void init(double a, double b, double c)
		{
			double normalizationScale = 1.0 / (a + b * 2.0 + c * 2.0);

			a = a * normalizationScale;
			b = b * normalizationScale;
			c = c * normalizationScale;

			for(unsigned i = 0; i < 256; i++)
			{
				m_weights_a[i] = (unsigned char)floor(a * i);
				m_weights_b[i] = (unsigned char)floor(b * i);
				m_weights_c[i] = (unsigned char)floor(c * i);
			}
		}

		// Access to cached weights

		inline unsigned weight_a(unsigned value) const
		{
			return m_weights_a[value];
		}

		inline unsigned weight_b(unsigned value) const
		{
			return m_weights_b[value];
		}

		inline unsigned weight_c(unsigned value) const
		{
			return m_weights_c[value];
		}

	private:
		unsigned char m_weights_a[256];
		unsigned char m_weights_b[256];
		unsigned char m_weights_c[256];
	};

}

#endif // AGG_PRIMARY_WEIGHTS_INCLUDED

