#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>

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

private:
    QComboBox *indentComboBox;
    QSpinBox *depthSpinBox;
    QCheckBox *showFilesCheckBox;
    QCheckBox *showHiddenCheckBox;
};

#endif // OPTIONSDIALOG_H 