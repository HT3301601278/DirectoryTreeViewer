#ifndef TREENODE_H
#define TREENODE_H

#include <QString>
#include <QVector>
#include <QFileInfo>
#include <QIcon>
#include <QVariant>
#include <memory>

class TreeNode
{
public:
    explicit TreeNode(const QString &name, const QString &path, bool isDir, TreeNode *parent = nullptr);
    ~TreeNode();
    
    // 节点数据访问方法
    QString name() const { return m_name; }
    QString path() const { return m_path; }
    bool isDir() const { return m_isDir; }
    QIcon icon() const;
    QString size() const;
    QString modifiedTime() const;
    
    // 树结构操作
    void appendChild(TreeNode *child);
    TreeNode* child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    TreeNode* parent();
    bool isExpanded() const { return m_isExpanded; }
    void setExpanded(bool expanded) { m_isExpanded = expanded; }
    
    // 自定义数据属性
    QVariant data(int column) const;
    bool setData(int column, const QVariant &value);

private:
    QString m_name;              // 文件或目录名
    QString m_path;              // 完整路径
    bool m_isDir;                // 是否为目录
    QFileInfo m_fileInfo;        // 文件信息
    bool m_isExpanded;           // 是否展开状态
    
    QVector<TreeNode*> m_childItems;  // 子节点
    TreeNode *m_parentItem;           // 父节点
};

#endif // TREENODE_H 