#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QKeySequence>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void search();
    void showSettings();
    void loadSettings();
    void saveSettings();

private:
    QPlainTextEdit *textEdit;
    QString currentFile;
    QSettings *settings;

    bool darkTheme;
    int fontSize;
    QFont textFont;
    QString currentFontFamily;

    void createActions();
    void applyTheme();
    bool maybeSave();
};

#endif