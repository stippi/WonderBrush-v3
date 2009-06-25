/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef SELECTION_H
#define SELECTION_H

#include <vector>

#include <List.h>

#include "Selectable.h"

class Selection {
public:
	class Controller {
	public:
								Controller();
		virtual					~Controller();
	};

	class Listener {
	public:
								Listener();
		virtual					~Listener();

		virtual	void			ObjectSelected(const Selectable& object,
									const Controller* controller) = 0;
		virtual	void			ObjectDeselected(const Selectable& object,
									const Controller* controller) = 0;
	};

public:
								Selection();
	virtual						~Selection();

	// modify selection
			bool				Select(const Selectable& object,
									const Controller* controller,
									bool extend = false);
			void				Deselect(const Selectable& object,
									const Controller* controller);
			void				DeselectAll(const Controller* controller);

	// query selection
			const Selectable&	SelectableAt(uint32 index) const;
			const Selectable&	SelectableAtFast(uint32 index) const;
			uint32				CountSelected() const;
			bool				IsEmpty() const;

	// listener support
			bool				AddListener(Listener* listener);
			void				RemoveListener(Listener* listener);

private:
			void				_DeselectAllExcept(const Selectable& object,
									const Controller* controller);

			void				_NotifyObjectSelected(
									const Selectable& object,
									const Controller* controller);
			void				_NotifyObjectDeselected(
									const Selectable& object,
									const Controller* controller);

	typedef std::vector<Selectable> Container;
			Container			fSelected;
			BList				fListeners;
};

#endif	// SELECTION_H
