#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include "DirectoryTree.h"

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(const QString &currentIndent, int currentDepth, 
                          bool showFiles, bool showHidden, QWidget *parent = nullptr);
    
    QString getIndentChars() const;
    int getMaxDepth() const;
    bool getShowFiles() const;
    bool getShowHidden() const;
    QStringList getIgnorePatterns() const;
    SortType getSortType() const;
    OutputFormat getOutputFormat() const;

private slots:
    void addIgnorePattern();
    void removeIgnorePattern();

private:
    QTabWidget *tabWidget;
    
    // 基本选项标签页
    QComboBox *indentComboBox;
    QSpinBox *depthSpinBox;
    QCheckBox *showFilesCheckBox;
    QCheckBox *showHiddenCheckBox;
    
    // 高级选项标签页
    QComboBox *sortTypeComboBox;
    QComboBox *outputFormatComboBox;
    
    // 忽略模式标签页
    QLineEdit *ignorePatternEdit;
    QListWidget *ignorePatternList;
    QPushButton *addIgnoreButton;
    QPushButton *removeIgnoreButton;
};

#endif // OPTIONSDIALOG_H 