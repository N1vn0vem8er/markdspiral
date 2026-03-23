#include "mainwindow.h"
#include "editor/editor.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    addEditor();
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
    handleTabChanged(ui->tabWidget->currentIndex());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEditor()
{
    ui->tabWidget->addTab(new Editor(ui->tabWidget), "editor");
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
    if (index == -1) return;
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
