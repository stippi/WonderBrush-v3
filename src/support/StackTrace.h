/*
 * Copyright 2012 Ingo Weinhold <ingo_weinhold@gmx.de>
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 */

#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <Locker.h>

void print_stack_trace();

BLocker& get_stack_trace_locker();

#endif // STACK_TRACE_H
