#include "DirectoryTree.h"
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDateTime>

DirectoryTree::DirectoryTree()
    : maxDepth(-1), indentChars("    "), showFiles(true), showHidden(false),
      sortType(SortType::DIRS_FIRST), outputFormat(OutputFormat::TEXT),
      totalItems(0), processedItems(0)
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

void DirectoryTree::setIgnorePatterns(const QStringList &patterns)
{
    ignorePatterns = patterns;
    ignoredDirs.clear();
    for (const QString &pattern : patterns) {
        if (!pattern.contains('*') && !pattern.contains('?')) {
            ignoredDirs.insert(pattern);
        }
    }
}

void DirectoryTree::setSortType(SortType type)
{
    sortType = type;
}

void DirectoryTree::setOutputFormat(OutputFormat format)
{
    outputFormat = format;
}

QString DirectoryTree::generateTree(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    totalItems = 0;
    processedItems = 0;
    
    // 预先计算总项目数
    QDir dir(rootPath);
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    totalItems = dir.count();
    
    // 根据输出格式选择相应的处理函数
    switch (outputFormat) {
        case OutputFormat::MARKDOWN:
            return rootName + "\n" + processDirectoryMarkdown(rootPath, 0);
        case OutputFormat::TEXT:
        default:
            return rootName + "\n" + processDirectory(rootPath, 0);
    }
}

QJsonObject DirectoryTree::generateJsonTree(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    totalItems = 0;
    processedItems = 0;
    
    // 预先计算总项目数
    QDir dir(rootPath);
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    totalItems = dir.count();
    
    QJsonObject root;
    root["name"] = rootName;
    root["path"] = rootPath;
    root["type"] = "directory";
    root["children"] = processDirectoryJson(rootPath, 0);
    
    return root;
}

bool DirectoryTree::shouldIgnore(const QString &name) const
{
    if (ignoredDirs.contains(name)) {
        return true;
    }
    
    for (const QString &pattern : ignorePatterns) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
        if (regex.match(name).hasMatch()) {
            return true;
        }
    }
    
    return false;
}

QFileInfoList DirectoryTree::getSortedEntries(const QDir &dir) const
{
    QFileInfoList list = dir.entryInfoList();
    
    switch (sortType) {
        case SortType::NAME:
            std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
                return a.fileName().toLower() < b.fileName().toLower();
            });
            break;
            
        case SortType::MODIFIED_TIME:
            std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
                return a.lastModified() > b.lastModified();
            });
            break;
            
        case SortType::FILES_FIRST:
            std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
                if (a.isFile() && b.isDir()) return true;
                if (a.isDir() && b.isFile()) return false;
                return a.fileName().toLower() < b.fileName().toLower();
            });
            break;
            
        case SortType::DIRS_FIRST:
            std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
                if (a.isDir() && b.isFile()) return true;
                if (a.isFile() && b.isDir()) return false;
                return a.fileName().toLower() < b.fileName().toLower();
            });
            break;
    }
    
    return list;
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
    
    // 获取排序后的文件列表
    QFileInfoList list = getSortedEntries(dir);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        processedItems++;
        
        // 检查是否应该忽略
        if (shouldIgnore(fileInfo.fileName())) {
            continue;
        }
        
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

QString DirectoryTree::processDirectoryMarkdown(const QString &path, int depth)
{
    QString result;
    QDir dir(path);
    
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    
    // 获取排序后的文件列表
    QFileInfoList list = getSortedEntries(dir);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        processedItems++;
        
        // 检查是否应该忽略
        if (shouldIgnore(fileInfo.fileName())) {
            continue;
        }
        
        QString indent = QString("  ").repeated(depth + 1);
        
        if (fileInfo.isDir()) {
            result += indent + "- " + fileInfo.fileName() + "\n";
            QString subResult = processDirectoryMarkdown(fileInfo.filePath(), depth + 1);
            if (!subResult.isEmpty()) {
                result += subResult;
            }
        } else if (showFiles) {
            result += indent + "- " + fileInfo.fileName() + "\n";
        }
    }
    
    return result;
}

QJsonArray DirectoryTree::processDirectoryJson(const QString &path, int depth)
{
    QJsonArray result;
    QDir dir(path);
    
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    
    // 获取排序后的文件列表
    QFileInfoList list = getSortedEntries(dir);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        processedItems++;
        
        // 检查是否应该忽略
        if (shouldIgnore(fileInfo.fileName())) {
            continue;
        }
        
        QJsonObject item;
        item["name"] = fileInfo.fileName();
        item["path"] = fileInfo.filePath();
        
        if (fileInfo.isDir()) {
            item["type"] = "directory";
            if (maxDepth <= 0 || depth + 1 < maxDepth) {
                item["children"] = processDirectoryJson(fileInfo.filePath(), depth + 1);
            }
            result.append(item);
        } else if (showFiles) {
            item["type"] = "file";
            result.append(item);
        }
    }
    
    return result;
} 