#include "TreeNode.h"
#include <QFileIconProvider>
#include <QDateTime>

TreeNode::TreeNode(const QString &name, const QString &path, bool isDir, TreeNode *parent)
    : m_name(name), m_path(path), m_isDir(isDir), m_fileInfo(path), 
      m_isExpanded(false), m_parentItem(parent)
{
}

TreeNode::~TreeNode()
{
    qDeleteAll(m_childItems);
}

QIcon TreeNode::icon() const
{
    static QFileIconProvider iconProvider;
    return m_isDir ? iconProvider.icon(QFileIconProvider::Folder) 
                   : iconProvider.icon(QFileIconProvider::File);
}

QString TreeNode::size() const
{
    if (m_isDir) {
        return QString("<目录>");
    }
    
    qint64 size = m_fileInfo.size();
    if (size < 1024) {
        return QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
    } else if (size < 1024 * 1024 * 1024) {
        return QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        return QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
    }
}

QString TreeNode::modifiedTime() const
{
    return m_fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
}

void TreeNode::appendChild(TreeNode *child)
{
    m_childItems.append(child);
}

TreeNode* TreeNode::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int TreeNode::childCount() const
{
    return m_childItems.count();
}

int TreeNode::columnCount() const
{
    return 3; // 名称、大小、修改时间
}

int TreeNode::row() const
{
    if (m_parentItem) {
        for (int i = 0; i < m_parentItem->m_childItems.size(); ++i) {
            if (m_parentItem->m_childItems[i] == this)
                return i;
        }
    }
    return 0;
}

TreeNode* TreeNode::parent()
{
    return m_parentItem;
}

QVariant TreeNode::data(int column) const
{
    switch (column) {
        case 0: // 名称
            return m_name;
        case 1: // 大小
            return size();
        case 2: // 修改时间
            return modifiedTime();
        default:
            return QVariant();
    }
}

bool TreeNode::setData(int column, const QVariant &value)
{
    if (column != 0)
        return false;
    
    m_name = value.toString();
    return true;
} 