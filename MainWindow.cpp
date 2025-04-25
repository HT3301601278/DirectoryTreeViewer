#include "MainWindow.h"
#include "OptionsDialog.h"
#include "ExportDialog.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>
#include <QHeaderView>
#include <QSplitter>
#include <QScrollBar>
#include <QInputDialog>
#include <QFontDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QStyleFactory>
#include <QShortcut>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      treeModel(nullptr),
      maxHistoryItems(10),
      isDarkTheme(false),
      isTreeViewMode(true)
{
    setWindowTitle(tr("目录树查看器"));
    setAcceptDrops(true);
    resize(1024, 768);

    // 初始化UI
    setupUI();
    createActions();
    createMenus();
    createStatusBar();
    
    // 连接扫描器信号
    connect(&scanner, &DirectoryScanner::scanStarted, this, &MainWindow::onScanStarted);
    connect(&scanner, &DirectoryScanner::scanProgress, this, &MainWindow::onScanProgress);
    connect(&scanner, &DirectoryScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(&scanner, &DirectoryScanner::scanCancelled, this, &MainWindow::onScanCancelled);
    connect(&scanner, &DirectoryScanner::scanError, this, &MainWindow::onScanError);

    // 加载设置
    loadSettings();
    
    // 默认显示拖放区域
    showDropArea(true);
    showScanProgress(false);
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (treeModel) {
        delete treeModel;
    }
}

void MainWindow::setupUI()
{
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    // 左侧面板
    leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // 树视图
    treeView = new QTreeView();
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->setAlternatingRowColors(true);
    treeView->setAnimated(true);
    treeView->setUniformRowHeights(true);
    treeView->setSortingEnabled(true);
    treeView->header()->setSortIndicator(0, Qt::AscendingOrder);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    // 过滤框
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterEdit = new QLineEdit();
    filterEdit->setPlaceholderText(tr("输入过滤条件..."));
    QPushButton *filterButton = new QPushButton(tr("过滤"));
    
    filterLayout->addWidget(filterEdit);
    filterLayout->addWidget(filterButton);
    
    // 拖放区域
    dropAreaLabel = new QLabel(tr("将文件夹拖放到此处或点击浏览按钮"));
    dropAreaLabel->setAlignment(Qt::AlignCenter);
    dropAreaLabel->setMinimumHeight(200);
    dropAreaLabel->setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 5px; background-color: #f8f8f8; }");
    dropAreaLabel->installEventFilter(this);
    
    // 进度条
    scanProgressBar = new QProgressBar();
    scanProgressBar->setRange(0, 0);
    scanProgressBar->setTextVisible(true);
    
    // 状态标签
    scanStatusLabel = new QLabel(tr("准备就绪"));
    
    // 取消按钮
    cancelButton = new QPushButton(tr("取消"));
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::cancelScan);
    
    // 添加到左侧布局
    leftLayout->addLayout(filterLayout);
    leftLayout->addWidget(treeView);
    leftLayout->addWidget(dropAreaLabel);
    leftLayout->addWidget(scanProgressBar);
    leftLayout->addWidget(scanStatusLabel);
    leftLayout->addWidget(cancelButton, 0, Qt::AlignRight);
    
    // 右侧面板
    rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    // 文本输出区
    treeTextEdit = new QTextEdit();
    treeTextEdit->setReadOnly(true);
    treeTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    treeTextEdit->setFont(QFont("Courier New", 10));
    
    // 工具栏
    QHBoxLayout *toolBarLayout = new QHBoxLayout();
    
    // 格式选择
    formatComboBox = new QComboBox();
    formatComboBox->addItem(tr("纯文本"));
    formatComboBox->addItem(tr("Markdown"));
    formatComboBox->addItem(tr("HTML"));
    formatComboBox->addItem(tr("XML"));
    formatComboBox->addItem(tr("JSON"));
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::updateTextOutput);
    
    // 复制按钮
    copyButton = new QPushButton(tr("复制"));
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::copyToClipboard);
    
    // 导出按钮
    exportButton = new QPushButton(tr("导出"));
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToFile);
    
    // 视图模式切换按钮
    viewModeButton = new QPushButton(tr("切换到列表视图"));
    connect(viewModeButton, &QPushButton::clicked, this, &MainWindow::toggleView);
    
    // 添加到工具栏布局
    toolBarLayout->addWidget(new QLabel(tr("输出格式:")));
    toolBarLayout->addWidget(formatComboBox);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(copyButton);
    toolBarLayout->addWidget(exportButton);
    toolBarLayout->addWidget(viewModeButton);
    
    // 添加到右侧布局
    rightLayout->addLayout(toolBarLayout);
    rightLayout->addWidget(treeTextEdit);
    
    // 添加到主分割器
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
    
    // 连接信号
    connect(filterButton, &QPushButton::clicked, this, &MainWindow::filterTree);
    connect(filterEdit, &QLineEdit::returnPressed, this, &MainWindow::filterTree);
    connect(treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);
}

// 创建动作
void MainWindow::createActions()
{
    // 文件菜单动作
    openAction = new QAction(tr("打开目录"), this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("打开一个目录进行扫描"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openDirectory);
    
    saveAction = new QAction(tr("保存结果"), this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip(tr("保存当前扫描结果"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveResults);
    
    printAction = new QAction(tr("打印"), this);
    printAction->setShortcut(QKeySequence::Print);
    printAction->setStatusTip(tr("打印当前结果"));
    connect(printAction, &QAction::triggered, this, &MainWindow::printResults);
    
    exportAction = new QAction(tr("导出"), this);
    exportAction->setStatusTip(tr("导出扫描结果到文件"));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportToFile);
    
    exitAction = new QAction(tr("退出"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip(tr("退出应用程序"));
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    // 编辑菜单动作
    copyAction = new QAction(tr("复制"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setStatusTip(tr("复制选中内容到剪贴板"));
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyToClipboard);
    
    selectAllAction = new QAction(tr("全选"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setStatusTip(tr("选择所有内容"));
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
    
    findAction = new QAction(tr("查找"), this);
    findAction->setShortcut(QKeySequence::Find);
    findAction->setStatusTip(tr("在当前视图中查找"));
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindDialog);
    
    // 视图菜单动作
    toggleViewAction = new QAction(tr("切换视图模式"), this);
    toggleViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    toggleViewAction->setStatusTip(tr("在树视图和文本视图之间切换"));
    connect(toggleViewAction, &QAction::triggered, this, &MainWindow::toggleView);
    
    expandAllAction = new QAction(tr("展开全部"), this);
    expandAllAction->setStatusTip(tr("展开树视图中的所有节点"));
    connect(expandAllAction, &QAction::triggered, this, &MainWindow::expandAll);
    
    collapseAllAction = new QAction(tr("折叠全部"), this);
    collapseAllAction->setStatusTip(tr("折叠树视图中的所有节点"));
    connect(collapseAllAction, &QAction::triggered, this, &MainWindow::collapseAll);
    
    // 工具菜单动作
    optionsAction = new QAction(tr("选项"), this);
    optionsAction->setStatusTip(tr("配置应用程序设置"));
    connect(optionsAction, &QAction::triggered, this, &MainWindow::showOptionsDialog);
    
    changeFontAction = new QAction(tr("更改字体"), this);
    changeFontAction->setStatusTip(tr("更改文本显示字体"));
    connect(changeFontAction, &QAction::triggered, this, &MainWindow::changeFont);
    
    toggleThemeAction = new QAction(tr("切换主题"), this);
    toggleThemeAction->setStatusTip(tr("在亮色和暗色主题之间切换"));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
    
    // 帮助菜单动作
    aboutAction = new QAction(tr("关于"), this);
    aboutAction->setStatusTip(tr("显示关于信息"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    
    aboutQtAction = new QAction(tr("关于 Qt"), this);
    aboutQtAction->setStatusTip(tr("显示关于 Qt 的信息"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

// 创建菜单
void MainWindow::createMenus()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(printAction);
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    
    // 最近打开的文件子菜单
    recentMenu = fileMenu->addMenu(tr("最近打开"));
    updateRecentFilesMenu();
    
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 编辑菜单
    editMenu = menuBar()->addMenu(tr("编辑"));
    editMenu->addAction(copyAction);
    editMenu->addAction(selectAllAction);
    editMenu->addSeparator();
    editMenu->addAction(findAction);
    
    // 视图菜单
    viewMenu = menuBar()->addMenu(tr("视图"));
    viewMenu->addAction(toggleViewAction);
    viewMenu->addSeparator();
    viewMenu->addAction(expandAllAction);
    viewMenu->addAction(collapseAllAction);
    
    // 工具菜单
    toolsMenu = menuBar()->addMenu(tr("工具"));
    toolsMenu->addAction(optionsAction);
    toolsMenu->addAction(changeFontAction);
    toolsMenu->addAction(toggleThemeAction);
    
    // 帮助菜单
    helpMenu = menuBar()->addMenu(tr("帮助"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

// 创建状态栏
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("就绪"));
    
    // 添加永久小部件
    itemCountLabel = new QLabel(tr("项目: 0"));
    statusBar()->addPermanentWidget(itemCountLabel);
    
    sizeLabel = new QLabel(tr("大小: 0 字节"));
    statusBar()->addPermanentWidget(sizeLabel);
    
    timeLabel = new QLabel(tr("时间: 0 秒"));
    statusBar()->addPermanentWidget(timeLabel);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        dropAreaLabel->setStyleSheet("QLabel { border: 2px dashed #00a0e9; border-radius: 5px; background-color: #e0f7ff; }");
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    dropAreaLabel->setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 5px; background-color: #f8f8f8; }");
    event->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    dropAreaLabel->setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 5px; background-color: #f8f8f8; }");
    
    if (event->mimeData()->hasUrls()) {
        QStringList paths;
        for (const QUrl &url : event->mimeData()->urls()) {
            // 只接受本地文件
            if (url.isLocalFile()) {
                paths.append(url.toLocalFile());
            }
        }
        
        if (!paths.isEmpty()) {
            handleDroppedPaths(paths);
        }
        
        event->acceptProposedAction();
    }
}

void MainWindow::handleDroppedPaths(const QStringList &paths)
{
    QStringList validDirs;
    
    // 验证所有路径是否为有效目录
    for (const QString &path : paths) {
        QFileInfo fileInfo(path);
        if (fileInfo.isDir() && fileInfo.exists()) {
            validDirs.append(path);
        }
    }
    
    // 如果没有有效目录，显示警告
    if (validDirs.isEmpty()) {
        QMessageBox::warning(this, tr("无效路径"), tr("没有找到有效的目录。请确保拖放的是有效目录。"));
        return;
    }
    
    // 如果有多个有效目录，让用户选择
    QString selectedPath;
    if (validDirs.size() == 1) {
        selectedPath = validDirs.first();
    } else {
        bool ok;
        selectedPath = QInputDialog::getItem(this, tr("选择目录"), 
                                          tr("检测到多个目录，请选择要扫描的目录:"), 
                                          validDirs, 0, false, &ok);
        if (!ok || selectedPath.isEmpty()) {
            return;
        }
    }
    
    // 保存最后处理的路径，处理选定目录
    lastHandledPaths = validDirs;
    processDirectoryAsync(selectedPath);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 处理拖放区域的点击事件
    if (watched == dropAreaLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            browseDirectory();
            return true;
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 如果扫描正在进行，提示用户是否取消
    if (scanner.isScanning()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("确认退出"),
            tr("扫描正在进行中。退出程序将取消当前扫描。\n确定要退出吗？"),
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            scanner.cancelScan();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::browseDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("选择目录"),
                                                         QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        QStringList paths = {directory};
        lastHandledPaths = paths;
        processDirectoryAsync(directory);
    }
}

void MainWindow::cancelScan()
{
    if (scanner.isScanning()) {
        scanner.cancelScan();
        statusBar()->showMessage(tr("扫描已取消"));
    }
}

void MainWindow::updateTreeView()
{
    if (lastHandledPaths.isEmpty())
        return;
        
    // 更新树视图
    treeModel->setRootPath(lastHandledPaths.first());
    treeView->expandToDepth(0); // 默认展开第一级
}

void MainWindow::expandAll()
{
    treeView->expandAll();
}

void MainWindow::collapseAll()
{
    treeView->collapseAll();
}

void MainWindow::filterTree()
{
    QString filterText = filterEdit->text().trimmed();
    
    // 如果过滤文本为空，则显示所有项
    if (filterText.isEmpty()) {
        // 重置过滤器
        treeView->collapseAll();
        treeView->expandToDepth(0);
        return;
    }
    
    // 查找匹配的项
    QModelIndexList matches = treeModel->findItems(filterText, Qt::MatchContains | Qt::MatchRecursive);
    
    // 如果有匹配项，则展开到匹配项
    if (!matches.isEmpty()) {
        for (const QModelIndex &index : matches) {
            // 展开所有父节点
            QModelIndex parentIndex = index.parent();
            while (parentIndex.isValid()) {
                treeView->expand(parentIndex);
                parentIndex = parentIndex.parent();
            }
        }
        
        // 选择第一个匹配项并滚动到可见区域
        treeView->setCurrentIndex(matches.first());
        treeView->scrollTo(matches.first());
    }
}

void MainWindow::showTreeContextMenu(const QPoint &pos)
{
    QModelIndex index = treeView->indexAt(pos);
    if (!index.isValid())
        return;
        
    QMenu contextMenu(this);
    
    // 获取选中的节点
    TreeNode *node = static_cast<TreeNode*>(index.internalPointer());
    
    // 如果是目录，添加展开/折叠选项
    if (node->isDir()) {
        QAction *expandAction = new QAction("展开", &contextMenu);
        connect(expandAction, &QAction::triggered, [this, index]() {
            treeView->expand(index);
        });
        
        QAction *collapseAction = new QAction("折叠", &contextMenu);
        connect(collapseAction, &QAction::triggered, [this, index]() {
            treeView->collapse(index);
        });
        
        contextMenu.addAction(expandAction);
        contextMenu.addAction(collapseAction);
        contextMenu.addSeparator();
    }
    
    // 复制路径选项
    QAction *copyPathAction = new QAction("复制路径", &contextMenu);
    connect(copyPathAction, &QAction::triggered, [node]() {
        QApplication::clipboard()->setText(node->path());
    });
    
    contextMenu.addAction(copyPathAction);
    
    // 在目录中显示选项 (打开资源管理器)
    QAction *showInExplorerAction = new QAction("在资源管理器中显示", &contextMenu);
    connect(showInExplorerAction, &QAction::triggered, [node]() {
        QString path = node->path();
        QStringList args;
        if (!node->isDir()) {
            args << "/select,";
        }
        args << QDir::toNativeSeparators(path);
        QProcess::startDetached("explorer", args);
    });
    
    contextMenu.addAction(showInExplorerAction);
    
    // 显示上下文菜单
    contextMenu.exec(treeView->viewport()->mapToGlobal(pos));
}

void MainWindow::updateTextOutput()
{
    if (lastHandledPaths.isEmpty())
        return;
    
    // 获取选中的输出格式
    OutputFormat format = static_cast<OutputFormat>(formatComboBox->currentData().toInt());
    directoryTree.setOutputFormat(format);
    
    // 生成目录树文本
    QString result = directoryTree.generateTree(lastHandledPaths.first());
    treeTextEdit->setText(result);
    
    // 保存到历史记录
    saveToHistory();
}

void MainWindow::copyToClipboard()
{
    if (treeTextEdit->toPlainText().isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("没有内容可复制"));
        return;
    }
    
    QApplication::clipboard()->setText(treeTextEdit->toPlainText());
    statusBar()->showMessage(tr("目录树已复制到剪贴板"), 3000);
}

void MainWindow::exportToFile()
{
    ExportDialog dialog(this);
    
    // 使用getOutputFormat()而不是setOutputFormat
    OutputFormat currentFormat = directoryTree.getOutputFormat();
    
    if (dialog.exec() == QDialog::Accepted) {
        QString filePath = dialog.getFilePath();
        OutputFormat format = dialog.getOutputFormat();
        
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // 获取当前选中目录路径
            QString currentPath = lastHandledPaths.isEmpty() ? "" : lastHandledPaths.first();
            
            // 根据选择的格式生成不同内容
            directoryTree.setOutputFormat(format);
            QString content = directoryTree.generateTree(currentPath);
            
            // 写入文件
            QTextStream stream(&file);
            stream << content;
            file.close();
            
            statusBar()->showMessage(tr("目录树已导出到: ") + filePath, 3000);
        } else {
            QMessageBox::critical(this, tr("错误"), tr("无法写入文件: ") + file.errorString());
        }
        
        // 恢复原来的格式
        directoryTree.setOutputFormat(currentFormat);
    }
}

void MainWindow::toggleView()
{
    isTreeViewMode = !isTreeViewMode;
    
    if (isTreeViewMode) {
        // 切换到树视图模式
        viewModeButton->setText(tr("切换到列表视图"));
        leftPanel->show();
    } else {
        // 切换到列表视图
        viewModeButton->setText(tr("切换到树视图模式"));
        leftPanel->hide();
    }
}

void MainWindow::onScanStarted(const QString &path)
{
    statusBar()->showMessage(tr("开始扫描: ") + path);
    scanStatusLabel->setText(tr("扫描中..."));
    scanProgressBar->setRange(0, 0); // 不确定进度模式
    showScanProgress(true);
}

void MainWindow::onScanProgress(int filesScanned, int dirsScanned)
{
    scanProgressBar->setValue(filesScanned);
    scanStatusLabel->setText(QString(tr("已扫描: %1 个文件，%2 个文件夹")).arg(filesScanned).arg(dirsScanned));
    statusBar()->showMessage(QString(tr("扫描中: %1 文件, %2 文件夹")).arg(filesScanned).arg(dirsScanned));
}

void MainWindow::onScanFinished(const ScanResult &result)
{
    showScanProgress(false);
    showDropArea(true);
    
    if (!result.success) {
        QMessageBox::warning(this, tr("扫描错误"), tr("扫描目录时出错: ") + result.errorMessage);
        statusBar()->showMessage(tr("扫描失败: ") + result.errorMessage);
        return;
    }
    
    if (result.cancelled) {
        statusBar()->showMessage(tr("扫描已取消"));
        return;
    }
    
    QString scanSummary = QString(tr("扫描完成: %1 个文件, %2 个文件夹")).arg(result.fileCount).arg(result.dirCount);
    scanStatusLabel->setText(scanSummary);
    statusBar()->showMessage(scanSummary);
    
    // 更新树视图
    updateTreeView();
    
    // 更新文本输出
    updateTextOutput();
    
    // 显示扫描完成通知
    QMessageBox::information(this, tr("扫描完成"), scanSummary);
}

void MainWindow::onScanCancelled()
{
    showScanProgress(false);
    showDropArea(true);
    statusBar()->showMessage(tr("扫描已取消"));
}

void MainWindow::onScanError(const QString &errorMessage)
{
    showScanProgress(false);
    showDropArea(true);
    QMessageBox::warning(this, tr("扫描错误"), tr("扫描目录时出错: ") + errorMessage);
    statusBar()->showMessage(tr("扫描错误: ") + errorMessage);
}

void MainWindow::showOptionsDialog()
{
    OptionsDialog dialog(this);
    
    // 使用getter方法而非直接访问私有成员
    dialog.setIndentChars(directoryTree.getIndentChars());
    dialog.setMaxDepth(directoryTree.getMaxDepth());
    dialog.setShowFiles(directoryTree.getShowFiles());
    dialog.setShowHidden(directoryTree.getShowHidden());
    dialog.setIgnorePatterns(directoryTree.getIgnorePatterns());
    dialog.setSortMode(directoryTree.getSortMode());
    dialog.setOutputFormat(directoryTree.getOutputFormat());
    dialog.setDarkTheme(isDarkTheme);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 获取新选项
        directoryTree.setIndentChars(dialog.getIndentChars());
        directoryTree.setMaxDepth(dialog.getMaxDepth());
        directoryTree.setShowFiles(dialog.getShowFiles());
        directoryTree.setShowHidden(dialog.getShowHidden());
        directoryTree.setIgnorePatterns(dialog.getIgnorePatterns());
        directoryTree.setSortMode(dialog.getSortMode());
        directoryTree.setOutputFormat(dialog.getOutputFormat());
        
        // 更新主题
        bool newDarkTheme = dialog.getDarkTheme();
        if (newDarkTheme != isDarkTheme) {
            applyTheme(newDarkTheme);
        }
        
        // 更新TreeModel设置
        if (treeModel) {
            treeModel->setShowHiddenFiles(directoryTree.getShowHidden());
            treeModel->setIgnorePatterns(directoryTree.getIgnorePatterns());
            treeModel->refresh();
        }
        
        // 更新文本输出
        updateTextOutput();
    }
}

void MainWindow::applyTheme(bool dark)
{
    isDarkTheme = dark;
    toggleThemeAction->setChecked(dark);
    
    if (dark) {
        // 设置深色主题
        qApp->setStyle(QStyleFactory::create("Fusion"));
        
        QPalette darkPalette;
        QColor darkColor = QColor(45, 45, 45);
        QColor disabledColor = QColor(127, 127, 127);
        
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        
        qApp->setPalette(darkPalette);
        
        // 设置拖放区域的样式
        dropAreaLabel->setStyleSheet("QLabel { background-color: #2d2d2d; border: 2px dashed #555; border-radius: 5px; padding: 30px; font-size: 16px; color: white; }");
    } else {
        // 设置浅色主题
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(QApplication::style()->standardPalette());
        
        // 设置拖放区域的样式
        dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
    }
}

void MainWindow::saveToHistory()
{
    if (lastHandledPaths.isEmpty()) {
        return;
    }
    
    QString path = lastHandledPaths.first();
    QString content = treeTextEdit->toPlainText();
    
    // 检查是否为空
    if (path.isEmpty() || content.isEmpty()) {
        return;
    }
    
    // 检查是否已存在相同路径的历史记录
    for (int i = 0; i < historyItems.size(); ++i) {
        if (historyItems[i].path == path) {
            // 已经存在，更新并移动到列表开头
            historyItems[i].content = content;
            historyItems[i].timestamp = QDateTime::currentDateTime();
            
            // 移动到第一位
            historyItems.move(i, 0);
            updateHistoryMenu();
            return;
        }
    }
    
    // 新建历史项
    HistoryItem item;
    item.path = path;
    item.content = content;
    item.timestamp = QDateTime::currentDateTime();
    
    // 添加到历史列表开头
    historyItems.prepend(item);
    
    // 如果超出最大数量，移除旧的
    if (historyItems.size() > maxHistoryItems) {
        historyItems.removeLast();
    }
    
    // 更新历史菜单
    updateHistoryMenu();
}

void MainWindow::showHistoryMenu()
{
    QMenu historyMenu(this);
    
    if (historyItems.isEmpty()) {
        QAction *noHistoryAction = historyMenu.addAction(tr("没有历史记录"));
        noHistoryAction->setEnabled(false);
    } else {
        for (int i = 0; i < historyItems.size(); ++i) {
            const HistoryItem &item = historyItems[i];
            QString name = QFileInfo(item.path).fileName();
            QString dateStr = item.timestamp.toString("yyyy-MM-dd hh:mm");
            
            QAction *action = historyMenu.addAction(
                QString("%1. %2 (%3)").arg(i+1).arg(name).arg(dateStr)
            );
            
            connect(action, &QAction::triggered, [this, i]() {
                loadFromHistory(i);
            });
        }
        
        historyMenu.addSeparator();
        QAction *clearAction = historyMenu.addAction(tr("清除历史记录"));
        connect(clearAction, &QAction::triggered, [this]() {
            historyItems.clear();
            updateHistoryMenu();
        });
    }
    
    historyMenu.exec(QCursor::pos());
}

void MainWindow::loadFromHistory(int index)
{
    if (index < 0 || index >= historyItems.size())
        return;
        
    const HistoryItem &item = historyItems[index];
    
    // 检查路径是否仍然存在
    if (!QFileInfo(item.path).exists()) {
        QMessageBox::warning(this, tr("错误"), tr("历史记录中的路径不存在: ") + item.path);
        return;
    }
    
    // 设置当前路径
    lastHandledPaths.clear();
    lastHandledPaths.append(item.path);
    
    // 加载内容
    treeTextEdit->setText(item.content);
    
    // 更新树视图
    updateTreeView();
    
    statusBar()->showMessage(tr("已加载历史记录: ") + QFileInfo(item.path).fileName());
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("关于目录树查看器"),
        tr("<h3>目录树查看器</h3>")
        + tr("<p>版本 1.0</p>")
        + tr("<p>一个强大的工具，用于生成和导出目录树结构。</p>")
        + tr("<p>支持多种输出格式、排序选项和自定义设置。</p>")
        + tr("<p>&copy; 2023 目录树查看器项目</p>"));
}

void MainWindow::showDropArea(bool show)
{
    dropAreaLabel->setVisible(show);
}

void MainWindow::showScanProgress(bool show)
{
    scanProgressBar->setVisible(show);
    scanStatusLabel->setVisible(show);
    cancelButton->setVisible(show);
}

void MainWindow::loadSettings()
{
    QSettings settings("DirectoryTreeViewer", "Settings");
    
    // 常规设置
    isDarkTheme = settings.value("UI/DarkTheme", false).toBool();
    applyTheme(isDarkTheme);
    
    // DirectoryTree设置
    directoryTree.setMaxDepth(settings.value("Tree/MaxDepth", -1).toInt());
    directoryTree.setIndentChars(settings.value("Tree/IndentChars", "    ").toString());
    directoryTree.setShowFiles(settings.value("Tree/ShowFiles", true).toBool());
    directoryTree.setShowHidden(settings.value("Tree/ShowHidden", false).toBool());
    directoryTree.setOutputFormat(static_cast<OutputFormat>(settings.value("Tree/OutputFormat", 0).toInt()));
    directoryTree.setSortMode(static_cast<SortMode>(settings.value("Tree/SortMode", 5).toInt()));
    
    // 忽略模式
    QStringList ignorePatterns = settings.value("Tree/IgnorePatterns").toStringList();
    if (ignorePatterns.isEmpty()) {
        // 默认忽略模式
        ignorePatterns << ".git" << "node_modules" << ".idea" << "__pycache__";
    }
    directoryTree.setIgnorePatterns(ignorePatterns);
    
    // 窗口设置
    if (settings.contains("UI/WindowGeometry")) {
        restoreGeometry(settings.value("UI/WindowGeometry").toByteArray());
    }
    if (settings.contains("UI/WindowState")) {
        restoreState(settings.value("UI/WindowState").toByteArray());
    }
    if (settings.contains("UI/SplitterState")) {
        mainSplitter->restoreState(settings.value("UI/SplitterState").toByteArray());
    }
    
    // 界面设置
    formatComboBox->setCurrentIndex(settings.value("UI/FormatComboIndex", 0).toInt());
}

void MainWindow::saveSettings()
{
    QSettings settings("DirectoryTreeViewer", "Settings");
    
    // 常规设置
    settings.setValue("UI/DarkTheme", isDarkTheme);
    
    // DirectoryTree设置
    settings.setValue("Tree/MaxDepth", directoryTree.getMaxDepth());
    settings.setValue("Tree/IndentChars", directoryTree.getIndentChars());
    settings.setValue("Tree/ShowFiles", directoryTree.getShowFiles());
    settings.setValue("Tree/ShowHidden", directoryTree.getShowHidden());
    settings.setValue("Tree/OutputFormat", static_cast<int>(directoryTree.getOutputFormat()));
    settings.setValue("Tree/SortMode", static_cast<int>(directoryTree.getSortMode()));
    settings.setValue("Tree/IgnorePatterns", directoryTree.getIgnorePatterns());
    
    // 窗口设置
    settings.setValue("UI/WindowGeometry", saveGeometry());
    settings.setValue("UI/WindowState", saveState());
    settings.setValue("UI/SplitterState", mainSplitter->saveState());
    
    // 界面设置
    settings.setValue("UI/FormatComboIndex", formatComboBox->currentIndex());
}

void MainWindow::processPath(const QString &path)
{
    if (path.isEmpty() || !QFileInfo(path).exists() || !QFileInfo(path).isDir()) {
        QMessageBox::warning(this, tr("错误"), tr("无效的目录路径: ") + path);
        return;
    }
    
    lastHandledPaths.clear();
    lastHandledPaths.append(path);
    
    // 开始异步扫描
    processDirectoryAsync(path);
}

void MainWindow::processDirectoryAsync(const QString &path)
{
    // 检查路径是否存在和可访问
    QFileInfo dirInfo(path);
    if (!dirInfo.exists() || !dirInfo.isDir()) {
        QMessageBox::critical(this, tr("错误"), tr("指定的路径不存在或不是有效目录。"));
        return;
    }
    
    // 设置扫描器参数 - 使用getter方法
    scanner.setMaxDepth(directoryTree.getMaxDepth());
    scanner.setShowHidden(directoryTree.getShowHidden());
    scanner.setIgnorePatterns(directoryTree.getIgnorePatterns());
    
    // 显示进度界面并开始扫描
    showScanProgress(true);
    showDropArea(false);
    scanner.startScan(path);
}

void MainWindow::updateHistoryMenu()
{
    // 和updateRecentFilesMenu实现相同，共用历史记录
    updateRecentFilesMenu();
}

void MainWindow::openDirectory()
{
    browseDirectory();
}

void MainWindow::saveResults()
{
    // 保存当前结果到文件
    exportToFile();
}

void MainWindow::printResults()
{
    if (treeTextEdit->toPlainText().isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("没有内容可打印"));
        return;
    }
    
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        treeTextEdit->print(&printer);
    }
}

void MainWindow::selectAll()
{
    treeTextEdit->selectAll();
}

void MainWindow::showFindDialog()
{
    // 简单实现，调用QTextEdit内置的查找功能
    bool ok;
    QString text = QInputDialog::getText(this, tr("查找"), 
                                      tr("搜索文本:"), QLineEdit::Normal,
                                      QString(), &ok);
    if (ok && !text.isEmpty()) {
        if (!treeTextEdit->find(text)) {
            QMessageBox::information(this, tr("查找结果"), 
                                   tr("未找到文本 '%1'").arg(text));
        }
    }
}

void MainWindow::toggleTheme()
{
    applyTheme(!isDarkTheme);
}

void MainWindow::changeFont()
{
    bool ok;
    QFont currentFont = treeTextEdit->font();
    QFont font = QFontDialog::getFont(&ok, currentFont, this, tr("选择字体"));
    if (ok) {
        treeTextEdit->setFont(font);
    }
}

void MainWindow::updateRecentFilesMenu()
{
    recentMenu->clear();
    
    // 如果没有历史记录
    if (historyItems.isEmpty()) {
        QAction *emptyAction = recentMenu->addAction(tr("无历史记录"));
        emptyAction->setEnabled(false);
        return;
    }
    
    // 添加清除动作
    QAction *clearAction = recentMenu->addAction(tr("清除历史记录"));
    connect(clearAction, &QAction::triggered, [this]() {
        historyItems.clear();
        updateRecentFilesMenu();
    });
    
    recentMenu->addSeparator();
    
    // 添加最近记录
    for (int i = 0; i < historyItems.size(); ++i) {
        const HistoryItem &item = historyItems[i];
        QString text = QFileInfo(item.path).fileName() + " - " + 
                      item.timestamp.toString("yyyy-MM-dd hh:mm");
                      
        QAction *action = recentMenu->addAction(text);
        connect(action, &QAction::triggered, [this, i]() {
            loadFromHistory(i);
        });
    }
} 