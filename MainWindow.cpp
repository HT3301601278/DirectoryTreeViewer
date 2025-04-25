#include "MainWindow.h"
#include "OptionsDialog.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("目录树查看器");
    setMinimumSize(800, 600);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建拖放区域
    dropAreaLabel = new QLabel("将文件夹拖放到此处或点击选择文件夹", this);
    dropAreaLabel->setAlignment(Qt::AlignCenter);
    dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
    dropAreaLabel->setMinimumHeight(100);
    dropAreaLabel->setAcceptDrops(true);
    mainLayout->addWidget(dropAreaLabel);
    
    // 点击拖放区域选择文件夹
    dropAreaLabel->setMouseTracking(true);
    dropAreaLabel->setCursor(Qt::PointingHandCursor);
    dropAreaLabel->installEventFilter(this);
    
    // 创建文本编辑区
    treeTextEdit = new QTextEdit(this);
    treeTextEdit->setReadOnly(true);
    treeTextEdit->setFont(QFont("Consolas", 10));
    treeTextEdit->setStyleSheet("QTextEdit { border: 1px solid #ccc; border-radius: 3px; }");
    treeTextEdit->setPlaceholderText("这里将显示生成的目录树");
    mainLayout->addWidget(treeTextEdit, 1);
    
    // 创建按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    copyButton = new QPushButton("复制到剪贴板", this);
    copyButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::copyToClipboard);
    
    optionsButton = new QPushButton("选项", this);
    optionsButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    connect(optionsButton, &QPushButton::clicked, this, &MainWindow::showOptionsDialog);
    
    buttonLayout->addWidget(optionsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(copyButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置接受拖放
    setAcceptDrops(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        dropAreaLabel->setStyleSheet("QLabel { background-color: #e0f0e0; border: 2px dashed #5a5; border-radius: 5px; padding: 30px; font-size: 16px; }");
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;
        
    const QString path = urls.first().toLocalFile();
    QFileInfo fileInfo(path);
    
    if (!fileInfo.isDir()) {
        QMessageBox::warning(this, "错误", "请拖放文件夹而不是文件");
        return;
    }
    
    dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
    generateTree(path);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == dropAreaLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            QString dirPath = QFileDialog::getExistingDirectory(this, "选择文件夹");
            if (!dirPath.isEmpty()) {
                generateTree(dirPath);
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::copyToClipboard()
{
    if (treeTextEdit->toPlainText().isEmpty()) {
        QMessageBox::information(this, "提示", "没有内容可复制");
        return;
    }
    
    QApplication::clipboard()->setText(treeTextEdit->toPlainText());
    QMessageBox::information(this, "成功", "目录树已复制到剪贴板");
}

void MainWindow::showOptionsDialog()
{
    OptionsDialog dialog(indentChars, maxDepth, showFiles, showHidden, this);
    if (dialog.exec() == QDialog::Accepted) {
        indentChars = dialog.getIndentChars();
        maxDepth = dialog.getMaxDepth();
        showFiles = dialog.getShowFiles();
        showHidden = dialog.getShowHidden();
        
        // 如果已经有目录，重新生成树
        if (!treeTextEdit->toPlainText().isEmpty()) {
            QString path = treeTextEdit->property("currentPath").toString();
            if (!path.isEmpty()) {
                generateTree(path);
            }
        }
    }
}

void MainWindow::generateTree(const QString &path)
{
    treeTextEdit->clear();
    
    QFileInfo fileInfo(path);
    QString rootName = fileInfo.fileName();
    QString result = rootName + "\n" + generateDirectoryTree(path);
    
    treeTextEdit->setText(result);
    treeTextEdit->setProperty("currentPath", path);
}

QString MainWindow::generateDirectoryTree(const QString &path, int depth)
{
    QString result;
    QDir dir(path);
    
    if (!showHidden) {
        dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks);
    } else {
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllEntries | QDir::NoSymLinks);
    }
    
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    
    QFileInfoList list = dir.entryInfoList();
    
    // 检查深度限制
    if (maxDepth > 0 && depth >= maxDepth) {
        return result;
    }
    
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString indent = QString(indentChars).repeated(depth + 1);
        
        // 是否最后一个项目
        bool isLast = (i == list.size() - 1);
        
        if (fileInfo.isDir()) {
            result += indent + "├── " + fileInfo.fileName() + "\n";
            QString subResult = generateDirectoryTree(fileInfo.filePath(), depth + 1);
            if (!subResult.isEmpty()) {
                result += subResult;
            }
        } else if (showFiles) {
            result += indent + "├── " + fileInfo.fileName() + "\n";
        }
    }
    
    return result;
} 