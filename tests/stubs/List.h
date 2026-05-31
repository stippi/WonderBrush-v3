#ifndef WB_TEST_STUB_LIST_H
#define WB_TEST_STUB_LIST_H

#include <SupportDefs.h>

#include <vector>

template<typename Value, bool>
class List {
public:
	List() {}
};

class BList {
public:
	BList(int32 = 20) {}
	BList(const BList& other)
		: fItems(other.fItems)
	{
	}
	~BList() {}

	bool AddItem(void* item)
	{
		fItems.push_back(item);
		return true;
	}

	bool AddItem(void* item, int32 index)
	{
		if (index < 0 || index > (int32)fItems.size())
			return false;
		fItems.insert(fItems.begin() + index, item);
		return true;
	}

	bool RemoveItem(void* item)
	{
		int32 index = IndexOf(item);
		if (index < 0)
			return false;
		fItems.erase(fItems.begin() + index);
		return true;
	}

	void* RemoveItem(int32 index)
	{
		if (index < 0 || index >= (int32)fItems.size())
			return 0;
		void* item = fItems[index];
		fItems.erase(fItems.begin() + index);
		return item;
	}

	bool HasItem(void* item) const { return IndexOf(item) >= 0; }
	bool HasItem(const void* item) const { return IndexOf(item) >= 0; }

	int32 IndexOf(void* item) const { return IndexOf((const void*)item); }
	int32 IndexOf(const void* item) const
	{
		for (size_t i = 0; i < fItems.size(); i++) {
			if (fItems[i] == item)
				return (int32)i;
		}
		return -1;
	}

	void* ItemAt(int32 index) const
	{
		if (index < 0 || index >= (int32)fItems.size())
			return 0;
		return fItems[index];
	}

	void* ItemAtFast(int32 index) const { return fItems[index]; }
	int32 CountItems() const { return (int32)fItems.size(); }
	void MakeEmpty() { fItems.clear(); }

private:
	std::vector<void*> fItems;
};

#endif // WB_TEST_STUB_LIST_H
