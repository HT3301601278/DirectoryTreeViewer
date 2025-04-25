#ifndef DIRECTORYTREE_H
#define DIRECTORYTREE_H

#include <QString>
#include <QDir>
#include <QStringList>
#include <QRegExp>
#include <QSet>
#include <QJsonObject>
#include <QJsonArray>

// 输出格式枚举
enum class OutputFormat {
    PlainText,
    Markdown,
    JSON
};

// 排序方式枚举
enum class SortMode {
    NameAsc,
    NameDesc,
    ModifiedTimeAsc,
    ModifiedTimeDesc,
    FilesFirst,
    DirectoriesFirst
};

class DirectoryTree
{
public:
    DirectoryTree();
    
    // 设置选项
    void setMaxDepth(int depth);
    void setIndentChars(const QString &chars);
    void setShowFiles(bool show);
    void setShowHidden(bool show);
    void setOutputFormat(OutputFormat format);
    void setSortMode(SortMode mode);
    void addIgnorePattern(const QString &pattern);
    void clearIgnorePatterns();
    void setIgnorePatterns(const QStringList &patterns);
    
    // 获取选项
    int getMaxDepth() const { return maxDepth; }
    QString getIndentChars() const { return indentChars; }
    bool getShowFiles() const { return showFiles; }
    bool getShowHidden() const { return showHidden; }
    OutputFormat getOutputFormat() const { return format; }
    SortMode getSortMode() const { return sortMode; }
    QStringList getIgnorePatterns() const { return ignorePatterns; }
    
    // 生成树
    QString generateTree(const QString &rootPath);
    
    // 不同格式的生成方法
    QString generatePlainText(const QString &rootPath);
    QString generateMarkdown(const QString &rootPath);
    QString generateJSON(const QString &rootPath);

private:
    int maxDepth;
    QString indentChars;
    bool showFiles;
    bool showHidden;
    OutputFormat format;
    SortMode sortMode;
    QStringList ignorePatterns;
    
    // 处理目录的辅助方法
    QString processDirectoryPlainText(const QString &path, int depth, bool isLast = false, const QString &prefix = "");
    QString processDirectoryMarkdown(const QString &path, int depth);
    QJsonArray processDirectoryJSON(const QString &path, int depth);
    
    // 辅助方法
    bool shouldIgnore(const QString &name) const;
    QFileInfoList getSortedEntries(QDir dir) const;
};

#endif // DIRECTORYTREE_H 