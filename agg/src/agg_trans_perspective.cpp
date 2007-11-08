//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Affine transformations
//
//----------------------------------------------------------------------------
#include "agg_trans_perspective.h"



namespace agg
{

    //------------------------------------------------------------------------
    const trans_perspective& 
    trans_perspective::multiply_inv(const trans_perspective& m)
    {
        trans_perspective t = m;
        t.invert();
        return multiply(t);
    }

    //------------------------------------------------------------------------
    const trans_perspective&
    trans_perspective::multiply_inv(const trans_affine& m)
    {
        trans_affine t = m;
        t.invert();
        return multiply(t);
    }

    //------------------------------------------------------------------------
    const trans_perspective&
    trans_perspective::premultiply_inv(const trans_perspective& m)
    {
        trans_perspective t = m;
        t.invert();
        return *this = t.multiply(*this);
    }

    //------------------------------------------------------------------------
    const trans_perspective&
    trans_perspective::premultiply_inv(const trans_affine& m)
    {
        trans_perspective t(m);
        t.invert();
        return *this = t.multiply(*this);
    }

    //------------------------------------------------------------------------
    void trans_perspective::translation(double* dx, double* dy) const
    {
        *dx = tx;
        *dy = ty;
    }

    //------------------------------------------------------------------------
    void trans_perspective::scaling(double* x, double* y) const
    {
        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = 1.0;
        double y2 = 1.0;
        trans_perspective t(*this);
        t *= trans_affine_rotation(-rotation());
        t.transform(&x1, &y1);
        t.transform(&x2, &y2);
        *x = x2 - x1;
        *y = y2 - y1;
    }

    //------------------------------------------------------------------------
    void trans_perspective::scaling_abs(double* x, double* y) const
    {
        *x = sqrt(sx  * sx  + shx * shx);
        *y = sqrt(shy * shy + sy  * sy);
    }


}



