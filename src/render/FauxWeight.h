/*
 * FauxWeight.h
 *
 *  Created on: 23.08.2012
 *      Author: stippi
 */

#ifndef FAUXWEIGHT_H_
#define FAUXWEIGHT_H_


template<class VertexSource>
class FauxWeight {
public:
	FauxWeight(VertexSource& source)
		:
		fMatrixZoomInY(agg::trans_affine_scaling(1, 100)),
		fMatrixZoomOutY(agg::trans_affine_scaling(1, 1.0 / 100.0)),
		fSource(&source),
		fTransformZoomInY(*fSource, fMatrixZoomInY),
		fContour(fTransformZoomInY),
		fTransformZoomOut(fContour, fMatrixZoomOutY)
	{
		fContour.auto_detect_orientation(false);
	}

	void weight(double v)
	{
		fContour.width(v);
	}

	void rewind(unsigned path_id = 0)
	{
		fTransformZoomOut.rewind(path_id);
	}

	unsigned vertex(double* x, double* y)
	{
		return fTransformZoomOut.vertex(x, y);
	}

private:
	typedef agg::trans_affine						Matrix;
	typedef agg::conv_transform<VertexSource>		TransformedSource;
	typedef agg::conv_contour<TransformedSource>	ContouredSource;
	typedef agg::conv_transform<ContouredSource>	InverseTransformedSource;

	Matrix						fMatrixZoomInY;
	Matrix						fMatrixZoomOutY;
	VertexSource*				fSource;
	TransformedSource			fTransformZoomInY;
	ContouredSource				fContour;
	InverseTransformedSource	fTransformZoomOut;
};


#endif /* FAUXWEIGHT_H_ */
