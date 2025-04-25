#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QCloseEvent>
#include <QMenu>
#include <QSettings>
#include <QStandardItemModel>
#include <QPrinter>
#include <QPrintDialog>
#include <QMenuBar>
#include <QStatusBar>

#include "DirectoryScanner.h"
#include "TreeModel.h"
#include "DirectoryTree.h"

// 为了简化代码，使用别名
using ScanResult = struct ScanResult;

// 存储历史记录的结构
struct HistoryItem {
    QString path;
    QString content;
    QDateTime timestamp;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void browseDirectory();
    void openDirectory();
    void saveResults();
    void printResults();
    void processDirectoryAsync(const QString &path);
    void cancelScan();
    void toggleView();
    void copyToClipboard();
    void exportToFile();
    void showOptionsDialog();
    void showAboutDialog();
    void expandAll();
    void collapseAll();
    void updateTextOutput();
    void selectAll();
    void showFindDialog();
    void changeFont();
    void toggleTheme();
    void filterTree();
    void showTreeContextMenu(const QPoint &point);
    void loadFromHistory(int index);
    void saveToHistory();
    void showHistoryMenu();

    // 扫描器槽函数
    void onScanStarted(const QString &path);
    void onScanProgress(int filesScanned, int dirsScanned);
    void onScanFinished(const ScanResult &result);
    void onScanCancelled();
    void onScanError(const QString &errorMessage);

private:
    void setupUI();
    void createActions();
    void createMenus();
    void createStatusBar();
    void loadSettings();
    void saveSettings();
    void updateRecentFilesMenu();
    void updateHistoryMenu();
    void updateTreeView();
    void showScanProgress(bool show);
    void showDropArea(bool show);
    void applyTheme(bool dark);
    void handleDroppedPaths(const QStringList &paths);
    void processPath(const QString &path);
    
    // UI组件
    QWidget *leftPanel;
    QWidget *rightPanel;
    QSplitter *mainSplitter;
    QTreeView *treeView;
    QTextEdit *treeTextEdit;
    QProgressBar *scanProgressBar;
    QLabel *scanStatusLabel;
    QLabel *dropAreaLabel;
    QPushButton *cancelButton;
    QPushButton *viewModeButton;
    QPushButton *copyButton;
    QPushButton *exportButton;
    QLineEdit *filterEdit;
    QComboBox *formatComboBox;
    QLabel *itemCountLabel;
    QLabel *sizeLabel;
    QLabel *timeLabel;
    
    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    QMenu *recentMenu;
    
    // 动作
    QAction *openAction;
    QAction *saveAction;
    QAction *printAction;
    QAction *exportAction;
    QAction *exitAction;
    QAction *copyAction;
    QAction *selectAllAction;
    QAction *findAction;
    QAction *toggleViewAction;
    QAction *expandAllAction;
    QAction *collapseAllAction;
    QAction *optionsAction;
    QAction *changeFontAction;
    QAction *toggleThemeAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    
    // 数据
    DirectoryScanner scanner;
    DirectoryTree directoryTree;
    TreeModel *treeModel;
    QStringList lastHandledPaths;
    
    // 历史记录
    QList<HistoryItem> historyItems;
    int maxHistoryItems;
    
    // 状态
    bool isDarkTheme;
    bool isTreeViewMode;
};

#endif // MAINWINDOW_H 