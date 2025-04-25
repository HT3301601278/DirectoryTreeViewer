#include "OptionsDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QFontDatabase>

OptionsDialog::OptionsDialog(const QString &currentIndent, int currentDepth, 
                           bool showFiles, bool showHidden, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("目录树选项");
    setMinimumWidth(450);
    setMinimumHeight(500);
    
    // 应用现代样式
    setStyleSheet(
        "QDialog { background-color: #f5f5f7; }"
        "QGroupBox { background-color: #ffffff; border-radius: 6px; border: 1px solid #e0e0e0; margin-top: 12px; padding-top: 16px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #444444; font-weight: bold; }"
        "QTabWidget::pane { border: 1px solid #e0e0e0; border-radius: 6px; background-color: #ffffff; }"
        "QTabBar::tab { background-color: #e8e8e8; border: 1px solid #d0d0d0; padding: 8px 16px; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: white; border-bottom-color: white; }"
        "QTabBar::tab:hover { background-color: #f0f0f0; }"
        "QPushButton { background-color: #0078d7; color: white; border: none; padding: 6px 14px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #0086f0; }"
        "QPushButton:pressed { background-color: #0069c0; }"
        "QPushButton:disabled { background-color: #cccccc; }"
        "QLineEdit, QComboBox, QSpinBox { border: 1px solid #d0d0d0; border-radius: 4px; padding: 5px; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #0078d7; }"
        "QListWidget { border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QCheckBox { spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);
    
    // 创建选项卡
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    
    // ----- 基本选项标签页 -----
    QWidget *basicTab = new QWidget;
    QVBoxLayout *basicLayout = new QVBoxLayout(basicTab);
    basicLayout->setContentsMargins(15, 15, 15, 15);
    basicLayout->setSpacing(15);
    
    // 缩进选项
    QGroupBox *indentGroup = new QGroupBox("缩进选项");
    QFormLayout *indentLayout = new QFormLayout(indentGroup);
    indentLayout->setContentsMargins(15, 15, 15, 15);
    indentLayout->setSpacing(10);
    indentLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    indentComboBox = new QComboBox;
    indentComboBox->addItem("4个空格 (标准)", "    ");
    indentComboBox->addItem("2个空格 (紧凑)", "  ");
    indentComboBox->addItem("8个空格 (宽松)", "        ");
    indentComboBox->addItem("Tab制表符", "\t");
    
    // 设置当前选中的缩进
    int index = 0;
    if (currentIndent == "  ") {
        index = 1;
    } else if (currentIndent == "        ") {
        index = 2;
    } else if (currentIndent == "\t") {
        index = 3;
    }
    indentComboBox->setCurrentIndex(index);
    
    QLabel *indentLabel = new QLabel("缩进类型:");
    indentLabel->setStyleSheet("font-weight: bold;");
    
    indentLayout->addRow(indentLabel, indentComboBox);
    
    // 深度选项
    QGroupBox *depthGroup = new QGroupBox("深度选项");
    QFormLayout *depthLayout = new QFormLayout(depthGroup);
    depthLayout->setContentsMargins(15, 15, 15, 15);
    depthLayout->setSpacing(10);
    depthLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    depthSpinBox = new QSpinBox;
    depthSpinBox->setRange(-1, 100);
    depthSpinBox->setValue(currentDepth);
    depthSpinBox->setSpecialValueText("不限制");
    
    QLabel *depthLabel = new QLabel("最大深度:");
    depthLabel->setStyleSheet("font-weight: bold;");
    
    depthLayout->addRow(depthLabel, depthSpinBox);
    
    // 显示选项
    QGroupBox *displayGroup = new QGroupBox("显示选项");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
    displayLayout->setContentsMargins(15, 15, 15, 15);
    displayLayout->setSpacing(10);
    
    showFilesCheckBox = new QCheckBox("显示文件");
    showFilesCheckBox->setChecked(showFiles);
    
    showHiddenCheckBox = new QCheckBox("显示隐藏文件");
    showHiddenCheckBox->setChecked(showHidden);
    
    displayLayout->addWidget(showFilesCheckBox);
    displayLayout->addWidget(showHiddenCheckBox);
    
    // 添加到基本选项布局
    basicLayout->addWidget(indentGroup);
    basicLayout->addWidget(depthGroup);
    basicLayout->addWidget(displayGroup);
    basicLayout->addStretch();
    
    // ----- 高级选项标签页 -----
    QWidget *advancedTab = new QWidget;
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->setContentsMargins(15, 15, 15, 15);
    advancedLayout->setSpacing(15);
    
    // 排序选项
    QGroupBox *sortGroup = new QGroupBox("排序选项");
    QFormLayout *sortLayout = new QFormLayout(sortGroup);
    sortLayout->setContentsMargins(15, 15, 15, 15);
    sortLayout->setSpacing(10);
    sortLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    sortTypeComboBox = new QComboBox;
    sortTypeComboBox->addItem("文件夹优先", static_cast<int>(SortType::DIRS_FIRST));
    sortTypeComboBox->addItem("文件优先", static_cast<int>(SortType::FILES_FIRST));
    sortTypeComboBox->addItem("按名称", static_cast<int>(SortType::NAME));
    sortTypeComboBox->addItem("按修改时间", static_cast<int>(SortType::MODIFIED_TIME));
    
    QLabel *sortLabel = new QLabel("排序方式:");
    sortLabel->setStyleSheet("font-weight: bold;");
    
    sortLayout->addRow(sortLabel, sortTypeComboBox);
    
    // 输出格式选项
    QGroupBox *formatGroup = new QGroupBox("输出格式");
    QFormLayout *formatLayout = new QFormLayout(formatGroup);
    formatLayout->setContentsMargins(15, 15, 15, 15);
    formatLayout->setSpacing(10);
    formatLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    outputFormatComboBox = new QComboBox;
    outputFormatComboBox->addItem(QIcon(style()->standardPixmap(QStyle::SP_FileDialogDetailedView)), "文本（树状符号）", static_cast<int>(OutputFormat::TEXT));
    outputFormatComboBox->addItem(QIcon(style()->standardPixmap(QStyle::SP_FileDialogContentsView)), "Markdown", static_cast<int>(OutputFormat::MARKDOWN));
    
    QLabel *formatLabel = new QLabel("输出格式:");
    formatLabel->setStyleSheet("font-weight: bold;");
    
    formatLayout->addRow(formatLabel, outputFormatComboBox);
    
    // 添加到高级选项布局
    advancedLayout->addWidget(sortGroup);
    advancedLayout->addWidget(formatGroup);
    advancedLayout->addStretch();
    
    // ----- 忽略模式标签页 -----
    QWidget *ignoreTab = new QWidget;
    QVBoxLayout *ignoreLayout = new QVBoxLayout(ignoreTab);
    ignoreLayout->setContentsMargins(15, 15, 15, 15);
    ignoreLayout->setSpacing(15);
    
    QLabel *ignoreLabel = new QLabel("指定要忽略的文件或文件夹（支持通配符 * 和 ?）");
    ignoreLabel->setStyleSheet("color: #444444; font-weight: bold; margin-bottom: 8px;");
    ignoreLayout->addWidget(ignoreLabel);
    
    // 输入和按钮区域
    QHBoxLayout *ignoreInputLayout = new QHBoxLayout;
    ignoreInputLayout->setSpacing(10);
    
    ignorePatternEdit = new QLineEdit;
    ignorePatternEdit->setPlaceholderText("输入忽略模式，如 node_modules 或 *.tmp");
    
    addIgnoreButton = new QPushButton("添加");
    addIgnoreButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(addIgnoreButton, &QPushButton::clicked, this, &OptionsDialog::addIgnorePattern);
    
    ignoreInputLayout->addWidget(ignorePatternEdit);
    ignoreInputLayout->addWidget(addIgnoreButton);
    
    ignoreLayout->addLayout(ignoreInputLayout);
    
    // 忽略列表框架
    QGroupBox *ignoreListGroup = new QGroupBox("忽略列表");
    QVBoxLayout *ignoreListLayout = new QVBoxLayout(ignoreListGroup);
    ignoreListLayout->setContentsMargins(15, 15, 15, 15);
    ignoreListLayout->setSpacing(10);
    
    // 忽略列表
    ignorePatternList = new QListWidget;
    ignorePatternList->setAlternatingRowColors(true);
    
    // 预填充一些常见的忽略模式
    ignorePatternList->addItem(".git");
    ignorePatternList->addItem("node_modules");
    ignorePatternList->addItem(".vscode");
    ignorePatternList->addItem("__pycache__");
    
    ignoreListLayout->addWidget(ignorePatternList);
    
    // 删除按钮
    removeIgnoreButton = new QPushButton("删除所选项");
    removeIgnoreButton->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    connect(removeIgnoreButton, &QPushButton::clicked, this, &OptionsDialog::removeIgnorePattern);
    
    ignoreListLayout->addWidget(removeIgnoreButton);
    ignoreLayout->addWidget(ignoreListGroup);
    
    // 添加所有标签页到选项卡控件
    tabWidget->addTab(basicTab, QIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView)), "基本选项");
    tabWidget->addTab(advancedTab, QIcon(style()->standardIcon(QStyle::SP_FileDialogListView)), "高级选项");
    tabWidget->addTab(ignoreTab, QIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton)), "忽略模式");
    
    mainLayout->addWidget(tabWidget);
    
    // 按钮区域
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    // 美化按钮
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    
    okButton->setText("确定");
    cancelButton->setText("取消");
    
    okButton->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox);
}

void OptionsDialog::addIgnorePattern()
{
    QString pattern = ignorePatternEdit->text().trimmed();
    if (pattern.isEmpty()) {
        return;
    }
    
    // 检查是否已存在
    QList<QListWidgetItem*> existingItems = ignorePatternList->findItems(pattern, Qt::MatchExactly);
    if (existingItems.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(QIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton)), pattern);
        ignorePatternList->addItem(item);
        ignorePatternEdit->clear();
    } else {
        QMessageBox::information(this, "提示", "该模式已存在于列表中");
    }
}

void OptionsDialog::removeIgnorePattern()
{
    QList<QListWidgetItem*> selectedItems = ignorePatternList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        delete ignorePatternList->takeItem(ignorePatternList->row(item));
    }
}

QString OptionsDialog::getIndentChars() const
{
    return indentComboBox->currentData().toString();
}

int OptionsDialog::getMaxDepth() const
{
    return depthSpinBox->value();
}

bool OptionsDialog::getShowFiles() const
{
    return showFilesCheckBox->isChecked();
}

bool OptionsDialog::getShowHidden() const
{
    return showHiddenCheckBox->isChecked();
}

QStringList OptionsDialog::getIgnorePatterns() const
{
    QStringList patterns;
    for (int i = 0; i < ignorePatternList->count(); ++i) {
        patterns << ignorePatternList->item(i)->text();
    }
    return patterns;
}

SortType OptionsDialog::getSortType() const
{
    return static_cast<SortType>(sortTypeComboBox->currentData().toInt());
}

OutputFormat OptionsDialog::getOutputFormat() const
{
    return static_cast<OutputFormat>(outputFormatComboBox->currentData().toInt());
} 