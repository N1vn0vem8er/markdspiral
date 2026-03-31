#include "markdownhighlighter.h"
#include "editor.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegularExpression(QStringLiteral("^#{1,6}\\s.*"));
    rule.pattern.setPatternOptions(QRegularExpression::MultilineOption);
    rule.format = headerFormat;
    highlightingRules.append(rule);
    boldFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("(\\*\\*|__)(.*?)\\1"));
    rule.format = boldFormat;
    highlightingRules.append(rule);
    italicFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(QStringLiteral("(?<!\\*)\\*(?!\\*)|(?<!_)_(?!_)"));
    rule.pattern = QRegularExpression(QStringLiteral("(\\*|_)(.*?)\\1"));
    rule.format = italicFormat;
    highlightingRules.append(rule);
    listFormat.setForeground(QColor(0, 128, 0));
    rule.pattern = QRegularExpression(QStringLiteral("^[ \t]*([*+-]|\\d+\\.)\\s"));
    rule.pattern.setPatternOptions(QRegularExpression::MultilineOption);
    rule.format = listFormat;
    highlightingRules.append(rule);
    linkFormat.setForeground(Qt::blue);
    linkFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression(QStringLiteral("\\[.*?\\]\\(.*?\\)"));
    rule.format = linkFormat;
    highlightingRules.append(rule);
    quoteFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("^[ \t]*>.*"));
    rule.pattern.setPatternOptions(QRegularExpression::MultilineOption);
    rule.format = quoteFormat;
    highlightingRules.append(rule);
    codeFormat.setForeground(QColor(150, 75, 0));
    codeFormat.setFontFamilies({QStringLiteral("monospace")});
    rule.pattern = QRegularExpression(QStringLiteral("`.*?`"));
    rule.format = codeFormat;
    highlightingRules.append(rule);
    commentStartExpression = QRegularExpression(QStringLiteral("^```"));
    commentEndExpression = QRegularExpression(QStringLiteral("^```"));
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

void MarkdownHighlighter::setErrorList(const QList<SpellError> &errors)
{
    this->errors = errors;
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    int blockStart = currentBlock().position();
    for(const auto& error : std::as_const(errors))
    {
        if(error.start >= blockStart && error.start < blockStart + text.length())
            setFormat(error.start - blockStart, error.length, errorFormat);
    }

    for(const HighlightingRule &rule : std::as_const(highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while(matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if(previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while(startIndex >= 0)
    {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex;
        int commentLength;

        if(!endMatch.hasMatch())
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            endIndex = endMatch.capturedStart();
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, codeFormat);

        if(currentBlockState() == 0)
            startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
        else
            break;
    }
}
