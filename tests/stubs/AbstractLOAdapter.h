#ifndef WB_TEST_STUB_ABSTRACT_LO_ADAPTER_H
#define WB_TEST_STUB_ABSTRACT_LO_ADAPTER_H

class BHandler;
class BMessage;

class AbstractLOAdapter {
public:
	AbstractLOAdapter(BHandler*) {}
	virtual ~AbstractLOAdapter() {}
	void DeliverMessage(BMessage&) {}
};

#endif // WB_TEST_STUB_ABSTRACT_LO_ADAPTER_H
