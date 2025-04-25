#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <QToolBar>
#include <QComboBox>
#include <QProgressBar>
#include <QTreeView>
#include <QStandardItemModel>
#include "DirectoryTree.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void copyToClipboard();
    void showOptionsDialog();
    void generateTree(const QString &path);
    void exportToFile();
    void toggleView();
    void switchFormat(int index);

private:
    QWidget *centralWidget;
    QLabel *dropAreaLabel;
    QTextEdit *treeTextEdit;
    QTreeView *treeView;
    QStandardItemModel *treeModel;
    QPushButton *copyButton;
    QPushButton *optionsButton;
    QToolBar *toolBar;
    QComboBox *formatComboBox;
    QProgressBar *progressBar;
    QMenu *exportMenu;
    QPushButton *exportButton;
    QPushButton *toggleViewButton;
    
    DirectoryTree dirTree;
    QString currentPath;
    OutputFormat currentFormat;
    bool isHierarchicalView;
    
    int maxDepth = -1;           // 不限制深度
    QString indentChars = "    "; // 默认缩进
    bool showFiles = true;       // 显示文件
    bool showHidden = false;     // 不显示隐藏文件
    QStringList ignorePatterns;  // 忽略模式
    SortType sortType = SortType::DIRS_FIRST;  // 排序方式
    
    void setupUI();
    void updateDirectoryTree();
    void createTreeViewModel(const QJsonObject &jsonTree);
    void addTreeItem(QStandardItem *parent, const QJsonObject &item);
    void exportToTextFile(const QString &filePath);
    void exportToMarkdownFile(const QString &filePath);
    void exportToJsonFile(const QString &filePath);
    void updateProgressBar(bool visible, int value = 0);
};

#endif // MAINWINDOW_H 