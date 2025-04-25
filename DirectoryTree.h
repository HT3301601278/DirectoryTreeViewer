#ifndef DIRECTORYTREE_H
#define DIRECTORYTREE_H

#include <QString>
#include <QDir>
#include <QSet>
#include <QJsonObject>
#include <QJsonArray>

enum class OutputFormat {
    TEXT,
    MARKDOWN,
    JSON
};

enum class SortType {
    NAME,
    MODIFIED_TIME,
    FILES_FIRST,
    DIRS_FIRST
};

class DirectoryTree
{
public:
    DirectoryTree();
    
    void setMaxDepth(int depth);
    void setIndentChars(const QString &chars);
    void setShowFiles(bool show);
    void setShowHidden(bool show);
    void setIgnorePatterns(const QStringList &patterns);
    void setSortType(SortType type);
    void setOutputFormat(OutputFormat format);
    
    QString generateTree(const QString &rootPath);
    QJsonObject generateJsonTree(const QString &rootPath);
    int getTotalItems() const { return totalItems; }
    int getProcessedItems() const { return processedItems; }

private:
    int maxDepth;
    QString indentChars;
    bool showFiles;
    bool showHidden;
    QStringList ignorePatterns;
    QSet<QString> ignoredDirs;
    SortType sortType;
    OutputFormat outputFormat;
    int totalItems;
    int processedItems;
    
    QString processDirectory(const QString &path, int depth);
    QString processDirectoryMarkdown(const QString &path, int depth);
    QJsonArray processDirectoryJson(const QString &path, int depth);
    bool shouldIgnore(const QString &name) const;
    QFileInfoList getSortedEntries(const QDir &dir) const;
};

#endif // DIRECTORYTREE_H 