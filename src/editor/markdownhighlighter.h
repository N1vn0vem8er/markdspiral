#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);
    struct SpellError {
        int start;
        int length;
    };
    void setErrorList(const QList<MarkdownHighlighter::SpellError> &errors);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;
    QTextCharFormat headerFormat;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat listFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat quoteFormat;
    QTextCharFormat errorFormat;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QList<SpellError> errors;
};

#endif // MARKDOWNHIGHLIGHTER_H
