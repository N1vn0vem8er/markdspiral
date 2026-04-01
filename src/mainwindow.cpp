#include "mainwindow.h"
#include "editor/dictionaryprovider.h"
#include "editor/editor.h"
#include "processmanager.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    runningProcessesLabel = new RunningProcessesLabel("", ui->statusbar);
    ui->statusbar->addPermanentWidget(runningProcessesLabel);
    languageLabel = new QLabel("", ui->statusbar);
    ui->statusbar->addPermanentWidget(languageLabel);

    connect(ProcessManager::getInstance(), &ProcessManager::processAdded, runningProcessesLabel, &RunningProcessesLabel::addProcess);
    connect(ProcessManager::getInstance(), &ProcessManager::processRemoved, runningProcessesLabel, &RunningProcessesLabel::removeProcess);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::handleCloseTab);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionClose, &QAction::triggered, this, [this]{handleCloseTab(ui->tabWidget->currentIndex());});
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::openNewEditor);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionPaste_from_file, &QAction::triggered, this, &MainWindow::pasteFromFile);
    connect(ui->actionSelect_All, &QAction::triggered, this, &MainWindow::selectAll);
    connect(ui->actionDelete_All, &QAction::triggered, this, &MainWindow::deleteAll);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteText);
    connect(ui->stylecomboBox, &QComboBox::currentIndexChanged, this, &MainWindow::handleChangeStyle);
    connect(ui->actionSet_font, &QAction::triggered, this, &MainWindow::setFont);
    connect(ui->actionIncrease_font_size, &QAction::triggered, this, &MainWindow::increaseFontSize);
    connect(ui->actionDecrease_font_size, &QAction::triggered, this, &MainWindow::decreaseFontSize);
    connect(ui->actionShow_Git, &QAction::triggered, this, &MainWindow::showGitWidget);
    connect(ui->actionOpen_Dir, &QAction::triggered, this, &MainWindow::openDir);
    connect(ui->gitWidget, &GitWidget::openFile, this, &MainWindow::openTextFile);
    handleTabChanged(ui->tabWidget->currentIndex());
    ui->splitter->setStretchFactor(1, 1);
    handleChangeStyle(0);
    for(const auto& i : DictionaryProvider::instance().getLanguages())
    {
        QAction* action = new QAction(ui->menuSelect_Language);
        action->setText(i);
        connect(action, &QAction::triggered, this, [action, this]{
            emit setLanguage(action->text());
            languageLabel->setText(action->text());
            QSettings("markdspiral").setValue("spellcheck.language", action->text());
        });
        ui->menuSelect_Language->addAction(action);
    }
    QSettings settings("markdspiral");
    languageLabel->setText(settings.value("spellcheck.language").toString());
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
    editor->setLanguage(QSettings("markdspiral").value("spellcheck.language").toString());
    connect(editor, &Editor::textChanged, this, &MainWindow::handleTextChanged);
    connect(this, &MainWindow::setLanguage, editor, &Editor::setLanguage);
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
    m_currentEditor = qobject_cast<Editor*>(ui->tabWidget->widget(index));
    if(m_currentEditor)
    {
        if(m_htmlCache.contains(m_currentEditor))
            ui->webEngineView->setHtml(m_htmlCache[m_currentEditor]);
        else
            handleTextChanged();
    }
}

void MainWindow::handleTextChanged()
{
    if(!m_currentEditor) return;
    QString html = QStringLiteral(R"(<html><body style="margin: 0; padding: 0;"><div class="markdown-body" style="width: 100%; height: 100%;">%1</div></body></html>)").arg(markdownToHtml(m_currentEditor->toPlainText()));
    m_htmlCache[m_currentEditor] = html;
    ui->webEngineView->setHtml(html);
}

void MainWindow::handleCloseTab(int index)
{
    QWidget* widget = ui->tabWidget->widget(index);
    if(widget)
    {
        Editor* editor = qobject_cast<Editor*>(widget);
        if(editor)
        {
            m_htmlCache.remove(editor);
        }
        ui->tabWidget->removeTab(index);
        widget->deleteLater();
    }
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open"), QDir::homePath());
    if(!path.isEmpty())
        openTextFile(path);
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

void MainWindow::undo()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->undo();
}

void MainWindow::redo()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->redo();
}

void MainWindow::cut()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->cut();
}

void MainWindow::copy()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->copy();
}

void MainWindow::paste()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->paste();
}

void MainWindow::selectAll()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->selectAll();
}

void MainWindow::pasteFromFile()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        const QString path = QFileDialog::getOpenFileName(this, tr("Paste From File"), QDir::homePath());
        if(!path.isEmpty())
        {
            QFile file(path);
            if(file.open(QIODevice::ReadOnly))
                editor->appendPlainText(file.readAll());
        }
    }
}

void MainWindow::deleteText()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        QTextCursor cursor = editor->textCursor();
        if(cursor.hasSelection())
            cursor.removeSelectedText();
        else
            cursor.deleteChar();
    }
}

void MainWindow::deleteAll()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        editor->selectAll();
        QTextCursor cursor = editor->textCursor();
        if(cursor.hasSelection())
            cursor.removeSelectedText();
    }
}

void MainWindow::handleChangeStyle(int index)
{
    QString path;
    switch(index)
    {
    case 0:
        path = ":/style/css/github/github-markdown-dark.css";
        break;
    case 1:
        path = ":/style/css/github/github-markdown-light.css";
        break;
    case 2:
        path = ":/style/css/github/github-markdown-dark-colorblind.css";
        break;
    case 3:
        path = ":/style/css/github/github-markdown-dark-dimmed.css";
        break;
    case 4:
        path = ":/style/css/github/github-markdown-dark-high-contrast.css";
        break;
    case 5:
        path = ":/style/css/github/github-markdown-light-colorblind.css";
        break;
    case 6:
        path = ":/style/css/gitlab/gitlab.css";
        break;
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        QString style = file.readAll();
        file.close();
        QString js = QString(R"(
            var style = document.createElement('style');
            style.innerHTML = `%1`;
            document.head.appendChild(style);)"
        ).arg(style.simplified());
        QWebEngineScript script;
        script.setSourceCode(js);
        script.setName("markdownStyle");
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setRunsOnSubFrames(true);
        ui->webEngineView->page()->scripts().insert(script);
        handleTextChanged();
    }
}

void MainWindow::increaseFontSize()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->increaseFontSize();
}

void MainWindow::decreaseFontSize()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->decreaseFontSize();
}

void MainWindow::setFont()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
    {
        bool ok;
        const QFont font = QFontDialog::getFont(&ok, editor->font(), this, tr("Select Font"));
        if(ok)
            editor->setFont(font);
    }
}

void MainWindow::showGitWidget()
{
    if(ui->stackedWidget->currentIndex() == 0 && ui->stackedWidget->isVisible())
    {
        ui->stackedWidget->setVisible(false);
    }
    else
    {
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::openDir()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open"));
    if(!path.isEmpty())
    {
        ui->gitWidget->setRepositoryPath(path);
    }
}

void MainWindow::openTextFile(const QString &path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        addEditor(file.readAll(), file.fileName(), path);
    }
}
