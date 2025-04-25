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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void copyToClipboard();
    void showOptionsDialog();
    void generateTree(const QString &path);

private:
    QWidget *centralWidget;
    QLabel *dropAreaLabel;
    QTextEdit *treeTextEdit;
    QPushButton *copyButton;
    QPushButton *optionsButton;

    int maxDepth = -1;           // 不限制深度
    QString indentChars = "    "; // 默认缩进
    bool showFiles = true;       // 显示文件
    bool showHidden = false;     // 不显示隐藏文件
    
    QString generateDirectoryTree(const QString &path, int depth = 0);
};

#endif // MAINWINDOW_H 