#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QDir>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QRegExp>

#include "TreeNode.h"

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel() override;

    // 基本模型API
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    // 树状结构特定方法
    void setRootPath(const QString &path);
    QString rootPath() const;
    
    // 过滤与排序
    void setIgnorePatterns(const QStringList &patterns);
    QStringList ignorePatterns() const;
    void setShowHiddenFiles(bool show);
    bool showHiddenFiles() const;
    void setSortColumn(int column);
    void setSortOrder(Qt::SortOrder order);
    
    // 搜索与展开
    void expandAll(const QModelIndex &index = QModelIndex());
    void collapseAll(const QModelIndex &index = QModelIndex());
    QModelIndexList findItems(const QString &text, Qt::MatchFlags flags = Qt::MatchContains);

signals:
    void directoryLoaded(const QString &path);
    void scanProgress(int filesScanned, int dirsScanned);

public slots:
    void refresh(const QModelIndex &index = QModelIndex());
    
private:
    TreeNode *getNode(const QModelIndex &index) const;
    void setupModelData(const QString &path, TreeNode *parent);
    bool shouldIgnore(const QString &name) const;
    void sortNodes(QVector<TreeNode*> &nodes);
    
    TreeNode *rootNode;
    QStringList m_ignorePatterns;
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;
    bool m_showHidden;
    int m_filesScanned;
    int m_dirsScanned;
};

#endif // TREEMODEL_H 