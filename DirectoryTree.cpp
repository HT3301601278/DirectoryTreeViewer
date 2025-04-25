#include "DirectoryTree.h"
#include <QFileInfo>

DirectoryTree::DirectoryTree()
    : maxDepth(-1), indentChars("    "), showFiles(true), showHidden(false)
{
}

void DirectoryTree::setMaxDepth(int depth)
{
    maxDepth = depth;
}

void DirectoryTree::setIndentChars(const QString &chars)
{
    indentChars = chars;
}

void DirectoryTree::setShowFiles(bool show)
{
    showFiles = show;
}

void DirectoryTree::setShowHidden(bool show)
{
    showHidden = show;
}

QString DirectoryTree::generateTree(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    return rootName + "\n" + processDirectory(rootPath, 0);
}

QString DirectoryTree::processDirectory(const QString &path, int depth)
{
    QString result;
    QDir dir(path);
    
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    
    // 设置排序方式（文件夹优先，按名称排序）
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    
    QFileInfoList list = dir.entryInfoList();
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString indent = QString(indentChars).repeated(depth + 1);
        
        // 是否是最后一个项目
        bool isLast = (i == list.size() - 1);
        
        QString prefix = isLast ? "└── " : "├── ";
        
        if (fileInfo.isDir()) {
            result += indent + prefix + fileInfo.fileName() + "\n";
            
            // 为子目录添加正确的缩进前缀
            QString childIndent = indent;
            if (!isLast) {
                childIndent.append("│   ");
            } else {
                childIndent.append("    ");
            }
            
            QString subResult = processDirectory(fileInfo.filePath(), depth + 1);
            if (!subResult.isEmpty()) {
                result += subResult;
            }
        } else if (showFiles) {
            result += indent + prefix + fileInfo.fileName() + "\n";
        }
    }
    
    return result;
} 