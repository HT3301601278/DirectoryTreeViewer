#include "MainWindow.h"
#include "OptionsDialog.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>
#include <QHeaderView>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QInputDialog>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), 
    currentFormat(OutputFormat::TEXT), isHierarchicalView(false), lastExportPath("")
{
    setWindowTitle("目录树查看器");
    setMinimumSize(800, 600);
    
    // 加载书签和历史记录
    loadBookmarks();
    loadHistory();
    
    setupUI();
    setupBookmarkMenu();
}

void MainWindow::setupUI()
{
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
    
    // 创建进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setTextVisible(true);
    progressBar->setFormat("扫描中 %p% (%v/%m)");
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);
    
    // 创建工具栏
    toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    
    // 视图切换按钮
    toggleViewButton = new QPushButton("切换到层级视图", this);
    toggleViewButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    connect(toggleViewButton, &QPushButton::clicked, this, &MainWindow::toggleView);
    toolBar->addWidget(toggleViewButton);
    
    toolBar->addSeparator();
    
    // 书签按钮
    bookmarkButton = new QPushButton("书签", this);
    bookmarkButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    toolBar->addWidget(bookmarkButton);
    
    toolBar->addSeparator();
    
    // 格式选择
    QLabel *formatLabel = new QLabel("输出格式: ");
    toolBar->addWidget(formatLabel);
    
    formatComboBox = new QComboBox(this);
    formatComboBox->addItem("文本（树状符号）", static_cast<int>(OutputFormat::TEXT));
    formatComboBox->addItem("Markdown", static_cast<int>(OutputFormat::MARKDOWN));
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::switchFormat);
    toolBar->addWidget(formatComboBox);
    
    toolBar->addSeparator();
    
    // 复制按钮
    copyButton = new QPushButton("复制到剪贴板", this);
    copyButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::copyToClipboard);
    toolBar->addWidget(copyButton);
    
    // 导出按钮和菜单
    exportButton = new QPushButton("导出", this);
    exportButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    
    exportMenu = new QMenu(this);
    exportMenu->addAction("导出为文本文件(.txt)", this, [this]() { exportToFile(); });
    exportMenu->addAction("导出为Markdown文件(.md)", this, [this]() { exportToFile(); });
    exportMenu->addAction("导出为JSON文件(.json)", this, [this]() { exportToFile(); });
    
    exportButton->setMenu(exportMenu);
    toolBar->addWidget(exportButton);
    
    // 选项按钮
    optionsButton = new QPushButton("选项", this);
    optionsButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    connect(optionsButton, &QPushButton::clicked, this, &MainWindow::showOptionsDialog);
    toolBar->addWidget(optionsButton);
    
    mainLayout->addWidget(toolBar);
    
    // 创建文本编辑区
    treeTextEdit = new QTextEdit(this);
    treeTextEdit->setReadOnly(true);
    treeTextEdit->setFont(QFont("Consolas", 10));
    treeTextEdit->setStyleSheet("QTextEdit { border: 1px solid #ccc; border-radius: 3px; }");
    treeTextEdit->setPlaceholderText("这里将显示生成的目录树");
    mainLayout->addWidget(treeTextEdit, 1);
    
    // 创建树形视图
    treeView = new QTreeView(this);
    treeView->setAlternatingRowColors(true);
    treeView->setAnimated(true);
    treeView->setHeaderHidden(false);
    treeView->setSortingEnabled(true);
    treeView->setVisible(false);
    
    treeModel = new QStandardItemModel(this);
    QStringList headers;
    headers << "名称" << "类型" << "大小";
    treeModel->setHorizontalHeaderLabels(headers);
    treeView->setModel(treeModel);
    treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    mainLayout->addWidget(treeView, 1);
    
    // 设置接受拖放
    setAcceptDrops(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        dropAreaLabel->setStyleSheet("QLabel { background-color: #e0f0e0; border: 2px dashed #5a5; border-radius: 5px; padding: 30px; font-size: 16px; color: #2a2; }");
        dropAreaLabel->setText("松手以添加文件夹");
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
    dropAreaLabel->setText("将文件夹拖放到此处或点击选择文件夹");
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
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;
        
    const QString path = urls.first().toLocalFile();
    QFileInfo fileInfo(path);
    
    if (!fileInfo.isDir()) {
        QMessageBox::warning(this, "错误", "请拖放文件夹而不是文件");
        dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
        dropAreaLabel->setText("将文件夹拖放到此处或点击选择文件夹");
        return;
    }
    
    if (!fileInfo.isReadable()) {
        QMessageBox::warning(this, "错误", "无法读取该文件夹，可能是权限不足");
        dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
        dropAreaLabel->setText("将文件夹拖放到此处或点击选择文件夹");
        return;
    }
    
    dropAreaLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 2px dashed #aaa; border-radius: 5px; padding: 30px; font-size: 16px; }");
    dropAreaLabel->setText("将文件夹拖放到此处或点击选择文件夹");
    
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
    if (isHierarchicalView) {
        QMessageBox::information(this, "提示", "层级视图模式下无法复制，请切换到文本视图");
        return;
    }
    
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
        ignorePatterns = dialog.getIgnorePatterns();
        sortType = dialog.getSortType();
        currentFormat = dialog.getOutputFormat();
        
        formatComboBox->setCurrentIndex(static_cast<int>(currentFormat));
        
        // 如果已经有目录，重新生成树
        if (!currentPath.isEmpty()) {
            updateDirectoryTree();
        }
    }
}

void MainWindow::generateTree(const QString &path)
{
    currentPath = path;
    
    // 更新历史记录
    updateHistory(path);
    
    // 配置DirectoryTree
    dirTree.setIndentChars(indentChars);
    dirTree.setMaxDepth(maxDepth);
    dirTree.setShowFiles(showFiles);
    dirTree.setShowHidden(showHidden);
    dirTree.setIgnorePatterns(ignorePatterns);
    dirTree.setSortType(sortType);
    dirTree.setOutputFormat(currentFormat);
    
    // 开始扫描目录
    updateProgressBar(true, 0);
    
    // 使用计时器进行进度更新
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer]() {
        int total = dirTree.getTotalItems();
        int processed = dirTree.getProcessedItems();
        
        if (total > 0) {
            int percent = (processed * 100) / total;
            updateProgressBar(true, percent);
            progressBar->setFormat(QString("扫描中 %1% (%2/%3)").arg(percent).arg(processed).arg(total));
        }
        
        if (processed >= total && total > 0) {
            timer->stop();
            timer->deleteLater();
            updateProgressBar(false);
            
            // 显示完成消息
            QMessageBox::information(this, "完成", QString("目录树生成完成，扫描了 %1 个项目").arg(processed));
        }
    });
    
    timer->start(100); // 每100毫秒更新一次
    
    // 更新目录树
    updateDirectoryTree();
}

void MainWindow::updateDirectoryTree()
{
    if (isHierarchicalView) {
        // 层级视图模式
        treeTextEdit->setVisible(false);
        treeView->setVisible(true);
        
        QJsonObject jsonTree = dirTree.generateJsonTree(currentPath);
        createTreeViewModel(jsonTree);
    } else {
        // 文本视图模式
        treeTextEdit->setVisible(true);
        treeView->setVisible(false);
        
        QString result = dirTree.generateTree(currentPath);
        treeTextEdit->setText(result);
    }
}

void MainWindow::createTreeViewModel(const QJsonObject &jsonTree)
{
    treeModel->clear();
    
    QStringList headers;
    headers << "名称" << "类型" << "路径";
    treeModel->setHorizontalHeaderLabels(headers);
    
    // 添加根节点
    QStandardItem *rootItem = new QStandardItem(jsonTree["name"].toString());
    rootItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    treeModel->appendRow(rootItem);
    
    // 添加子节点
    QJsonArray children = jsonTree["children"].toArray();
    for (const QJsonValue &child : children) {
        QJsonObject childObj = child.toObject();
        
        QStandardItem *nameItem = new QStandardItem(childObj["name"].toString());
        QStandardItem *typeItem = new QStandardItem();
        QStandardItem *pathItem = new QStandardItem();
        
        QString type = childObj["type"].toString();
        if (type == "directory") {
            nameItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            typeItem->setText("文件夹");
            
            // 递归添加子目录
            QJsonArray subChildren = childObj["children"].toArray();
            for (const QJsonValue &subChild : subChildren) {
                addTreeItem(nameItem, subChild.toObject());
            }
        } else {
            nameItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
            typeItem->setText("文件");
            
            // 尺寸列显示路径
            pathItem->setText(childObj["path"].toString());
        }
        
        QList<QStandardItem*> rowItems;
        rowItems << nameItem << typeItem << pathItem;
        rootItem->appendRow(rowItems);
    }
    
    treeView->expandAll();
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
    treeView->resizeColumnToContents(2);
}

void MainWindow::addTreeItem(QStandardItem *parent, const QJsonObject &item)
{
    QStandardItem *nameItem = new QStandardItem(item["name"].toString());
    QStandardItem *typeItem = new QStandardItem();
    QStandardItem *sizeItem = new QStandardItem();
    
    QString type = item["type"].toString();
    if (type == "directory") {
        nameItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        typeItem->setText("文件夹");
        
        // 递归添加子目录
        QJsonArray children = item["children"].toArray();
        for (const QJsonValue &child : children) {
            addTreeItem(nameItem, child.toObject());
        }
    } else {
        nameItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        typeItem->setText("文件");
        
        // 尺寸列显示路径
        sizeItem->setText(item["path"].toString());
    }
    
    QList<QStandardItem*> rowItems;
    rowItems << nameItem << typeItem << sizeItem;
    parent->appendRow(rowItems);
}

void MainWindow::exportToFile()
{
    if (treeTextEdit->toPlainText().isEmpty() && !isHierarchicalView) {
        QMessageBox::information(this, "提示", "没有内容可导出");
        return;
    }
    
    QAction *action = qobject_cast<QAction*>(sender());
    QString format;
    QString filter;
    
    if (action) {
        QString actionText = action->text();
        if (actionText.contains(".txt")) {
            format = "txt";
            filter = "文本文件 (*.txt)";
        } else if (actionText.contains(".md")) {
            format = "md";
            filter = "Markdown文件 (*.md)";
        } else if (actionText.contains(".json")) {
            format = "json";
            filter = "JSON文件 (*.json)";
        }
    } else {
        // 默认使用当前选择的格式
        switch (currentFormat) {
            case OutputFormat::MARKDOWN:
                format = "md";
                filter = "Markdown文件 (*.md)";
                break;
            case OutputFormat::JSON:
                format = "json";
                filter = "JSON文件 (*.json)";
                break;
            case OutputFormat::TEXT:
            default:
                format = "txt";
                filter = "文本文件 (*.txt)";
                break;
        }
    }
    
    QFileInfo pathInfo(currentPath);
    QString defaultFileName = pathInfo.fileName() + "." + format;
    
    // 使用上次的导出路径，如果没有则使用文档目录
    QString startPath;
    if (!lastExportPath.isEmpty()) {
        QFileInfo lastPathInfo(lastExportPath);
        startPath = lastPathInfo.absolutePath() + "/" + defaultFileName;
    } else {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        startPath = documentsPath + "/" + defaultFileName;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "导出目录树", 
                                            startPath, 
                                            filter);
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 保存本次导出路径
    lastExportPath = filePath;
    
    if (format == "txt") {
        exportToTextFile(filePath);
    } else if (format == "md") {
        exportToMarkdownFile(filePath);
    } else if (format == "json") {
        exportToJsonFile(filePath);
    }
}

void MainWindow::exportToTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }
    
    OutputFormat originalFormat = currentFormat;
    dirTree.setOutputFormat(OutputFormat::TEXT);
    
    QTextStream out(&file);
    out << dirTree.generateTree(currentPath);
    
    file.close();
    
    dirTree.setOutputFormat(originalFormat);
    QMessageBox::information(this, "成功", "目录树已导出为文本文件");
}

void MainWindow::exportToMarkdownFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }
    
    OutputFormat originalFormat = currentFormat;
    dirTree.setOutputFormat(OutputFormat::MARKDOWN);
    
    QTextStream out(&file);
    out << dirTree.generateTree(currentPath);
    
    file.close();
    
    dirTree.setOutputFormat(originalFormat);
    QMessageBox::information(this, "成功", "目录树已导出为Markdown文件");
}

void MainWindow::exportToJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }
    
    QJsonObject jsonTree = dirTree.generateJsonTree(currentPath);
    QJsonDocument doc(jsonTree);
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    QMessageBox::information(this, "成功", "目录树已导出为JSON文件");
}

void MainWindow::toggleView()
{
    isHierarchicalView = !isHierarchicalView;
    
    if (isHierarchicalView) {
        toggleViewButton->setText("切换到文本视图");
        toggleViewButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    } else {
        toggleViewButton->setText("切换到层级视图");
        toggleViewButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    }
    
    if (!currentPath.isEmpty()) {
        updateDirectoryTree();
    }
}

void MainWindow::switchFormat(int index)
{
    currentFormat = static_cast<OutputFormat>(index);
    dirTree.setOutputFormat(currentFormat);
    
    if (!currentPath.isEmpty() && !isHierarchicalView) {
        QString result = dirTree.generateTree(currentPath);
        treeTextEdit->setText(result);
    }
}

void MainWindow::updateProgressBar(bool visible, int value)
{
    progressBar->setVisible(visible);
    if (visible) {
        progressBar->setValue(value);
    }
}

// 新增的书签和历史记录相关方法

void MainWindow::setupBookmarkMenu()
{
    bookmarkMenu = new QMenu(this);
    
    // 添加到书签
    addBookmarkAction = new QAction("添加当前目录到书签", this);
    addBookmarkAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(addBookmarkAction, &QAction::triggered, this, &MainWindow::addBookmark);
    bookmarkMenu->addAction(addBookmarkAction);
    
    // 管理书签
    manageBookmarksAction = new QAction("管理书签...", this);
    manageBookmarksAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    connect(manageBookmarksAction, &QAction::triggered, this, &MainWindow::showBookmarkDialog);
    bookmarkMenu->addAction(manageBookmarksAction);
    
    bookmarkMenu->addSeparator();
    
    // 添加历史记录菜单标题
    QAction *historyTitle = new QAction("最近历史", this);
    historyTitle->setEnabled(false);
    bookmarkMenu->addAction(historyTitle);
    
    // 添加历史记录项
    updateBookmarkMenu();
    
    // 设置书签按钮的菜单
    bookmarkButton->setMenu(bookmarkMenu);
}

void MainWindow::loadBookmarks()
{
    QSettings settings("DirectoryTreeViewer", "Bookmarks");
    bookmarks = settings.value("bookmarks").toStringList();
}

void MainWindow::saveBookmarks()
{
    QSettings settings("DirectoryTreeViewer", "Bookmarks");
    settings.setValue("bookmarks", bookmarks);
}

void MainWindow::updateHistory(const QString &path)
{
    // 如果路径已在历史记录中，先移除旧的
    recentHistory.removeAll(path);
    
    // 添加到历史记录的开头
    recentHistory.prepend(path);
    
    // 限制历史记录数量
    while (recentHistory.size() > MAX_HISTORY) {
        recentHistory.removeLast();
    }
    
    // 保存历史记录
    saveHistory();
    
    // 更新书签菜单
    updateBookmarkMenu();
}

void MainWindow::loadHistory()
{
    QSettings settings("DirectoryTreeViewer", "History");
    recentHistory = settings.value("recentHistory").toStringList();
    
    // 限制历史记录数量
    while (recentHistory.size() > MAX_HISTORY) {
        recentHistory.removeLast();
    }
}

void MainWindow::saveHistory()
{
    QSettings settings("DirectoryTreeViewer", "History");
    settings.setValue("recentHistory", recentHistory);
}

void MainWindow::updateBookmarkMenu()
{
    // 清除现有的书签和历史菜单项，但保留固定的前三项（添加书签、管理书签、分隔符和历史标题）
    QList<QAction*> actions = bookmarkMenu->actions();
    int fixedItems = 4; // 添加书签、管理书签、分隔符和历史标题
    
    while (actions.size() > fixedItems) {
        bookmarkMenu->removeAction(actions.last());
        actions.removeLast();
    }
    
    // 添加书签到菜单
    if (!bookmarks.isEmpty()) {
        bookmarkMenu->addSeparator();
        QAction *bookmarksTitle = new QAction("收藏的目录", this);
        bookmarksTitle->setEnabled(false);
        bookmarkMenu->addAction(bookmarksTitle);
        
        for (const QString &bookmark : bookmarks) {
            QFileInfo fi(bookmark);
            QAction *action = new QAction(fi.fileName(), this);
            action->setToolTip(bookmark);
            action->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            connect(action, &QAction::triggered, this, [this, bookmark]() {
                selectBookmark(bookmark);
            });
            bookmarkMenu->addAction(action);
        }
    }
    
    // 添加历史记录到菜单
    if (!recentHistory.isEmpty()) {
        for (const QString &historyItem : recentHistory) {
            QFileInfo fi(historyItem);
            QAction *action = new QAction(fi.fileName(), this);
            action->setToolTip(historyItem);
            action->setIcon(style()->standardIcon(QStyle::SP_DirLinkIcon));
            connect(action, &QAction::triggered, this, [this, historyItem]() {
                selectBookmark(historyItem);
            });
            bookmarkMenu->addAction(action);
        }
    } else {
        QAction *noHistoryAction = new QAction("无历史记录", this);
        noHistoryAction->setEnabled(false);
        bookmarkMenu->addAction(noHistoryAction);
    }
    
    // 禁用"添加当前目录到书签"，如果当前没有选择目录
    addBookmarkAction->setEnabled(!currentPath.isEmpty());
}

void MainWindow::addBookmark()
{
    if (currentPath.isEmpty()) {
        return;
    }
    
    // 检查是否已在书签中
    if (bookmarks.contains(currentPath)) {
        QMessageBox::information(this, "提示", "此目录已在书签中");
        return;
    }
    
    // 获取自定义名称（可选）
    bool ok;
    QFileInfo fi(currentPath);
    QString defaultName = fi.fileName();
    QString name = QInputDialog::getText(this, "添加书签", 
                                        "请输入书签名称：", 
                                        QLineEdit::Normal,
                                        defaultName, &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    
    // 添加到书签
    bookmarks.append(currentPath);
    saveBookmarks();
    
    // 更新书签菜单
    updateBookmarkMenu();
    
    QMessageBox::information(this, "成功", "目录已添加到书签");
}

void MainWindow::removeBookmark()
{
    // 获取书签列表中当前选择的项
    QListWidgetItem *item = bookmarkList->currentItem();
    if (!item) {
        return;
    }
    
    QString path = item->data(Qt::UserRole).toString();
    
    // 从书签中移除
    bookmarks.removeAll(path);
    saveBookmarks();
    
    // 更新列表
    delete item;
    
    // 更新书签菜单
    updateBookmarkMenu();
}

void MainWindow::selectBookmark(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isDir()) {
        QMessageBox::warning(this, "错误", "目录不存在: " + path);
        
        // 如果是书签，询问是否要移除
        if (bookmarks.contains(path)) {
            if (QMessageBox::question(this, "错误", 
                                    "此书签指向的目录不存在，是否要移除此书签？",
                                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                bookmarks.removeAll(path);
                saveBookmarks();
                updateBookmarkMenu();
            }
        }
        // 如果是历史记录，直接移除
        else if (recentHistory.contains(path)) {
            recentHistory.removeAll(path);
            saveHistory();
            updateBookmarkMenu();
        }
        
        return;
    }
    
    // 生成目录树
    generateTree(path);
}

void MainWindow::showBookmarkDialog()
{
    // 创建书签管理对话框
    QDialog dialog(this);
    dialog.setWindowTitle("管理书签和历史记录");
    dialog.setMinimumSize(600, 400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // 创建选项卡
    QTabWidget *tabWidget = new QTabWidget(&dialog);
    
    // 书签选项卡
    QWidget *bookmarksTab = new QWidget();
    QVBoxLayout *bookmarksLayout = new QVBoxLayout(bookmarksTab);
    
    bookmarkList = new QListWidget();
    bookmarkList->setAlternatingRowColors(true);
    bookmarkList->setIconSize(QSize(24, 24));
    
    // 填充书签列表
    for (const QString &path : bookmarks) {
        QFileInfo fi(path);
        QListWidgetItem *item = new QListWidgetItem(fi.fileName());
        item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        item->setToolTip(path);
        item->setData(Qt::UserRole, path);
        bookmarkList->addItem(item);
    }
    
    connect(bookmarkList, &QListWidget::itemDoubleClicked, this, &MainWindow::bookmarkItemClicked);
    
    bookmarksLayout->addWidget(bookmarkList);
    
    // 书签操作按钮
    QHBoxLayout *bookmarkButtonsLayout = new QHBoxLayout();
    
    QPushButton *removeBookmarkButton = new QPushButton("删除书签");
    removeBookmarkButton->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    connect(removeBookmarkButton, &QPushButton::clicked, this, &MainWindow::removeBookmark);
    
    QPushButton *openBookmarkButton = new QPushButton("打开选中的书签");
    openBookmarkButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(openBookmarkButton, &QPushButton::clicked, this, [this, &dialog]() {
        QListWidgetItem *item = bookmarkList->currentItem();
        if (item) {
            dialog.accept();
            selectBookmark(item->data(Qt::UserRole).toString());
        }
    });
    
    bookmarkButtonsLayout->addWidget(removeBookmarkButton);
    bookmarkButtonsLayout->addWidget(openBookmarkButton);
    
    bookmarksLayout->addLayout(bookmarkButtonsLayout);
    
    // 历史记录选项卡
    QWidget *historyTab = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
    
    historyList = new QListWidget();
    historyList->setAlternatingRowColors(true);
    historyList->setIconSize(QSize(24, 24));
    
    // 填充历史记录列表
    for (const QString &path : recentHistory) {
        QFileInfo fi(path);
        QListWidgetItem *item = new QListWidgetItem(fi.fileName());
        item->setIcon(style()->standardIcon(QStyle::SP_DirLinkIcon));
        item->setToolTip(path);
        item->setData(Qt::UserRole, path);
        historyList->addItem(item);
    }
    
    connect(historyList, &QListWidget::itemDoubleClicked, this, &MainWindow::historyItemClicked);
    
    historyLayout->addWidget(historyList);
    
    // 历史记录操作按钮
    QHBoxLayout *historyButtonsLayout = new QHBoxLayout();
    
    QPushButton *clearHistoryButton = new QPushButton("清除历史记录");
    clearHistoryButton->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    connect(clearHistoryButton, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "确认", 
                                "确定要清除所有历史记录吗？",
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            recentHistory.clear();
            saveHistory();
            historyList->clear();
            updateBookmarkMenu();
        }
    });
    
    QPushButton *openHistoryButton = new QPushButton("打开选中的历史记录");
    openHistoryButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(openHistoryButton, &QPushButton::clicked, this, [this, &dialog]() {
        QListWidgetItem *item = historyList->currentItem();
        if (item) {
            dialog.accept();
            selectBookmark(item->data(Qt::UserRole).toString());
        }
    });
    
    historyButtonsLayout->addWidget(clearHistoryButton);
    historyButtonsLayout->addWidget(openHistoryButton);
    
    historyLayout->addLayout(historyButtonsLayout);
    
    // 添加选项卡到选项卡控件
    tabWidget->addTab(bookmarksTab, "收藏的目录");
    tabWidget->addTab(historyTab, "历史记录");
    
    mainLayout->addWidget(tabWidget);
    
    // 底部按钮
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("关闭");
    closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    bottomButtonsLayout->addStretch();
    bottomButtonsLayout->addWidget(closeButton);
    
    mainLayout->addLayout(bottomButtonsLayout);
    
    // 显示对话框
    dialog.exec();
}

void MainWindow::bookmarkItemClicked(QListWidgetItem *item)
{
    if (item) {
        selectBookmark(item->data(Qt::UserRole).toString());
    }
}

void MainWindow::historyItemClicked(QListWidgetItem *item)
{
    if (item) {
        selectBookmark(item->data(Qt::UserRole).toString());
    }
} 