#include "BMessageFilter.h"


BMessageFilter::BMessageFilter(uint32 what, filter_hook func)
{
// TODO:...
}


BMessageFilter::BMessageFilter(message_delivery delivery, message_source source,
	filter_hook func)
{
// TODO:...
}


BMessageFilter::BMessageFilter(message_delivery delivery, message_source source,
	uint32 what, filter_hook func)
{
// TODO:...
}


BMessageFilter::BMessageFilter(const BMessageFilter &filter)
{
// TODO:...
}


BMessageFilter::BMessageFilter(const BMessageFilter *filter)
{
// TODO:...
}


BMessageFilter::~BMessageFilter()
{
// TODO:...
}


filter_result
BMessageFilter::Filter(BMessage* message, BHandler** _target)
{
	return B_DISPATCH_MESSAGE;
}
