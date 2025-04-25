#include "DirectoryTree.h"
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

DirectoryTree::DirectoryTree()
    : maxDepth(-1), indentChars("    "), showFiles(true), showHidden(false),
      format(OutputFormat::PlainText), sortMode(SortMode::DirectoriesFirst)
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

void DirectoryTree::setOutputFormat(OutputFormat format)
{
    this->format = format;
}

void DirectoryTree::setSortMode(SortMode mode)
{
    this->sortMode = mode;
}

void DirectoryTree::addIgnorePattern(const QString &pattern)
{
    if (!ignorePatterns.contains(pattern)) {
        ignorePatterns.append(pattern);
    }
}

void DirectoryTree::clearIgnorePatterns()
{
    ignorePatterns.clear();
}

void DirectoryTree::setIgnorePatterns(const QStringList &patterns)
{
    ignorePatterns = patterns;
}

QString DirectoryTree::generateTree(const QString &rootPath)
{
    switch (format) {
        case OutputFormat::Markdown:
            return generateMarkdown(rootPath);
        case OutputFormat::JSON:
            return generateJSON(rootPath);
        case OutputFormat::PlainText:
        default:
            return generatePlainText(rootPath);
    }
}

QString DirectoryTree::generatePlainText(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    return rootName + "\n" + processDirectoryPlainText(rootPath, 0);
}

QString DirectoryTree::generateMarkdown(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    return "# " + rootName + "\n\n" + processDirectoryMarkdown(rootPath, 0);
}

QString DirectoryTree::generateJSON(const QString &rootPath)
{
    QFileInfo rootInfo(rootPath);
    QString rootName = rootInfo.fileName();
    
    if (rootName.isEmpty()) {
        rootName = rootPath;
    }
    
    QJsonObject rootObject;
    rootObject["name"] = rootName;
    rootObject["type"] = "directory";
    rootObject["path"] = rootPath;
    rootObject["children"] = processDirectoryJSON(rootPath, 0);
    
    QJsonDocument doc(rootObject);
    return doc.toJson();
}

bool DirectoryTree::shouldIgnore(const QString &name) const
{
    for (const QString &pattern : ignorePatterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(name)) {
            return true;
        }
    }
    return false;
}

QFileInfoList DirectoryTree::getSortedEntries(QDir dir) const
{
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    
    // 设置排序方式
    QDir::SortFlags sortFlags;
    
    switch (sortMode) {
        case SortMode::NameAsc:
            sortFlags = QDir::Name;
            break;
        case SortMode::NameDesc:
            sortFlags = QDir::Name | QDir::Reversed;
            break;
        case SortMode::ModifiedTimeAsc:
            sortFlags = QDir::Time;
            break;
        case SortMode::ModifiedTimeDesc:
            sortFlags = QDir::Time | QDir::Reversed;
            break;
        case SortMode::FilesFirst:
            sortFlags = QDir::Type;
            break;
        case SortMode::DirectoriesFirst:
        default:
            sortFlags = QDir::DirsFirst | QDir::Name;
            break;
    }
    
    dir.setSorting(sortFlags);
    
    QFileInfoList entries = dir.entryInfoList();
    QFileInfoList filteredEntries;
    
    // 应用忽略规则
    for (const QFileInfo &info : entries) {
        if (!shouldIgnore(info.fileName())) {
            filteredEntries.append(info);
        }
    }
    
    return filteredEntries;
}

QString DirectoryTree::processDirectoryPlainText(const QString &path, int depth, bool isLast, const QString &prefix)
{
    QString result;
    QDir dir(path);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    QFileInfoList list = getSortedEntries(dir);
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        bool currentIsLast = (i == list.size() - 1);
        
        QString line = prefix;
        
        // 为当前项添加适当的前缀
        if (depth > 0) {
            line += currentIsLast ? "└── " : "├── ";
        }
        
        line += fileInfo.fileName();
        
        if (fileInfo.isDir()) {
            result += line + "\n";
            
            // 计算子目录的前缀
            QString newPrefix = prefix;
            if (depth > 0) {
                newPrefix += currentIsLast ? "    " : "│   ";
            }
            
            result += processDirectoryPlainText(fileInfo.filePath(), depth + 1, currentIsLast, newPrefix);
        } else if (showFiles) {
            result += line + "\n";
        }
    }
    
    return result;
}

QString DirectoryTree::processDirectoryMarkdown(const QString &path, int depth)
{
    QString result;
    QDir dir(path);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    QFileInfoList list = getSortedEntries(dir);
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString indent = QString(indentChars).repeated(depth);
        
        if (fileInfo.isDir()) {
            result += indent + "- **" + fileInfo.fileName() + "/**\n";
            result += processDirectoryMarkdown(fileInfo.filePath(), depth + 1);
        } else if (showFiles) {
            result += indent + "- " + fileInfo.fileName() + "\n";
        }
    }
    
    return result;
}

QJsonArray DirectoryTree::processDirectoryJSON(const QString &path, int depth)
{
    QJsonArray children;
    QDir dir(path);
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return children;
    }
    
    QFileInfoList list = getSortedEntries(dir);
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        
        QJsonObject item;
        item["name"] = fileInfo.fileName();
        item["path"] = fileInfo.filePath();
        
        if (fileInfo.isDir()) {
            item["type"] = "directory";
            item["children"] = processDirectoryJSON(fileInfo.filePath(), depth + 1);
            children.append(item);
        } else if (showFiles) {
            item["type"] = "file";
            item["size"] = fileInfo.size();
            item["modified"] = fileInfo.lastModified().toString(Qt::ISODate);
            children.append(item);
        }
    }
    
    return children;
} 