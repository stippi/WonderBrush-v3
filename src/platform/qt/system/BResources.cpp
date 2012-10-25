#include "BResources.h"

#include <QByteArray>
#include <QString>


struct BResources::ResourceKey {
	ResourceKey(type_code type, int32 id)
		:
		fType(type),
		fId(id)
	{
	}

	type_code Type() const
	{
		return fType;
	}

	int32 Id() const
	{
		return fId;
	}

	bool operator==(const ResourceKey& other) const
	{
		return fType == other.fType && fId == other.fId;
	}

	bool operator!=(const ResourceKey& other) const
	{
		return !(*this == other);
	}

	friend uint qHash(const ResourceKey& key)
	{
		return (uint)key.fType * 37 + (uint)key.Id();
	}

private:
	type_code	fType;
	int32		fId;
};


struct BResources::Resource {
	Resource(const QByteArray& data, const QByteArray& name)
		:
		fData(data),
		fName(name)
	{
	}

	const QByteArray& Data() const
	{
		return fData;
	}

	const QByteArray& Name() const
	{
		return fName;
	}

private:
	QByteArray	fData;
	QByteArray	fName;
};


BResources::BResources()
{
}


BResources::~BResources()
{
	Unset();
}


void BResources::Unset()
{
	foreach (Resource* resource, fResources)
		delete resource;
	fResources.clear();
}


status_t
BResources::AddResource(type_code type, int32 id, const QByteArray& data,
	const QByteArray& name)
{
	Resource* resource = new(std::nothrow) Resource(data, name);
	if (resource == NULL)
		return B_NO_MEMORY;

	ResourceKey key(type, id);
	delete fResources.value(key, NULL);

	try {
		fResources.insert(key, resource);
	} catch (...) {
		delete resource;
		return B_NO_MEMORY;
	}

	return B_OK;
}


const void*
BResources::LoadResource(type_code type, int32 id, size_t* _outSize)
{
	Resource* resource = fResources.value(ResourceKey(type, id), NULL);
	if (resource == NULL)
		return NULL;

	const QByteArray& data = resource->Data();
	if (_outSize != NULL)
		*_outSize = data.size();
	return data.data();
}


BResources&
BResources::operator=(const BResources& other)
{
	Unset();

	for (QHash<ResourceKey, Resource*>::const_iterator it
				= other.fResources.begin();
			it != other.fResources.end(); ++it) {
		const ResourceKey& key = it.key();
		const Resource* resource = it.value();
		AddResource(key.Type(), key.Id(), resource->Data(), resource->Name());
	}

	return *this;
}
