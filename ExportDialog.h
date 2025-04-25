#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>

#include "DirectoryTree.h"

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    
    QString getFilePath() const;
    OutputFormat getOutputFormat() const;

private slots:
    void browseFile();
    void updateFileExtension();
    void validateInput();

private:
    QLabel *pathLabel;
    QLineEdit *pathEdit;
    QPushButton *browseButton;
    
    QButtonGroup *formatGroup;
    QRadioButton *txtRadio;
    QRadioButton *mdRadio;
    QRadioButton *jsonRadio;
    
    QDialogButtonBox *buttonBox;
    QPushButton *exportButton;
    
    QString lastDirectory;
};

#endif // EXPORTDIALOG_H 