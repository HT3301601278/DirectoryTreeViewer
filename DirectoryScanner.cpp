#include "DirectoryScanner.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

DirectoryScanner::DirectoryScanner(QObject *parent)
    : QObject(parent), m_showHidden(false), m_maxDepth(-1),
      m_isCancelled(false), m_isScanning(false),
      m_filesScanned(0), m_dirsScanned(0)
{
}

DirectoryScanner::~DirectoryScanner()
{
    cancelScan();
}

void DirectoryScanner::setIgnorePatterns(const QStringList &patterns)
{
    m_ignorePatterns = patterns;
}

void DirectoryScanner::setShowHidden(bool show)
{
    m_showHidden = show;
}

void DirectoryScanner::setMaxDepth(int depth)
{
    m_maxDepth = depth;
}

void DirectoryScanner::startScan(const QString &path)
{
    if (m_isScanning) {
        cancelScan();
    }
    
    m_isCancelled = false;
    m_isScanning = true;
    m_filesScanned = 0;
    m_dirsScanned = 0;
    
    emit scanStarted(path);
    m_scanTimer.start();
    
    // 使用QtConcurrent在另一个线程中运行扫描
    m_scanFuture = QtConcurrent::run([this, path]() {
        ScanResult result = scanDirectory(path);
        
        // 当扫描完成时，发出信号
        if (!m_isCancelled) {
            QMetaObject::invokeMethod(this, [this, result]() {
                emit scanFinished(result);
            }, Qt::QueuedConnection);
        }
        
        m_isScanning = false;
    });
}

void DirectoryScanner::cancelScan()
{
    if (!m_isScanning) {
        return;
    }
    
    m_isCancelled = true;
    
    if (m_scanFuture.isRunning()) {
        m_scanFuture.waitForFinished();
    }
    
    emit scanCancelled();
    m_isScanning = false;
}

bool DirectoryScanner::isScanning() const
{
    return m_isScanning;
}

ScanResult DirectoryScanner::scanDirectory(const QString &path)
{
    ScanResult result;
    result.path = path;
    result.fileCount = 0;
    result.dirCount = 0;
    result.cancelled = false;
    result.success = true;
    
    try {
        // 检查目录是否存在和可访问
        QDir dir(path);
        if (!dir.exists()) {
            result.success = false;
            result.errorMessage = "目录不存在：" + path;
            return result;
        }
        
        // 递归处理目录
        processDirectory(path, 0, result);
        
        if (m_isCancelled) {
            result.cancelled = true;
            result.success = false;
            result.errorMessage = "扫描被取消";
        }
    } catch (const std::exception &e) {
        result.success = false;
        result.errorMessage = QString("扫描错误：%1").arg(e.what());
    }
    
    return result;
}

void DirectoryScanner::processDirectory(const QString &path, int depth, ScanResult &result)
{
    // 检查是否取消
    if (m_isCancelled) {
        return;
    }
    
    // 处理QCoreApplication事件，以保持UI响应
    QCoreApplication::processEvents();
    
    // 检查深度限制
    if (m_maxDepth > 0 && depth >= m_maxDepth) {
        return;
    }
    
    QDir dir(path);
    
    // 设置过滤器
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks;
    if (m_showHidden) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    
    QFileInfoList entries = dir.entryInfoList();
    
    for (const QFileInfo &info : entries) {
        // 检查是否取消
        if (m_isCancelled) {
            return;
        }
        
        // 检查是否应忽略
        if (shouldIgnore(info.fileName())) {
            result.ignoredItems.append(info.filePath());
            continue;
        }
        
        if (info.isDir()) {
            // 统计目录
            result.dirCount++;
            m_dirsScanned++;
            
            // 每10个目录更新一次进度
            if (m_dirsScanned % 10 == 0) {
                QMetaObject::invokeMethod(this, [this]() {
                    emit scanProgress(m_filesScanned, m_dirsScanned);
                }, Qt::QueuedConnection);
            }
            
            // 递归扫描子目录
            processDirectory(info.filePath(), depth + 1, result);
        } else {
            // 统计文件
            result.fileCount++;
            m_filesScanned++;
            
            // 每100个文件更新一次进度
            if (m_filesScanned % 100 == 0) {
                QMetaObject::invokeMethod(this, [this]() {
                    emit scanProgress(m_filesScanned, m_dirsScanned);
                }, Qt::QueuedConnection);
            }
        }
    }
}

bool DirectoryScanner::shouldIgnore(const QString &name) const
{
    for (const QString &pattern : m_ignorePatterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(name)) {
            return true;
        }
    }
    return false;
} 