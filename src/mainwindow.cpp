#include "mainwindow.h"
#include "editor/editor.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::handleCloseTab);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionClose, &QAction::triggered, this, [this]{handleCloseTab(ui->tabWidget->currentIndex());});
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::openNewEditor);
    handleTabChanged(ui->tabWidget->currentIndex());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEditor(const QString& text, const QString& name, const QString& path)
{
    Editor* editor = new Editor(ui->tabWidget);
    editor->setPath(path);
    editor->setPlainText(text);
    ui->tabWidget->addTab(editor, name);
}

QString MainWindow::markdownToHtml(const QString &markdown)
{
    QString html;
    auto process_output = [](const MD_CHAR* text, MD_SIZE size, void* userdata){
        auto* out = static_cast<QString*>(userdata);
        out->append(QString::fromUtf8(text, size));
    };
    QByteArray bytes = markdown.toUtf8();
    md_html(bytes.constData(), bytes.size(), process_output, &html, MD_DIALECT_GITHUB, MD_HTML_FLAG_SKIP_UTF8_BOM);

    return html;
}

void MainWindow::handleTabChanged(int index)
{
    if(index == -1) return;
    if(m_currentEditor)
        disconnect(m_currentEditor, &Editor::textChanged, this, &MainWindow::handleTextChanged);
    m_currentEditor = qobject_cast<Editor*>(ui->tabWidget->widget(index));
    if(m_currentEditor)
    {
        if(m_htmlCache.contains(m_currentEditor))
            ui->webEngineView->setHtml(m_htmlCache[m_currentEditor]);
        else
            handleTextChanged();
        connect(m_currentEditor, &Editor::textChanged, this, &MainWindow::handleTextChanged);
    }
}

void MainWindow::handleTextChanged()
{
    if(!m_currentEditor) return;
    QString html = markdownToHtml(m_currentEditor->toPlainText());
    m_htmlCache[m_currentEditor] = html;
    ui->webEngineView->setHtml(html);
}

void MainWindow::handleCloseTab(int index)
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->widget(index));
    if(editor)
    {
        m_htmlCache.remove(editor);
        editor->deleteLater();
    }
    ui->tabWidget->removeTab(index);
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open"), QDir::homePath());
    if(!path.isEmpty())
    {
        QFile file(path);
        if(file.open(QIODevice::ReadOnly))
        {
            addEditor(file.readAll(), file.fileName(), path);
        }
    }
}

void MainWindow::saveFile()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        const QString path = editor->getPath();
        if(!path.isEmpty())
        {
            QFile file(path);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(editor->toPlainText().toUtf8());
                file.close();
                editor->setSaved(true);
                ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), file.fileName());
            }
        }
        else
            saveFileAs();
    }
}

void MainWindow::saveFileAs()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        const QString path = QFileDialog::getSaveFileName(this, tr("Save As"), QDir::homePath());
        if(!path.isEmpty())
        {
            QFile file(path);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(editor->toPlainText().toUtf8());
                file.close();
                editor->setSaved(true);
                editor->setPath(path);
            }
        }
    }
}

void MainWindow::openNewEditor()
{
    addEditor(QString(), tr("New File"), QString());
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}
