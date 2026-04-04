#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class textInputDialog;
}

class TextInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextInputDialog(QWidget *parent = nullptr);
    QString getText() const;
    ~TextInputDialog();

private:
    Ui::textInputDialog *ui;
    void keyPressed(QKeyEvent event);
    void acceptAndClose();
};

#endif // TEXTINPUTDIALOG_H
