#include "dictionaryprovider.h"
#include "editor.h"
#include "editor/markdownhighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <qdir.h>
#include <qtconcurrentrun.h>

Editor::Editor(QWidget *parent) : QPlainTextEdit(parent)
{
    init();
}

Editor::Editor(const QString &text, const QString &path, QWidget *parent)
{
    init();
}

Editor::~Editor()
{
    delayTimer.stop();
    watcher.future().cancel();
    watcher.waitForFinished();
}

QString Editor::getPath() const
{
    return path;
}

void Editor::setPath(const QString &newPath)
{
    path = newPath;
    if(!path.isEmpty())
    {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        if(file.isOpen())
        {
            orginalContent = file.readAll();
        }
    }
}

int Editor::lineNumberWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while(max >= 10)
    {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void Editor::lineNumberAreaPaint(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    QColor bgColor = palette().color(QPalette::Window);
    QColor textColor = palette().color(QPalette::WindowText);
    painter.fillRect(event->rect(), bgColor);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while(block.isValid() && top <= event->rect().bottom())
    {
        if(block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(textColor);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool Editor::getSaved() const
{
    return saved;
}

void Editor::setSaved(bool newSaved)
{
    saved = newSaved;
}

bool Editor::isSaved()
{
    return toPlainText() == orginalContent;
}

QString Editor::getName() const
{
    return name;
}

void Editor::setName(const QString &newName)
{
    name = newName;
}

void Editor::find(const QString &text)
{
    if(text.isEmpty()) return;
    if(!QPlainTextEdit::find(text))
    {
        moveCursor(QTextCursor::Start);
        QPlainTextEdit::find(text);
    }
}

void Editor::replace(const QString &findText, const QString &replaceText)
{
    if(findText.isEmpty()) return;
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection() && cursor.selectedText() == findText)
        cursor.insertText(replaceText);
    find(findText);
}

void Editor::clearSearchFormatting()
{

}

void Editor::increaseFontSize()
{
    QFont f = font();
    f.setPointSize(f.pointSize() + 1);
    setFont(f);
    updateLineNumberWidth(0);
    emit fontSizeChanged(f.pointSize());
}

void Editor::decreaseFontSize()
{
    QFont f = font();
    if(f.pointSize() > 4)
    {
        f.setPointSize(f.pointSize() - 1);
        setFont(f);
        updateLineNumberWidth(0);
        emit fontSizeChanged(f.pointSize());
    }
}

void Editor::setFontSize(int size)
{

}

void Editor::setLanguage(const QString &code)
{
    languageCode = code;
}

void Editor::setSpellCheckEnabled(bool val)
{
    spellCheckEnabled = val;
    highlighter->setErrorList({});
}

void Editor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberWidth(), cr.height()));
}

void Editor::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier)
    {
        if(event->angleDelta().y() > 0)
            increaseFontSize();
        else
            decreaseFontSize();
        event->accept();
    }
    else
        QPlainTextEdit::wheelEvent(event);
}

void Editor::startAsyncCheck()
{
    if(!spellCheckEnabled || watcher.isRunning()) return;

    auto checkText = [](const QString& text, std::shared_ptr<nuspell::Dictionary> dict){
        QList<MarkdownHighlighter::SpellError> errors;
        if(!dict) return errors;
        QString masked = text;
        auto maskMatchesWithSpaces = [](QString& s, const QRegularExpression& re) {
            QRegularExpressionMatchIterator it = re.globalMatch(s);
            QList<QPair<int, int>> ranges;
            while(it.hasNext())
            {
                auto m = it.next();
                ranges.append({m.capturedStart(), m.capturedLength()});
            }
            for(int i = ranges.size() - 1; i >= 0; i--)
            {
                int len = ranges[i].second;
                s.replace(ranges[i].first, len, QString(len, QChar(' ')));
            }
        };
        static QRegularExpression fencedCodeRegex(R"(```[\s\S]*?```)", QRegularExpression::MultilineOption);
        static QRegularExpression inlineCodeRegex(R"(`[^`\n]*`)");
        static QRegularExpression htmlTagRegex(R"(<[^>]+>)");
        static QRegularExpression urlRegex(R"(\bhttps?://[^\s)]+)");
        static QRegularExpression imageRegex(R"(!\[[^\]]*\]\([^)]+\))");
        static QRegularExpression linkRegex(R"(\[[^\]]+\]\([^)]+\))");

        maskMatchesWithSpaces(masked, fencedCodeRegex);
        maskMatchesWithSpaces(masked, inlineCodeRegex);
        maskMatchesWithSpaces(masked, htmlTagRegex);
        maskMatchesWithSpaces(masked, urlRegex);
        maskMatchesWithSpaces(masked, imageRegex);
        maskMatchesWithSpaces(masked, linkRegex);

        static QRegularExpression wordRegex(R"(\b\p{L}+(?:['’\-]\p{L}+)*\b)", QRegularExpression::UseUnicodePropertiesOption);
        QRegularExpressionMatchIterator it = wordRegex.globalMatch(masked);
        while(it.hasNext())
        {
            auto match = it.next();
            QString word = match.captured(0);
            if (!dict->spell(word.toUtf8().constData()))
            {
                errors.append({static_cast<int>(match.capturedStart()), static_cast<int>(word.length())});
            }
        }

        return errors;

    };
    QString textToCheck = this->toPlainText();
    QFuture<QList<MarkdownHighlighter::SpellError>> future = QtConcurrent::run(checkText, textToCheck, DictionaryProvider::instance().getDictionary(languageCode));
    watcher.setFuture(future);
}

void Editor::handleResults()
{
    QList<MarkdownHighlighter::SpellError> errors = watcher.result();
    highlighter->setErrorList(errors);
    blockSignals(true);
    highlighter->rehighlight();
    blockSignals(false);
}

void Editor::init()
{
    highlighter = new MarkdownHighlighter(this->document());
    lineNumberArea = new LineNumberArea(this);
    connect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberWidth);
    connect(this, &Editor::updateRequest, this, &Editor::updateLineNumber);
    updateLineNumberWidth(0);
    defaultFormat = textCursor().charFormat();
    connect(&watcher, &QFutureWatcher<QList<MarkdownHighlighter::SpellError>>::finished, this, &Editor::handleResults);
    connect(&delayTimer, &QTimer::timeout, this, &Editor::startAsyncCheck);
    connect(this, &Editor::textChanged, this, [this]{delayTimer.stop(); delayTimer.start(500);});
}

void Editor::updateLineNumberWidth(int count)
{
    setViewportMargins(lineNumberWidth(), 0, 0, 0);
}

void Editor::updateLineNumber(const QRect &rect, int dy)
{
    if(dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if(rect.contains(viewport()->rect()))
        updateLineNumberWidth(0);
}

bool Editor::getSaveWarningEnabled() const
{
    return saveWarningEnabled;
}

void Editor::setSaveWarningEnabled(bool newSaveWarningEnabled)
{
    saveWarningEnabled = newSaveWarningEnabled;
}


Editor::LineNumberArea::LineNumberArea(Editor *parent) : QWidget(parent), textEditor(parent)
{

}

QSize Editor::LineNumberArea::sizeHint() const
{
    return QSize(textEditor->lineNumberWidth(), 0);
}

void Editor::LineNumberArea::paintEvent(QPaintEvent *event)
{
    textEditor->lineNumberAreaPaint(event);
}
