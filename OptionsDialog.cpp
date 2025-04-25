#include "OptionsDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>

OptionsDialog::OptionsDialog(const QString &currentIndent, int currentDepth, 
                           bool showFiles, bool showHidden, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("目录树选项");
    setMinimumWidth(300);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 缩进选项
    QGroupBox *indentGroup = new QGroupBox("缩进选项");
    QFormLayout *indentLayout = new QFormLayout(indentGroup);
    
    indentComboBox = new QComboBox;
    indentComboBox->addItem("4个空格", "    ");
    indentComboBox->addItem("2个空格", "  ");
    indentComboBox->addItem("Tab制表符", "\t");
    
    // 设置当前选中的缩进
    int index = 0;
    if (currentIndent == "  ") {
        index = 1;
    } else if (currentIndent == "\t") {
        index = 2;
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
    
    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // 添加所有组件到主布局
    mainLayout->addWidget(indentGroup);
    mainLayout->addWidget(depthGroup);
    mainLayout->addWidget(displayGroup);
    mainLayout->addWidget(buttonBox);
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