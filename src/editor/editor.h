#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>

class Editor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Editor(QWidget *parent = nullptr);
    Editor(const QString& text, const QString& path, QWidget* parent = nullptr);
    QString getPath() const;
    void setPath(const QString &newPath);
    int lineNumberWidth();
    void lineNumberAreaPaint(QPaintEvent* event);
    bool getSaved() const;
    void setSaved(bool newSaved);
    bool isSaved();
    QString getName() const;
    void setName(const QString &newName);
    void find(const QString& text);
    void replace(const QString& findText, const QString& replaceText);
    void clearSearchFormatting();
    void increaseFontSize();
    void decreaseFontSize();
    void setFontSize(int size);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    class LineNumberArea : public QWidget
    {
    public:
        LineNumberArea(Editor* parent = nullptr);
    public:
        QSize sizeHint() const override;
    protected:
        void paintEvent(QPaintEvent *event) override;
    private:
        Editor* textEditor;
    };
    QString path;
    LineNumberArea* lineNumberArea;
    bool saved = true;
    QString name;
    QString oldContent = "";
    void init();
    void updateLineNumberWidth(int count);
    void updateLineNumber(const QRect &rect, int dy);
    QTextCharFormat defaultFormat;

signals:
    void fontSizeChanged(int size);
};

#endif // EDITOR_H
