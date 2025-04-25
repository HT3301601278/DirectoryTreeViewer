#include "TreeModel.h"
#include <QFileInfo>
#include <QFileIconProvider>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent), rootNode(nullptr),
      m_sortColumn(0), m_sortOrder(Qt::AscendingOrder), m_showHidden(false),
      m_filesScanned(0), m_dirsScanned(0)
{
    rootNode = new TreeNode("Root", "", true, nullptr);
}

TreeModel::~TreeModel()
{
    delete rootNode;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeNode *item = getNode(index);

    switch (role) {
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::DecorationRole:
            if (index.column() == 0)
                return item->icon();
            break;
        case Qt::ToolTipRole:
            return item->path();
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("名称");
            case 1: return tr("大小");
            case 2: return tr("修改日期");
            default: return QVariant();
        }
    }
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeNode *parentItem;
    if (!parent.isValid())
        parentItem = rootNode;
    else
        parentItem = getNode(parent);

    TreeNode *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeNode *childItem = getNode(index);
    TreeNode *parentItem = childItem->parent();

    if (parentItem == rootNode || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeNode *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootNode;
    else
        parentItem = getNode(parent);

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return getNode(parent)->columnCount();
    else
        return rootNode->columnCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeNode *item = getNode(index);
    bool result = item->setData(index.column(), value);
    if (result)
        emit dataChanged(index, index);

    return result;
}

void TreeModel::setRootPath(const QString &path)
{
    beginResetModel();
    
    delete rootNode;
    m_filesScanned = 0;
    m_dirsScanned = 0;
    
    QFileInfo fileInfo(path);
    QString rootName = fileInfo.fileName();
    if (rootName.isEmpty())
        rootName = path;
    
    rootNode = new TreeNode(rootName, path, true, nullptr);
    setupModelData(path, rootNode);
    
    endResetModel();
    emit directoryLoaded(path);
}

QString TreeModel::rootPath() const
{
    return rootNode->path();
}

void TreeModel::setIgnorePatterns(const QStringList &patterns)
{
    m_ignorePatterns = patterns;
}

QStringList TreeModel::ignorePatterns() const
{
    return m_ignorePatterns;
}

void TreeModel::setShowHiddenFiles(bool show)
{
    m_showHidden = show;
}

bool TreeModel::showHiddenFiles() const
{
    return m_showHidden;
}

void TreeModel::setSortColumn(int column)
{
    m_sortColumn = column;
}

void TreeModel::setSortOrder(Qt::SortOrder order)
{
    m_sortOrder = order;
}

void TreeModel::expandAll(const QModelIndex &index)
{
    // 递归展开所有节点
    TreeNode *node;
    if (!index.isValid())
        node = rootNode;
    else
        node = getNode(index);
    
    // 设置当前节点为展开状态
    node->setExpanded(true);
    
    // 递归展开所有子节点
    for (int i = 0; i < node->childCount(); ++i) {
        TreeNode *child = node->child(i);
        if (child->isDir()) {
            QModelIndex childIndex = createIndex(i, 0, child);
            expandAll(childIndex);
        }
    }
}

void TreeModel::collapseAll(const QModelIndex &index)
{
    // 递归折叠所有节点
    TreeNode *node;
    if (!index.isValid())
        node = rootNode;
    else
        node = getNode(index);
    
    // 设置当前节点为折叠状态
    node->setExpanded(false);
    
    // 递归折叠所有子节点
    for (int i = 0; i < node->childCount(); ++i) {
        TreeNode *child = node->child(i);
        if (child->isDir()) {
            QModelIndex childIndex = createIndex(i, 0, child);
            collapseAll(childIndex);
        }
    }
}

QModelIndexList TreeModel::findItems(const QString &text, Qt::MatchFlags flags)
{
    QModelIndexList result;
    
    // 递归搜索匹配的项
    QModelIndex rootIndex = index(0, 0, QModelIndex());
    QModelIndexList indexes = match(rootIndex, Qt::DisplayRole, text, -1, flags);
    result.append(indexes);
    
    // 递归搜索子节点
    for (int i = 0; i < rowCount(rootIndex); ++i) {
        QModelIndex childIndex = index(i, 0, rootIndex);
        indexes = match(childIndex, Qt::DisplayRole, text, -1, flags);
        result.append(indexes);
        
        // 如果是目录，则递归搜索
        TreeNode *node = getNode(childIndex);
        if (node->isDir() && node->childCount() > 0) {
            result.append(findItems(text, flags));
        }
    }
    
    return result;
}

void TreeModel::refresh(const QModelIndex &index)
{
    if (!index.isValid()) {
        // 刷新整个模型
        beginResetModel();
        
        QString path = rootNode->path();
        delete rootNode;
        m_filesScanned = 0;
        m_dirsScanned = 0;
        
        QFileInfo fileInfo(path);
        QString rootName = fileInfo.fileName();
        if (rootName.isEmpty())
            rootName = path;
        
        rootNode = new TreeNode(rootName, path, true, nullptr);
        setupModelData(path, rootNode);
        
        endResetModel();
        emit directoryLoaded(path);
    } else {
        // 仅刷新选中的节点
        TreeNode *node = getNode(index);
        if (node && node->isDir()) {
            // 删除所有现有子项
            beginRemoveRows(index, 0, node->childCount() - 1);
            
            QVector<TreeNode*> children;
            for (int i = 0; i < node->childCount(); ++i) {
                children.append(node->child(i));
            }
            qDeleteAll(children);
            
            endRemoveRows();
            
            // 重新加载目录
            setupModelData(node->path(), node);
            emit dataChanged(index, index);
        }
    }
}

TreeNode *TreeModel::getNode(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<TreeNode*>(index.internalPointer());
    else
        return rootNode;
}

void TreeModel::setupModelData(const QString &path, TreeNode *parent)
{
    QDir dir(path);
    
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (m_showHidden)
        filters |= QDir::Hidden;
    dir.setFilter(filters);
    
    QFileInfoList entries = dir.entryInfoList();
    
    // 过滤出目录和文件
    QVector<TreeNode*> dirs;
    QVector<TreeNode*> files;
    
    for (const QFileInfo &info : entries) {
        if (shouldIgnore(info.fileName()))
            continue;
            
        if (info.isDir()) {
            TreeNode *dirNode = new TreeNode(info.fileName(), info.filePath(), true, parent);
            dirs.append(dirNode);
            m_dirsScanned++;
            emit scanProgress(m_filesScanned, m_dirsScanned);
            
            // 递归处理子目录
            setupModelData(info.filePath(), dirNode);
        } else {
            TreeNode *fileNode = new TreeNode(info.fileName(), info.filePath(), false, parent);
            files.append(fileNode);
            m_filesScanned++;
            emit scanProgress(m_filesScanned, m_dirsScanned);
        }
    }
    
    // 排序
    sortNodes(dirs);
    sortNodes(files);
    
    // 添加到父节点
    for (auto *dirNode : dirs) {
        parent->appendChild(dirNode);
    }
    
    for (auto *fileNode : files) {
        parent->appendChild(fileNode);
    }
}

bool TreeModel::shouldIgnore(const QString &name) const
{
    for (const QString &pattern : m_ignorePatterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(name)) {
            return true;
        }
    }
    return false;
}

void TreeModel::sortNodes(QVector<TreeNode*> &nodes)
{
    std::sort(nodes.begin(), nodes.end(), [this](TreeNode *a, TreeNode *b) {
        switch (m_sortColumn) {
            case 0: // 名称
                if (m_sortOrder == Qt::AscendingOrder)
                    return a->name().toLower() < b->name().toLower();
                else
                    return a->name().toLower() > b->name().toLower();
            case 1: // 大小
                if (m_sortOrder == Qt::AscendingOrder)
                    return QFileInfo(a->path()).size() < QFileInfo(b->path()).size();
                else
                    return QFileInfo(a->path()).size() > QFileInfo(b->path()).size();
            case 2: // 修改时间
                if (m_sortOrder == Qt::AscendingOrder)
                    return QFileInfo(a->path()).lastModified() < QFileInfo(b->path()).lastModified();
                else
                    return QFileInfo(a->path()).lastModified() > QFileInfo(b->path()).lastModified();
            default:
                return a->name().toLower() < b->name().toLower();
        }
    });
} 