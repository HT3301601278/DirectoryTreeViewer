#ifndef DIRECTORYTREE_H
#define DIRECTORYTREE_H

#include <QString>
#include <QDir>

class DirectoryTree
{
public:
    DirectoryTree();
    
    void setMaxDepth(int depth);
    void setIndentChars(const QString &chars);
    void setShowFiles(bool show);
    void setShowHidden(bool show);
    
    QString generateTree(const QString &rootPath);

private:
    int maxDepth;
    QString indentChars;
    bool showFiles;
    bool showHidden;
    
    QString processDirectory(const QString &path, int depth);
};

#endif // DIRECTORYTREE_H 