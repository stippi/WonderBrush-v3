#ifndef TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H
#define TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H


#include "TransformToolState.h"


class TransformToolState::PlatformDelegate {
public:
	PlatformDelegate(TransformToolState* state)
		:
		fState(state)
	{
	}

	void Draw(PlatformDrawContext& drawContext, DrawParameters& parameters)
	{
		QPainter painter(drawContext.View());

		if (parameters.subpixelPrecise)
			painter.setRenderHint(QPainter::Antialiasing, true);

		// white corner fills

		painter.setBrush(QColor(255, 255, 255));
		painter.setPen(Qt::NoPen);

		painter.drawPolygon(QPolygonF() << parameters.ltF0 << parameters.ltF1
			<< parameters.ltF2 << parameters.ltF3);
		painter.drawPolygon(QPolygonF() << parameters.rtF0 << parameters.rtF1
			<< parameters.rtF2 << parameters.rtF3);
		painter.drawPolygon(QPolygonF() << parameters.rbF0 << parameters.rbF1
			<< parameters.rbF2 << parameters.rbF3);
		painter.drawPolygon(QPolygonF() << parameters.lbF0 << parameters.lbF1
			<< parameters.lbF2 << parameters.lbF3);

		// transparent side fills

		painter.setBrush(QColor(0, 0, 0, 30));

		painter.drawPolygon(QPolygonF() << parameters.ltF0 << parameters.lt3
			<< parameters.rt1 << parameters.rtF0);
		painter.drawPolygon(QPolygonF() << parameters.rtF0 << parameters.rt3
			<< parameters.rb1 << parameters.rbF0);
		painter.drawPolygon(QPolygonF() << parameters.rbF0 << parameters.rb3
			<< parameters.lb1 << parameters.lbF0);
		painter.drawPolygon(QPolygonF() << parameters.lbF0 << parameters.lb3
			<< parameters.lt1 << parameters.ltF0);

		// white outlines

		painter.setPen(QColor(255, 255, 255, 200));
		painter.setBrush(Qt::NoBrush);

		painter.drawPolygon(QPolygonF() << parameters.ltF0 << parameters.rtF0
			<< parameters.rbF0 << parameters.lbF0);

		// black outlines

		painter.setPen(QColor(0, 0, 0, 200));

		painter.drawPolygon(QPolygonF() << parameters.lt1 << parameters.rt3
			<< parameters.rt2 << parameters.rt1 << parameters.rb3
			<< parameters.rb2 << parameters.rb1 << parameters.lb3
			<< parameters.lb2 << parameters.lb1 << parameters.lt3
			<< parameters.lt2);

		// pivot

		painter.setPen(QColor(0, 0, 0, 200));
		painter.setBrush(QColor(255, 255, 255, 200));

		painter.drawEllipse(parameters.pivot, parameters.pivotSize,
			parameters.pivotSize);
	}

private:
	TransformToolState*	fState;
};


#endif // TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H
