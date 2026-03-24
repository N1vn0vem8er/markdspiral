#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editor/editor.h"
#include <QMainWindow>
#include <md4c-html.h>
#include <md4c.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleTabChanged(int index);
    void handleTextChanged();
    void handleCloseTab(int index);

private:
    QMap<Editor*, QString> m_htmlCache;
    Editor* m_currentEditor = nullptr;
    Ui::MainWindow *ui;
    void addEditor();
    QString markdownToHtml(const QString& markdown);
};
#endif // MAINWINDOW_H
