#include "mainwindow.h"
#include "editor/dictionaryprovider.h"
#include "editor/editor.h"
#include "processmanager.h"
#include "ui_mainwindow.h"
#include "widgets/markdownwebpage.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <widgets/gitbranchdialog.h>

#define VERSION "0.1.0"
#define LICENSELINK "https://www.gnu.org/licenses/gpl-3.0.html"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    runningProcessesLabel = new RunningProcessesLabel("", ui->statusbar);
    ui->statusbar->addPermanentWidget(runningProcessesLabel);
    languageLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(languageLabel);
    openedFileLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(openedFileLabel);
    branchButton = new QPushButton(ui->statusbar);
    branchButton->setVisible(false);
    branchButton->setFlat(true);
    ui->statusbar->addPermanentWidget(branchButton);
    connect(branchButton, &QPushButton::clicked, this, &MainWindow::openBranchDialog);

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
    connect(ui->actionAbout_Qt, &QAction::triggered, this, [this]{QMessageBox::aboutQt(this, tr("About Qt"));});
    connect(ui->actionAbout_Markdspiral, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionClose_All, &QAction::triggered, this, [&]{while(ui->tabWidget->count() > 0) handleCloseTab(ui->tabWidget->currentIndex());});
    connect(ui->actionClose_All_But_This, &QAction::triggered, this, [this]{for(int i=ui->tabWidget->count()-1; i >= 0;--i) if(i != ui->tabWidget->currentIndex()) handleCloseTab(i);});
    connect(ui->actionShow_Files, &QAction::triggered, this, &MainWindow::showFilesWidget);
    connect(ui->treeWidget, &FileSystemTree::openFile, this, &MainWindow::openTextFile);
    connect(ui->treeWidget, &FileSystemTree::gitAdd, ui->gitWidget, &GitWidget::gitAddFile);
    connect(ui->treeWidget, &FileSystemTree::gitDiff, ui->gitWidget, &GitWidget::gitFileDiff);
    connect(ui->actionFind_Replace, &QAction::triggered, this, [this]{ui->searchWidget->setVisible(!ui->searchWidget->isVisible());});
    connect(ui->gitWidget, &GitWidget::addTab, this, &MainWindow::addTab);
    connect(ui->actionCommit, &QAction::triggered, ui->gitWidget, &GitWidget::openGitCommit);
    connect(ui->actionLog, &QAction::triggered, ui->gitWidget, &GitWidget::gitLog);
    connect(ui->gitWidget, &GitWidget::openInEditor, this, &MainWindow::openInEditor);
    connect(ui->actionStatus, &QAction::triggered, ui->gitWidget, &GitWidget::gitStatus);
    connect(ui->actionPull, &QAction::triggered, ui->gitWidget, &GitWidget::gitPull);
    connect(ui->actionPush, &QAction::triggered, ui->gitWidget, &GitWidget::gitPush);
    connect(ui->actionFetch, &QAction::triggered, ui->gitWidget, &GitWidget::gitFetch);
    connect(ui->actionBranch, &QAction::triggered, this, &MainWindow::openBranchDialog);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::print);
    connect(ui->forwardButton, &QPushButton::clicked, this, &MainWindow::goForwardInPreview);
    connect(ui->backButton, &QPushButton::clicked, this, &MainWindow::goBackInPreview);
    connect(ui->actionFull_Screen, &QAction::triggered, this, &MainWindow::fullScreen);
    connect(ui->homeButton, &QPushButton::clicked, this, &MainWindow::handleTextChanged);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->actionShow_Preview, &QAction::triggered, this, [this](bool val){ui->previewWidget->setVisible(val);});
    connect(ui->gitWidget, &GitWidget::branchNameChanged, this, [this](const QString& name){branchButton->setText(name);});
    connect(ui->refreshButton, &QPushButton::clicked, ui->webEngineView, &QWebEngineView::reload);
    connect(ui->findButton, &QPushButton::clicked, this, &MainWindow::find);
    connect(ui->replaceButton, &QPushButton::clicked, this, &MainWindow::replace);
    connect(ui->replaceAllButton, &QPushButton::clicked, this, &MainWindow::replaceAll);

    MarkdownWebPage *page = new MarkdownWebPage(this);
    ui->webEngineView->setPage(page);
    connect(page, &MarkdownWebPage::localLinkClicked, this, &MainWindow::handleLocalLink);

    ui->searchWidget->setVisible(false);

    handleTabChanged(ui->tabWidget->currentIndex());
    ui->splitter->setStretchFactor(1, 1);
    handleChangeStyle(0);
    const auto languages = DictionaryProvider::instance().getLanguages();
    for(const auto& i : languages)
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
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    ui->stackedWidget->setVisible(settings.value("leftWidget.visible", false).toBool());
    ui->stackedWidget->setCurrentIndex(settings.value("leftWidget.index", 0).toInt());
    ui->actionShow_Preview->setChecked(settings.value("previewWidget.visible", true).toBool());
    ui->previewWidget->setVisible(ui->actionShow_Preview->isChecked());
}

MainWindow::~MainWindow()
{
    delete ui;
}

Editor* MainWindow::addEditor(const QString& text, const QString& name, const QString& path)
{
    Editor* editor = new Editor(ui->tabWidget);
    editor->setPath(path);
    editor->setPlainText(text);
    editor->setSpellCheckEnabled(ui->actionEnableSpellcheck->isChecked());
    editor->setLanguage(QSettings("markdspiral").value("spellcheck.language").toString());
    connect(editor, &Editor::textChanged, this, &MainWindow::handleTextChanged);
    connect(this, &MainWindow::setLanguage, editor, &Editor::setLanguage);
    connect(ui->actionEnableSpellcheck, &QAction::triggered, editor, &Editor::setSpellCheckEnabled);
    connect(editor, &Editor::fontSizeChanged, this, [this](int size){ui->statusbar->showMessage(tr("Font size: %1").arg(size), 5000);});
    ui->tabWidget->addTab(editor, name);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    return editor;
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

void MainWindow::renderPreviewFile(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    QString html = QStringLiteral(R"(<html><style>%1</style><body style="margin: 0; padding: 0;"><div class="markdown-body" style="width: 100%; min-height: 100vh;">%2</div></body></html>)").arg(currentStyle, markdownToHtml(content));
    ui->webEngineView->setHtml(html, QUrl::fromLocalFile(filePath));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        handleCloseTab(i);
    }
    QSettings settings("markdspiral");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("leftWidget.visible", ui->stackedWidget->isVisible());
    settings.setValue("leftWidget.index", ui->stackedWidget->currentIndex());
    settings.setValue("previewWidget.visible", ui->previewWidget->isVisible());
    QMainWindow::closeEvent(event);
}

void MainWindow::handleTabChanged(int index)
{
    if(index == -1)
    {
        m_currentEditor = nullptr;
        return;
    }
    m_currentEditor = qobject_cast<Editor*>(ui->tabWidget->widget(index));

    previewHistory.clear();
    previewHistoryIndex = -1;

    if(m_currentEditor)
    {
        if(m_htmlCache.contains(m_currentEditor))
            ui->webEngineView->setHtml(m_htmlCache[m_currentEditor], QUrl::fromLocalFile(m_currentEditor->getPath()));
        else
            handleTextChanged();
        openedFileLabel->setText(m_currentEditor->getPath());
    }
}

void MainWindow::handleTextChanged()
{
    if(!m_currentEditor) return;
    QString html = QStringLiteral(R"(<html><style>%1</style><body style="margin: 0; padding: 0;"><div class="markdown-body" style="width: 100%; min-height: 100vh;">%2</div></body></html>)").arg(currentStyle, markdownToHtml(m_currentEditor->toPlainText()));
    m_htmlCache[m_currentEditor] = html;
    ui->webEngineView->setHtml(html, QUrl::fromLocalFile(m_currentEditor->getPath()));
}

void MainWindow::handleCloseTab(int index)
{
    QWidget* widget = ui->tabWidget->widget(index);
    if(widget)
    {
        Editor* editor = qobject_cast<Editor*>(widget);
        if(editor)
        {
            if(!editor->isSaved())
            {
                const auto response = QMessageBox::question(this, tr("Save File?"),
                                                            tr("Save file %1?").arg(editor->getPath()),
                                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                            QMessageBox::Yes);
                if(response == QMessageBox::Yes)
                    saveFile();
                else if(response == QMessageBox::Cancel)
                    return;
            }
            m_htmlCache.remove(editor);
        }
        ui->tabWidget->removeTab(index);
        widget->deleteLater();
        openedFileLabel->clear();
    }
}

void MainWindow::openFile()
{
    QString openPath;
    if(!openedDir.isEmpty()) openPath = openedDir;
    else openPath = QSettings("markdspiral").value("lastOpenedFilePath", QDir::homePath()).toString();
    const QString path = QFileDialog::getOpenFileName(this, tr("Open"), openPath);
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
                ui->statusbar->showMessage(tr("Saved: %1").arg(path), 5000);
                QSettings settings("markdspiral");
                settings.setValue("lastOpenedFilePath", QFileInfo(path).dir().absolutePath());
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
        QString openPath;
        QSettings settings("markdspiral");
        if(!openedDir.isEmpty()) openPath = openedDir;
        else openPath = settings.value("lastOpenedFilePath", QDir::homePath()).toString();
        const QString path = QFileDialog::getSaveFileName(this, tr("Save As"), openPath);
        if(!path.isEmpty())
        {
            QFile file(path);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(editor->toPlainText().toUtf8());
                file.close();
                editor->setSaved(true);
                editor->setPath(path);
                ui->statusbar->showMessage(tr("Saved: %1").arg(path), 5000);
                ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QFileInfo(path).fileName());
                settings.setValue("lastOpenedFilePath", QFileInfo(path).dir().absolutePath());
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
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        currentStyle = file.readAll();
        file.close();
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
    QSettings settings("markdspiral");
    const QString path = QFileDialog::getExistingDirectory(this, tr("Open"), settings.value("lastOpenedDirPath", QDir::homePath()).toString());
    if(!path.isEmpty())
    {
        ui->gitWidget->setRepositoryPath(path);
        if(ui->gitWidget->hasRepository())
        {
            branchButton->setText(ui->gitWidget->getBranchName());
            branchButton->setVisible(true);
        }
        ui->treeWidget->open(path);
        openedDir = path;
        settings.setValue("lastOpenedDirPath", path);
    }
}

void MainWindow::openTextFile(const QString &path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        addEditor(file.readAll(), file.fileName(), path);
        QSettings settings("markdspiral");
        settings.setValue("lastOpenedFilePath", QFileInfo(path).dir().absolutePath());
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Markdspiral"), tr(R"(<h3>About Markdspiral</h3><p>Markdspiral is a markdown editor.<p>Version: %1</p><p>License: <a href="%2">GPL 3</a></p>)").arg(VERSION, LICENSELINK));
}

void MainWindow::showFilesWidget()
{
    if(ui->stackedWidget->currentIndex() == 1 && ui->stackedWidget->isVisible())
    {
        ui->stackedWidget->setVisible(false);
    }
    else
    {
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::addTab(QWidget *widget, const QString &title)
{
    ui->tabWidget->addTab(widget, title);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}

void MainWindow::openInEditor(const QString &text, const QString &title, bool readOnly, bool spellChecking, bool disableSaveWarning)
{
    Editor* editor = addEditor(text, title, "");
    editor->setReadOnly(readOnly);
    editor->setSaveWarningEnabled(!disableSaveWarning);
}

void MainWindow::openBranchDialog()
{
    GitBranchDialog* dialog = new GitBranchDialog(ui->gitWidget->getRepoPath(), ui->gitWidget->getBranches(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &GitBranchDialog::setBranch, ui->gitWidget, &GitWidget::setBranch);
    dialog->show();
}

void MainWindow::print()
{
    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    QPrintDialog dialog(printer, this);
    if(dialog.exec() == QDialog::Accepted)
    {
        connect(ui->webEngineView, &QWebEngineView::printFinished, this, [printer]{delete printer;}, Qt::SingleShotConnection);
        ui->webEngineView->print(printer);
    }
    else
        delete printer;
}

void MainWindow::handleLocalLink(const QUrl &url)
{
    QString filePath = url.toLocalFile();
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isFile()) return;
    if(previewHistoryIndex < previewHistory.size() - 1)
        previewHistory.erase(previewHistory.begin() + previewHistoryIndex + 1, previewHistory.end());
    previewHistory.append(filePath);
    previewHistoryIndex++;
    renderPreviewFile(filePath);
}

void MainWindow::goBackInPreview()
{
    if(previewHistoryIndex > 0)
    {
        previewHistoryIndex--;
        renderPreviewFile(previewHistory[previewHistoryIndex]);
    }
    else if(previewHistoryIndex == 0)
    {
        previewHistoryIndex--;
        if(m_currentEditor && m_htmlCache.contains(m_currentEditor))
            ui->webEngineView->setHtml(m_htmlCache[m_currentEditor], QUrl::fromLocalFile(m_currentEditor->getPath()));
    }
}

void MainWindow::goForwardInPreview()
{
    if(previewHistoryIndex < previewHistory.size() - 1)
    {
        previewHistoryIndex++;
        renderPreviewFile(previewHistory[previewHistoryIndex]);
    }
}

void MainWindow::fullScreen()
{
    if(windowState().testFlag(Qt::WindowFullScreen))
        showNormal();
    else
        showFullScreen();
}

void MainWindow::find()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->find(ui->findLineEdit->text());
}

void MainWindow::replace()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->replace(ui->findLineEdit->text(), ui->replaceLineEdit->text());
}

void MainWindow::replaceAll()
{
    Editor* editor = qobject_cast<Editor*>(ui->tabWidget->currentWidget());
    if(editor)
        editor->replaceAll(ui->findLineEdit->text(), ui->replaceLineEdit->text());
}
