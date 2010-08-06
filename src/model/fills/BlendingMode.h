/*
 * Copyright 2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef BLENDING_MODE_H
#define BLENDING_MODE_H

#include <agg_pixfmt_rgba.h>

enum BlendingMode {
	CompOpClear					= agg::comp_op_clear,
	CompOpSrc					= agg::comp_op_src,
	CompOpDst					= agg::comp_op_dst,
	CompOpSrcOver				= agg::comp_op_src_over,
	CompOpDstOver				= agg::comp_op_dst_over,
	CompOpSrcIn					= agg::comp_op_src_in,
	CompOpDstIn					= agg::comp_op_dst_in,
	CompOpSrcOut				= agg::comp_op_src_out,
	CompOpDstOut				= agg::comp_op_dst_out,
	CompOpSrcAtop				= agg::comp_op_src_atop,
	CompOpDstAtop				= agg::comp_op_dst_atop,
	CompOpXor					= agg::comp_op_xor,
	CompOpPlus					= agg::comp_op_plus,
	CompOpMinus					= agg::comp_op_minus,
	CompOpMultiply				= agg::comp_op_multiply,
	CompOpScreen				= agg::comp_op_screen,
	CompOpOverlay				= agg::comp_op_overlay,
	CompOpDarken				= agg::comp_op_darken,
	CompOpLighten				= agg::comp_op_lighten,
	CompOpDodge					= agg::comp_op_color_dodge,
	CompOpColorBurn				= agg::comp_op_color_burn,
	CompOpHardLight				= agg::comp_op_hard_light,       
	CompOpSoftLight				= agg::comp_op_soft_light,
	CompOpDifference			= agg::comp_op_difference,
	CompOpExclusion				= agg::comp_op_exclusion,
	CompOpContrast				= agg::comp_op_contrast,
	CompOpInvert				= agg::comp_op_invert,
	CompOpInvertRGB				= agg::comp_op_invert_rgb
};

static const BlendingMode kMinBlendingMode = CompOpClear;
static const BlendingMode kMaxBlendingMode = CompOpInvertRGB;

#endif // BLENDING_MODE_H
