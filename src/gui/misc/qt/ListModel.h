#ifndef LISTMODEL_H
#define LISTMODEL_H


#include <QAbstractListModel>
#include <QBitArray>
#include <QMimeData>
#include <QStringList>
#include <QSet>


template<typename Item>
class ListModel : public QAbstractListModel {
public:
	ListModel(QObject* parent = NULL) :
		QAbstractListModel(parent),
		fCheckable(false),
		fDecorationEnabled(false),
		fDragSortable(false),
		fHeaderTitle(tr("Name"))
	{
	}

	bool IsDecorationEnabled() const
	{
		return fDecorationEnabled;
	}

	void setDecorationEnabled(bool decorationEnabled)
	{
		if (decorationEnabled != fDecorationEnabled) {
			beginResetModel();
			fDecorationEnabled = decorationEnabled;
			endResetModel();
		}
	}

	bool IsCheckable() const
	{
		return fCheckable;
	}

	void SetCheckable(bool checkable)
	{
		if (checkable != fCheckable) {
			beginResetModel();
			fCheckable = checkable;
			endResetModel();
		}
	}

	bool IsDragSortable() const
	{
		return fDragSortable;
	}

	void SetDragSortable(bool dragSortable)
	{
		fDragSortable = dragSortable;
	}

	void SetHeaderTitle(const QString& title)
	{
		fHeaderTitle = title;
		headerDataChanged(Qt::Horizontal, 0, 0);
	}

	int CountItems() const
	{
		return fItems.count();
	}

	QList<Item> Items() const
	{
		return fItems;
	}

	void SetItems(const QList<Item>& items)
	{
		beginResetModel();

		fItems = items;
		fCheckedItems.fill(false, fItems.count());

		endResetModel();
	}

	void MakeEmpty()
	{
		SetItems(QList<Item>());
	}

	void AppendItem(const Item& item)
	{
		InsertItem(fItems.count(), item);
	}

	bool InsertItem(int index, const Item& item)
	{
		if (index < 0 || index > fItems.count())
			return false;

		beginInsertRows(QModelIndex(), index, index);

		fItems.insert(index, item);

		// m_includedItems updaten -- die Indizes verschieben sich.
		QBitArray oldIncludedItems = fCheckedItems;
		int count = fItems.count();
		fCheckedItems.resize(count);
		fCheckedItems.setBit(index, false);
		for (int i = index + 1; i < count; i++)
			fCheckedItems.setBit(i, oldIncludedItems.at(i - 1));

		endInsertRows();

		return true;
	}

	bool RemoveItem(const Item& item)
	{
		return RemoveItemAt(IndexOf(item));
	}

	bool RemoveItemAt(int index)
	{
		if (index < 0 || index >= fItems.count())
			return false;

		beginRemoveRows(QModelIndex(), index, index);

		fItems.removeAt(index);

		// m_includedItems updaten -- die Indizes verschieben sich.
		QBitArray oldIncludedItems = fCheckedItems;
		int count = fItems.count();
		for (int i = index; i < count; i++)
			fCheckedItems.setBit(i, oldIncludedItems.at(i + 1));
		fCheckedItems.resize(count);

		endRemoveRows();

		return true;
	}

	void MoveItem(int fromIndex, int toIndex)
	{
		if (fromIndex < 0 || fromIndex >= fItems.count() || toIndex < 0
			|| toIndex >= fItems.count() || fromIndex == toIndex) {
			return;
		}

		beginMoveRows(QModelIndex(), fromIndex, fromIndex, QModelIndex(),
			toIndex > fromIndex ? toIndex + 1 : toIndex);

		fItems.move(fromIndex, toIndex);

		endMoveRows();
	}

	bool ReplaceItemAt(int index, const Item& newItem)
	{
		if (index < 0 || index >= fItems.count())
			return false;

		fItems.replace(index, newItem);
		ItemChangedAt(index);

		return true;
	}

	void ItemChanged(const Item& item)
	{
		return ItemChangedAt(IndexOf(item));
	}

	void ItemChangedAt(int index)
	{
		if (index >= 0 && index < fItems.count()) {
			QModelIndex index = this->index(index, 0);
			dataChanged(index, index);
		}
	}

	QList<Item> CheckedItems() const
	{
		QList<Item> items;
		GetCheckedItems(items);
		return items;
	}

	void GetCheckedItems(QList<Item>& items) const
	{
		int count = fItems.count();
		for (int i = 0; i < count; i++) {
			if (fCheckedItems.at(i))
				items.append(fItems.at(i));
		}
	}

	void SetCheckedItems(const QList<Item>& items)
	{
		SetCheckedItems(items.toSet());
	}

	void SetCheckedItems(const QSet<Item>& items)
	{
		int firstModified = -1;
		int lastModified = -1;

		int count = fItems.count();
		for (int i = 0; i < count; i++) {
			const Item& item = fItems.at(i);
			bool newValue = items.contains(item);
			if (newValue != fCheckedItems.at(i)) {
				fCheckedItems.setBit(i, newValue);

				if (firstModified < 0)
					firstModified = i;
				lastModified = i;
			}
		}

		if (firstModified >= 0)
			dataChanged(index(firstModified, 0), index(lastModified, 0));
	}

	Item ItemAt(const QModelIndex& index,
		const Item& fallback = Item()) const
	{
		return index.isValid() ? ItemAt(index.row(), fallback) : fallback;
	}

	Item ItemAt(int index, const Item& fallback = Item()) const
	{
		return index >= 0 && index < fItems.count()
			? fItems.at(index) : fallback;
	}

	int IndexOf(const Item& item) const
	{
		return fItems.indexOf(item);
	}

	QModelIndex ModelIndexOf(const Item& item) const
	{
		int index = fItems.indexOf(item);
		return index >= 0 ? this->index(index, 0) : QModelIndex();
	}

	virtual int rowCount(const QModelIndex& /*parent*/) const
	{
		return fItems.count();
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (!index.isValid())
			return QVariant();

		int rowIndex = index.row();
		if (rowIndex >= fItems.count())
			return QVariant();

		switch (role)
		{
			case Qt::CheckStateRole:
				if (fCheckable) {
					return fCheckedItems.at(rowIndex)
						? Qt::Checked : Qt::Unchecked;
				}
				break;

			case Qt::DisplayRole:
				return ItemText(fItems.at(rowIndex));

			case Qt::DecorationRole:
				if (fDecorationEnabled)
					return ItemDecoration(fItems.at(rowIndex));
				break;
		}

		return QVariant();
	}

	virtual bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole)
	{
		int rowIndex = index.row();
		if (!fCheckable || role != Qt::CheckStateRole || !index.isValid()
			|| rowIndex >= fItems.count()) {
			return false;
		}

		bool included = value.toInt() == Qt::Checked;
		if (included == fCheckedItems.at(rowIndex))
			return true;

		fCheckedItems.setBit(rowIndex, included);
		dataChanged(index, index);

		return true;
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		Qt::ItemFlags flags = Qt::ItemIsEnabled
			| (fCheckable ? Qt::ItemIsUserCheckable : Qt::ItemIsSelectable);
		if (fDragSortable) {
			flags |= index.isValid()
				? Qt::ItemIsDragEnabled : Qt::ItemIsDropEnabled;
		}

		return flags;
	}

	virtual QVariant headerData(int section, Qt::Orientation orientation,
		int i_role) const
	{
		if (i_role == Qt::DisplayRole && orientation == Qt::Horizontal
			&& section == 0) {
			return fHeaderTitle;
		}

		return QVariant();
	}

	virtual Qt::DropActions supportedDragActions() const
	{
		return fDragSortable ? Qt::MoveAction : Qt::DropActions(0);
	}

	virtual Qt::DropActions supportedDropActions() const
	{
		return fDragSortable ? Qt::MoveAction : Qt::DropActions(0);
	}

	virtual QStringList mimeTypes() const
	{
		if (!fDragSortable)
			return QStringList();

		QStringList types;
		types += QString::fromAscii(ItemMimeData::MimeString());
		return types;
	}

	virtual QMimeData* mimeData(const QModelIndexList& indexes) const
	{
		if (!fDragSortable)
			return 0;

		ItemMimeData* mimeData = new ItemMimeData;

		foreach (QModelIndex index, indexes) {
			if (index.isValid() && flags(index).testFlag(Qt::ItemIsDragEnabled))
				mimeData->AddItem(ItemAt(index));
		}

		return mimeData;
	}

	virtual bool dropMimeData(const QMimeData* data, Qt::DropAction /*action*/,
		int row, int /*column*/, const QModelIndex& /*parent*/)
	{
		const ItemMimeData* itemData = dynamic_cast<const ItemMimeData*>(data);
		if (itemData == NULL)
			return false;

		return DropItems(row, itemData->Items());
	}

	virtual QString ItemText(const Item& item) const = 0;

	virtual QVariant ItemDecoration(const Item& /*item*/) const
	{
		return QVariant();
	}

	virtual bool DropItems(int row, QList<Item> items)
	{
		return false;
	}

private:
	class ItemMimeData : public QMimeData {
	public:
		ItemMimeData()
		{
		}

		void AddItem(const Item& item)
		{
			fItems.append(item);
		}

		const QList<Item>& Items() const
		{
			return fItems;
		}

		virtual QStringList formats() const
		{
			QStringList types;
			types += QString::fromAscii(ItemMimeData::MimeString());
			return types;
		}

		virtual bool hasFormat(const QString& mimeType) const
		{
			return mimeType == QString::fromAscii(MimeString());
		}

		static const char* MimeString()
		{
			return "application/x-vnd.yellowbites-list-model-item";
		}

	private:
		QList<Item> fItems;
	};

protected:
	QList<Item>	fItems;
	QBitArray		fCheckedItems;
	bool			fCheckable;
	bool			fDecorationEnabled;
	bool			fDragSortable;
	QString			fHeaderTitle;
};


#endif // LISTMODEL_H
