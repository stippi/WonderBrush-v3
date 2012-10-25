#ifndef PLATFORM_QT_B_RESOURCES_H
#define PLATFORM_QT_B_RESOURCES_H


#include <SupportDefs.h>
#include <TypeConstants.h>

#include <QHash>


class QByteArray;
class QString;


class BResources {
public:
								BResources();
								~BResources();

			void				Unset();

			status_t			AddResource(type_code type, int32 id,
									const QByteArray& data,
									const QByteArray& name = QByteArray());

			const void*			LoadResource(type_code type, int32 id,
									size_t* _outSize);

			BResources&			operator=(const BResources& other);

private:
			struct ResourceKey;
			struct Resource;

private:
			QHash<ResourceKey, Resource*> fResources;
};


#endif // PLATFORM_QT_B_RESOURCES_H
