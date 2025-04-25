#include "OptionsDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

OptionsDialog::OptionsDialog(const QString &currentIndent, int currentDepth, 
                           bool showFiles, bool showHidden, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("目录树选项");
    setMinimumWidth(400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    tabWidget = new QTabWidget(this);
    
    // ----- 基本选项标签页 -----
    QWidget *basicTab = new QWidget;
    QVBoxLayout *basicLayout = new QVBoxLayout(basicTab);
    
    // 缩进选项
    QGroupBox *indentGroup = new QGroupBox("缩进选项");
    QFormLayout *indentLayout = new QFormLayout(indentGroup);
    
    indentComboBox = new QComboBox;
    indentComboBox->addItem("4个空格", "    ");
    indentComboBox->addItem("2个空格", "  ");
    indentComboBox->addItem("8个空格", "        ");
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
    
    indentLayout->addRow("缩进类型:", indentComboBox);
    
    // 深度选项
    QGroupBox *depthGroup = new QGroupBox("深度选项");
    QFormLayout *depthLayout = new QFormLayout(depthGroup);
    
    depthSpinBox = new QSpinBox;
    depthSpinBox->setRange(-1, 100);
    depthSpinBox->setValue(currentDepth);
    depthSpinBox->setSpecialValueText("不限制");
    
    depthLayout->addRow("最大深度:", depthSpinBox);
    
    // 显示选项
    QGroupBox *displayGroup = new QGroupBox("显示选项");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
    
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
    
    // 排序选项
    QGroupBox *sortGroup = new QGroupBox("排序选项");
    QFormLayout *sortLayout = new QFormLayout(sortGroup);
    
    sortTypeComboBox = new QComboBox;
    sortTypeComboBox->addItem("文件夹优先", static_cast<int>(SortType::DIRS_FIRST));
    sortTypeComboBox->addItem("文件优先", static_cast<int>(SortType::FILES_FIRST));
    sortTypeComboBox->addItem("按名称", static_cast<int>(SortType::NAME));
    sortTypeComboBox->addItem("按修改时间", static_cast<int>(SortType::MODIFIED_TIME));
    
    sortLayout->addRow("排序方式:", sortTypeComboBox);
    
    // 输出格式选项
    QGroupBox *formatGroup = new QGroupBox("输出格式");
    QFormLayout *formatLayout = new QFormLayout(formatGroup);
    
    outputFormatComboBox = new QComboBox;
    outputFormatComboBox->addItem("文本（树状符号）", static_cast<int>(OutputFormat::TEXT));
    outputFormatComboBox->addItem("Markdown", static_cast<int>(OutputFormat::MARKDOWN));
    outputFormatComboBox->addItem("JSON", static_cast<int>(OutputFormat::JSON));
    
    formatLayout->addRow("输出格式:", outputFormatComboBox);
    
    // 添加到高级选项布局
    advancedLayout->addWidget(sortGroup);
    advancedLayout->addWidget(formatGroup);
    advancedLayout->addStretch();
    
    // ----- 忽略模式标签页 -----
    QWidget *ignoreTab = new QWidget;
    QVBoxLayout *ignoreLayout = new QVBoxLayout(ignoreTab);
    
    QLabel *ignoreLabel = new QLabel("指定要忽略的文件或文件夹（支持通配符 * 和 ?）");
    ignoreLayout->addWidget(ignoreLabel);
    
    // 输入和按钮区域
    QHBoxLayout *ignoreInputLayout = new QHBoxLayout;
    
    ignorePatternEdit = new QLineEdit;
    ignorePatternEdit->setPlaceholderText("输入忽略模式，如 node_modules 或 *.tmp");
    
    addIgnoreButton = new QPushButton("添加");
    connect(addIgnoreButton, &QPushButton::clicked, this, &OptionsDialog::addIgnorePattern);
    
    ignoreInputLayout->addWidget(ignorePatternEdit);
    ignoreInputLayout->addWidget(addIgnoreButton);
    
    ignoreLayout->addLayout(ignoreInputLayout);
    
    // 忽略列表
    ignorePatternList = new QListWidget;
    
    // 预填充一些常见的忽略模式
    ignorePatternList->addItem(".git");
    ignorePatternList->addItem("node_modules");
    ignorePatternList->addItem(".vscode");
    ignorePatternList->addItem("__pycache__");
    
    ignoreLayout->addWidget(ignorePatternList);
    
    // 删除按钮
    removeIgnoreButton = new QPushButton("删除所选项");
    connect(removeIgnoreButton, &QPushButton::clicked, this, &OptionsDialog::removeIgnorePattern);
    
    ignoreLayout->addWidget(removeIgnoreButton);
    
    // 添加所有标签页到选项卡控件
    tabWidget->addTab(basicTab, "基本选项");
    tabWidget->addTab(advancedTab, "高级选项");
    tabWidget->addTab(ignoreTab, "忽略模式");
    
    mainLayout->addWidget(tabWidget);
    
    // 按钮区域
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
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
        ignorePatternList->addItem(pattern);
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