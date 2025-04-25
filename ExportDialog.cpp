#include "ExportDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("导出目录树");
    setMinimumWidth(450);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 文件路径选择
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLabel = new QLabel("导出到文件:");
    pathEdit = new QLineEdit();
    browseButton = new QPushButton("浏览...");
    
    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::browseFile);
    connect(pathEdit, &QLineEdit::textChanged, this, &ExportDialog::validateInput);
    
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathEdit, 1);
    pathLayout->addWidget(browseButton);
    
    // 格式选择
    QGroupBox *formatGroup = new QGroupBox("输出格式");
    QVBoxLayout *formatLayout = new QVBoxLayout(formatGroup);
    
    this->formatGroup = new QButtonGroup(this);
    txtRadio = new QRadioButton("文本文件 (.txt)");
    mdRadio = new QRadioButton("Markdown文件 (.md)");
    jsonRadio = new QRadioButton("JSON文件 (.json)");
    
    this->formatGroup->addButton(txtRadio, static_cast<int>(OutputFormat::PlainText));
    this->formatGroup->addButton(mdRadio, static_cast<int>(OutputFormat::Markdown));
    this->formatGroup->addButton(jsonRadio, static_cast<int>(OutputFormat::JSON));
    
    txtRadio->setChecked(true);
    
    connect(txtRadio, &QRadioButton::toggled, this, &ExportDialog::updateFileExtension);
    connect(mdRadio, &QRadioButton::toggled, this, &ExportDialog::updateFileExtension);
    connect(jsonRadio, &QRadioButton::toggled, this, &ExportDialog::updateFileExtension);
    
    formatLayout->addWidget(txtRadio);
    formatLayout->addWidget(mdRadio);
    formatLayout->addWidget(jsonRadio);
    
    // 按钮
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    exportButton = buttonBox->button(QDialogButtonBox::Ok);
    exportButton->setText("导出");
    exportButton->setEnabled(false);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // 设置默认导出路径和文件名
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    lastDirectory = documentsPath;
    pathEdit->setText(QDir(documentsPath).filePath("目录树.txt"));
    
    // 添加到主布局
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(formatGroup);
    mainLayout->addWidget(buttonBox);
}

void ExportDialog::browseFile()
{
    QString selectedFormat;
    QString fileExtension;
    
    if (txtRadio->isChecked()) {
        selectedFormat = "文本文件 (*.txt)";
        fileExtension = ".txt";
    } else if (mdRadio->isChecked()) {
        selectedFormat = "Markdown文件 (*.md)";
        fileExtension = ".md";
    } else if (jsonRadio->isChecked()) {
        selectedFormat = "JSON文件 (*.json)";
        fileExtension = ".json";
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "导出目录树",
                                                   lastDirectory,
                                                   selectedFormat);
    
    if (!filePath.isEmpty()) {
        // 确保文件有正确的扩展名
        QFileInfo fileInfo(filePath);
        lastDirectory = fileInfo.absolutePath();
        
        if (fileInfo.suffix().isEmpty()) {
            filePath += fileExtension;
        }
        
        pathEdit->setText(filePath);
    }
}

void ExportDialog::updateFileExtension()
{
    QString currentPath = pathEdit->text();
    if (currentPath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(currentPath);
    QString basePath = fileInfo.absolutePath();
    QString baseName = fileInfo.completeBaseName();
    QString newExtension;
    
    if (txtRadio->isChecked()) {
        newExtension = ".txt";
    } else if (mdRadio->isChecked()) {
        newExtension = ".md";
    } else if (jsonRadio->isChecked()) {
        newExtension = ".json";
    }
    
    QString newPath = QDir(basePath).filePath(baseName + newExtension);
    pathEdit->setText(newPath);
}

void ExportDialog::validateInput()
{
    QString path = pathEdit->text().trimmed();
    bool isValid = !path.isEmpty();
    
    if (isValid) {
        QFileInfo dirInfo(QFileInfo(path).absolutePath());
        isValid = dirInfo.exists() && dirInfo.isWritable();
    }
    
    exportButton->setEnabled(isValid);
}

QString ExportDialog::getFilePath() const
{
    return pathEdit->text();
}

OutputFormat ExportDialog::getOutputFormat() const
{
    return static_cast<OutputFormat>(formatGroup->checkedId());
} 