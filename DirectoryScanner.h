#ifndef DIRECTORYSCANNER_H
#define DIRECTORYSCANNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QElapsedTimer>
#include <atomic>

// 扫描结果结构体
struct ScanResult {
    QString path;
    int fileCount;
    int dirCount;
    QStringList ignoredItems;
    bool cancelled;
    bool success;
    QString errorMessage;
};

class DirectoryScanner : public QObject
{
    Q_OBJECT

public:
    explicit DirectoryScanner(QObject *parent = nullptr);
    ~DirectoryScanner();

    // 设置扫描参数
    void setIgnorePatterns(const QStringList &patterns);
    void setShowHidden(bool show);
    void setMaxDepth(int depth);
    
    // 控制扫描
    void startScan(const QString &path);
    void cancelScan();
    bool isScanning() const;

signals:
    // 扫描过程事件
    void scanStarted(const QString &path);
    void scanProgress(int filesScanned, int dirsScanned);
    void scanFinished(const ScanResult &result);
    void scanCancelled();
    void scanError(const QString &errorMessage);

private:
    // 内部扫描方法
    ScanResult scanDirectory(const QString &path);
    void processDirectory(const QString &path, int depth, ScanResult &result);
    bool shouldIgnore(const QString &name) const;

    // 扫描参数
    QStringList m_ignorePatterns;
    bool m_showHidden;
    int m_maxDepth;

    // 扫描状态
    QFuture<void> m_scanFuture;
    QElapsedTimer m_scanTimer;
    std::atomic<bool> m_isCancelled;
    std::atomic<bool> m_isScanning;
    std::atomic<int> m_filesScanned;
    std::atomic<int> m_dirsScanned;
};

#endif // DIRECTORYSCANNER_H 