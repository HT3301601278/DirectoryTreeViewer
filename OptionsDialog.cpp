#include "OptionsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QStyle>
#include <QApplication>
#include <QScrollArea>
#include <QTextEdit>

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("目录树选项");
    setMinimumSize(500, 450);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡控件
    tabWidget = new QTabWidget(this);
    
    // 创建四个选项卡
    setupBasicTab();
    setupFilterTab();
    setupFormatTab();
    setupAppearanceTab();
    
    // 添加选项卡到主界面
    tabWidget->addTab(basicTab, "基本");
    tabWidget->addTab(filterTab, "过滤");
    tabWidget->addTab(formatTab, "格式");
    tabWidget->addTab(appearanceTab, "外观");
    
    // 按钮
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // 添加组件到主布局
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
}

// 设置基本选项卡
void OptionsDialog::setupBasicTab()
{
    basicTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(basicTab);
    
    // 缩进选项组
    QGroupBox *indentGroup = new QGroupBox("缩进选项");
    QVBoxLayout *indentLayout = new QVBoxLayout(indentGroup);
    
    // 缩进类型
    QHBoxLayout *indentTypeLayout = new QHBoxLayout();
    QLabel *indentTypeLabel = new QLabel("缩进类型:");
    indentComboBox = new QComboBox();
    indentComboBox->addItem("空格", " ");
    indentComboBox->addItem("制表符 (Tab)", "\t");
    
    indentTypeLayout->addWidget(indentTypeLabel);
    indentTypeLayout->addWidget(indentComboBox);
    
    // 空格数量
    QHBoxLayout *indentSizeLayout = new QHBoxLayout();
    QLabel *indentSizeLabel = new QLabel("空格数量:");
    indentSizeSpinBox = new QSpinBox();
    indentSizeSpinBox->setRange(1, 8);
    indentSizeSpinBox->setValue(4);
    indentSizeSpinBox->setEnabled(true);
    
    connect(indentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        indentSizeSpinBox->setEnabled(index == 0); // 只有在选择"空格"时才启用
        updateIndentPreview();
    });
    
    connect(indentSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &OptionsDialog::updateIndentPreview);
    
    indentSizeLayout->addWidget(indentSizeLabel);
    indentSizeLayout->addWidget(indentSizeSpinBox);
    
    // 缩进预览
    QLabel *previewLabel = new QLabel("预览:");
    indentPreview = new QTextEdit();
    indentPreview->setReadOnly(true);
    indentPreview->setMaximumHeight(100);
    indentPreview->setFont(QFont("Consolas", 10));
    
    updateIndentPreview();
    
    indentLayout->addLayout(indentTypeLayout);
    indentLayout->addLayout(indentSizeLayout);
    indentLayout->addWidget(previewLabel);
    indentLayout->addWidget(indentPreview);
    
    // 深度选项组
    QGroupBox *depthGroup = new QGroupBox("深度选项");
    QHBoxLayout *depthLayout = new QHBoxLayout(depthGroup);
    
    QLabel *depthLabel = new QLabel("最大深度:");
    depthSpinBox = new QSpinBox();
    depthSpinBox->setRange(-1, 999);
    depthSpinBox->setValue(-1);
    depthSpinBox->setSpecialValueText("不限制");
    
    depthLayout->addWidget(depthLabel);
    depthLayout->addWidget(depthSpinBox);
    depthLayout->addStretch();
    
    // 显示选项组
    QGroupBox *displayGroup = new QGroupBox("显示选项");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
    
    showFilesCheckBox = new QCheckBox("显示文件");
    showFilesCheckBox->setChecked(true);
    
    showHiddenCheckBox = new QCheckBox("显示隐藏文件/文件夹");
    showHiddenCheckBox->setChecked(false);
    
    displayLayout->addWidget(showFilesCheckBox);
    displayLayout->addWidget(showHiddenCheckBox);
    
    // 添加组到布局
    layout->addWidget(indentGroup);
    layout->addWidget(depthGroup);
    layout->addWidget(displayGroup);
    layout->addStretch();
}

// 设置过滤选项卡
void OptionsDialog::setupFilterTab()
{
    filterTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(filterTab);
    
    // 忽略项列表
    QLabel *ignoreLabel = new QLabel("忽略以下文件或文件夹 (支持通配符, 如 *.git, node_modules):");
    ignoreListWidget = new QListWidget();
    ignoreListWidget->setAlternatingRowColors(true);
    
    // 添加默认忽略项
    ignoreListWidget->addItem(".git");
    ignoreListWidget->addItem("node_modules");
    ignoreListWidget->addItem(".idea");
    ignoreListWidget->addItem("__pycache__");
    
    connect(ignoreListWidget, &QListWidget::itemSelectionChanged, this, &OptionsDialog::togglePatternEditButton);
    
    // 添加/删除按钮
    QHBoxLayout *patternButtonLayout = new QHBoxLayout();
    
    ignorePatternEdit = new QLineEdit();
    ignorePatternEdit->setPlaceholderText("输入要忽略的文件/文件夹名或通配符");
    
    addPatternButton = new QPushButton("添加");
    addPatternButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(addPatternButton, &QPushButton::clicked, this, &OptionsDialog::addIgnorePattern);
    
    removePatternButton = new QPushButton("删除");
    removePatternButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    removePatternButton->setEnabled(false);
    connect(removePatternButton, &QPushButton::clicked, this, &OptionsDialog::removeIgnorePattern);
    
    patternButtonLayout->addWidget(ignorePatternEdit);
    patternButtonLayout->addWidget(addPatternButton);
    patternButtonLayout->addWidget(removePatternButton);
    
    // 帮助说明
    QLabel *helpLabel = new QLabel("提示：通配符支持 * (匹配任意多个字符) 和 ? (匹配单个字符)");
    helpLabel->setStyleSheet("color: gray; font-style: italic;");
    
    layout->addWidget(ignoreLabel);
    layout->addWidget(ignoreListWidget);
    layout->addLayout(patternButtonLayout);
    layout->addWidget(helpLabel);
}

// 设置格式选项卡
void OptionsDialog::setupFormatTab()
{
    formatTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(formatTab);
    
    // 输出格式组
    QGroupBox *outputGroup = new QGroupBox("输出格式");
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    outputFormatGroup = new QButtonGroup(this);
    plainTextRadio = new QRadioButton("纯文本 (带树状符号 ├─, └─)");
    markdownRadio = new QRadioButton("Markdown (- folder/, - file.txt)");
    jsonRadio = new QRadioButton("JSON (节点数组)");
    
    outputFormatGroup->addButton(plainTextRadio, static_cast<int>(OutputFormat::PlainText));
    outputFormatGroup->addButton(markdownRadio, static_cast<int>(OutputFormat::Markdown));
    outputFormatGroup->addButton(jsonRadio, static_cast<int>(OutputFormat::JSON));
    
    plainTextRadio->setChecked(true);
    
    outputLayout->addWidget(plainTextRadio);
    outputLayout->addWidget(markdownRadio);
    outputLayout->addWidget(jsonRadio);
    
    // 排序方式组
    QGroupBox *sortGroup = new QGroupBox("排序方式");
    QVBoxLayout *sortLayout = new QVBoxLayout(sortGroup);
    
    sortModeGroup = new QButtonGroup(this);
    sortNameAscRadio = new QRadioButton("按名称 (升序 A-Z)");
    sortNameDescRadio = new QRadioButton("按名称 (降序 Z-A)");
    sortTimeAscRadio = new QRadioButton("按修改时间 (旧-新)");
    sortTimeDescRadio = new QRadioButton("按修改时间 (新-旧)");
    sortFilesFirstRadio = new QRadioButton("文件优先");
    sortDirsFirstRadio = new QRadioButton("文件夹优先");
    
    sortModeGroup->addButton(sortNameAscRadio, static_cast<int>(SortMode::NameAsc));
    sortModeGroup->addButton(sortNameDescRadio, static_cast<int>(SortMode::NameDesc));
    sortModeGroup->addButton(sortTimeAscRadio, static_cast<int>(SortMode::ModifiedTimeAsc));
    sortModeGroup->addButton(sortTimeDescRadio, static_cast<int>(SortMode::ModifiedTimeDesc));
    sortModeGroup->addButton(sortFilesFirstRadio, static_cast<int>(SortMode::FilesFirst));
    sortModeGroup->addButton(sortDirsFirstRadio, static_cast<int>(SortMode::DirectoriesFirst));
    
    sortDirsFirstRadio->setChecked(true);
    
    sortLayout->addWidget(sortNameAscRadio);
    sortLayout->addWidget(sortNameDescRadio);
    sortLayout->addWidget(sortTimeAscRadio);
    sortLayout->addWidget(sortTimeDescRadio);
    sortLayout->addWidget(sortFilesFirstRadio);
    sortLayout->addWidget(sortDirsFirstRadio);
    
    layout->addWidget(outputGroup);
    layout->addWidget(sortGroup);
    layout->addStretch();
}

// 设置外观选项卡
void OptionsDialog::setupAppearanceTab()
{
    appearanceTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(appearanceTab);
    
    // 主题设置
    QGroupBox *themeGroup = new QGroupBox("主题");
    QVBoxLayout *themeLayout = new QVBoxLayout(themeGroup);
    
    darkThemeCheckBox = new QCheckBox("深色主题");
    themeLayout->addWidget(darkThemeCheckBox);
    
    // 图标风格
    QGroupBox *iconGroup = new QGroupBox("图标风格");
    QHBoxLayout *iconLayout = new QHBoxLayout(iconGroup);
    
    QLabel *iconStyleLabel = new QLabel("图标风格:");
    iconStyleComboBox = new QComboBox();
    iconStyleComboBox->addItem("系统默认");
    iconStyleComboBox->addItem("彩色图标");
    iconStyleComboBox->addItem("单色图标");
    
    iconLayout->addWidget(iconStyleLabel);
    iconLayout->addWidget(iconStyleComboBox);
    iconLayout->addStretch();
    
    // 自定义节点前缀
    customPrefixGroup = new QGroupBox("自定义节点前缀");
    QVBoxLayout *prefixLayout = new QVBoxLayout(customPrefixGroup);
    
    QFormLayout *prefixFormLayout = new QFormLayout();
    dirPrefixEdit = new QLineEdit("📁 ");
    filePrefixEdit = new QLineEdit("📄 ");
    linePrefixEdit = new QLineEdit("│ ");
    
    prefixFormLayout->addRow("目录前缀:", dirPrefixEdit);
    prefixFormLayout->addRow("文件前缀:", filePrefixEdit);
    prefixFormLayout->addRow("连接线前缀:", linePrefixEdit);
    
    QLabel *prefixHelpLabel = new QLabel("提示：可以使用Unicode符号如 📁 📄 → • │ 等");
    prefixHelpLabel->setStyleSheet("color: gray; font-style: italic;");
    
    prefixLayout->addLayout(prefixFormLayout);
    prefixLayout->addWidget(prefixHelpLabel);
    
    layout->addWidget(themeGroup);
    layout->addWidget(iconGroup);
    layout->addWidget(customPrefixGroup);
    layout->addStretch();
}

// 获取/设置配置
QString OptionsDialog::getIndentChars() const
{
    if (indentComboBox->currentIndex() == 0) { // 空格
        return QString(" ").repeated(indentSizeSpinBox->value());
    } else { // 制表符
        return "\t";
    }
}

void OptionsDialog::setIndentChars(const QString &indent)
{
    if (indent == "\t") {
        indentComboBox->setCurrentIndex(1); // 制表符
    } else {
        indentComboBox->setCurrentIndex(0); // 空格
        indentSizeSpinBox->setValue(indent.length());
    }
    updateIndentPreview();
}

int OptionsDialog::getMaxDepth() const
{
    return depthSpinBox->value();
}

void OptionsDialog::setMaxDepth(int depth)
{
    depthSpinBox->setValue(depth);
}

bool OptionsDialog::getShowFiles() const
{
    return showFilesCheckBox->isChecked();
}

void OptionsDialog::setShowFiles(bool show)
{
    showFilesCheckBox->setChecked(show);
}

bool OptionsDialog::getShowHidden() const
{
    return showHiddenCheckBox->isChecked();
}

void OptionsDialog::setShowHidden(bool show)
{
    showHiddenCheckBox->setChecked(show);
}

QStringList OptionsDialog::getIgnorePatterns() const
{
    QStringList patterns;
    for (int i = 0; i < ignoreListWidget->count(); ++i) {
        patterns << ignoreListWidget->item(i)->text();
    }
    return patterns;
}

void OptionsDialog::setIgnorePatterns(const QStringList &patterns)
{
    ignoreListWidget->clear();
    ignoreListWidget->addItems(patterns);
}

SortMode OptionsDialog::getSortMode() const
{
    return static_cast<SortMode>(sortModeGroup->checkedId());
}

void OptionsDialog::setSortMode(SortMode mode)
{
    QAbstractButton *button = sortModeGroup->button(static_cast<int>(mode));
    if (button) {
        button->setChecked(true);
    }
}

OutputFormat OptionsDialog::getOutputFormat() const
{
    return static_cast<OutputFormat>(outputFormatGroup->checkedId());
}

void OptionsDialog::setOutputFormat(OutputFormat format)
{
    QAbstractButton *button = outputFormatGroup->button(static_cast<int>(format));
    if (button) {
        button->setChecked(true);
    }
}

bool OptionsDialog::getDarkTheme() const
{
    return darkThemeCheckBox->isChecked();
}

void OptionsDialog::setDarkTheme(bool dark)
{
    darkThemeCheckBox->setChecked(dark);
}

void OptionsDialog::addIgnorePattern()
{
    QString pattern = ignorePatternEdit->text().trimmed();
    if (!pattern.isEmpty()) {
        // 检查是否已存在
        QList<QListWidgetItem*> items = ignoreListWidget->findItems(pattern, Qt::MatchExactly);
        if (items.isEmpty()) {
            ignoreListWidget->addItem(pattern);
            ignorePatternEdit->clear();
        }
    }
}

void OptionsDialog::removeIgnorePattern()
{
    QList<QListWidgetItem*> selectedItems = ignoreListWidget->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        delete ignoreListWidget->takeItem(ignoreListWidget->row(item));
    }
    togglePatternEditButton();
}

void OptionsDialog::updateIndentPreview()
{
    QString indent = getIndentChars();
    QString preview = "根目录\n";
    preview += indent + "├── 文件夹1\n";
    preview += indent + indent + "├── 文件1.txt\n";
    preview += indent + indent + "└── 文件2.txt\n";
    preview += indent + "└── 文件夹2\n";
    preview += indent + indent + "└── 文件3.txt\n";
    
    indentPreview->setText(preview);
}

void OptionsDialog::togglePatternEditButton()
{
    removePatternButton->setEnabled(!ignoreListWidget->selectedItems().isEmpty());
} 