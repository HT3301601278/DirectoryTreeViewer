#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QTextEdit>

#include "DirectoryTree.h"

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    
    // 缩进选项
    QString getIndentChars() const;
    void setIndentChars(const QString &indent);
    
    // 深度选项
    int getMaxDepth() const;
    void setMaxDepth(int depth);
    
    // 显示选项
    bool getShowFiles() const;
    void setShowFiles(bool show);
    
    bool getShowHidden() const;
    void setShowHidden(bool show);
    
    // 忽略规则
    QStringList getIgnorePatterns() const;
    void setIgnorePatterns(const QStringList &patterns);
    
    // 排序选项
    SortMode getSortMode() const;
    void setSortMode(SortMode mode);
    
    // 输出格式
    OutputFormat getOutputFormat() const;
    void setOutputFormat(OutputFormat format);
    
    // 主题选项
    bool getDarkTheme() const;
    void setDarkTheme(bool dark);

private slots:
    void addIgnorePattern();
    void removeIgnorePattern();
    void updateIndentPreview();
    void togglePatternEditButton();

private:
    // 主选项卡控件
    QTabWidget *tabWidget;
    
    // 基本选项卡
    QWidget *basicTab;
    QComboBox *indentComboBox;
    QSpinBox *indentSizeSpinBox;
    QTextEdit *indentPreview;
    QSpinBox *depthSpinBox;
    QCheckBox *showFilesCheckBox;
    QCheckBox *showHiddenCheckBox;
    
    // 过滤选项卡
    QWidget *filterTab;
    QListWidget *ignoreListWidget;
    QLineEdit *ignorePatternEdit;
    QPushButton *addPatternButton;
    QPushButton *removePatternButton;
    
    // 格式选项卡
    QWidget *formatTab;
    QButtonGroup *outputFormatGroup;
    QRadioButton *plainTextRadio;
    QRadioButton *markdownRadio;
    QRadioButton *jsonRadio;
    
    QButtonGroup *sortModeGroup;
    QRadioButton *sortNameAscRadio;
    QRadioButton *sortNameDescRadio;
    QRadioButton *sortTimeAscRadio;
    QRadioButton *sortTimeDescRadio;
    QRadioButton *sortFilesFirstRadio;
    QRadioButton *sortDirsFirstRadio;
    
    // 外观选项卡
    QWidget *appearanceTab;
    QCheckBox *darkThemeCheckBox;
    QComboBox *iconStyleComboBox;
    QGroupBox *customPrefixGroup;
    QLineEdit *dirPrefixEdit;
    QLineEdit *filePrefixEdit;
    QLineEdit *linePrefixEdit;
    
    QDialogButtonBox *buttonBox;
    
    void setupBasicTab();
    void setupFilterTab();
    void setupFormatTab();
    void setupAppearanceTab();
};

#endif // OPTIONSDIALOG_H 