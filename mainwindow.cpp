#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFontDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QScrollBar>
#include <QTextCursor>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QComboBox>
#include <QStatusBar>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Настройки окна
    setWindowTitle("FastPad");
    resize(800, 600);

    // Создаём текстовое поле
    textEdit = new QPlainTextEdit(this);
    textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    setCentralWidget(textEdit);

    // Загружаем настройки
    settings = new QSettings("MrMeRKa", "FastPad");
    loadSettings();

    // Применяем тему
    applyTheme();

    // Создаём действия
    createActions();

    // Текущий файл
    currentFile = "";

    // Загружаем последний файл если есть
    QString lastFile = settings->value("lastFile", "").toString();
    if (!lastFile.isEmpty() && QFile::exists(lastFile)) {
        QFile file(lastFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
            file.close();
            currentFile = lastFile;
            setWindowTitle("FastPad - " + QFileInfo(lastFile).fileName());
            textEdit->document()->setModified(false);
        }
    }
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::createActions()
{
    // Новый файл - Ctrl+N
    QAction *newAction = new QAction("Новый", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    addAction(newAction);

    // Открыть - Ctrl+O
    QAction *openAction = new QAction("Открыть", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    addAction(openAction);

    // Сохранить - Ctrl+S
    QAction *saveAction = new QAction("Сохранить", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    addAction(saveAction);

    // Сохранить как - Ctrl+Shift+S
    QAction *saveAsAction = new QAction("Сохранить как", this);
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    addAction(saveAsAction);

    // Поиск - Ctrl+F
    QAction *searchAction = new QAction("Поиск", this);
    searchAction->setShortcut(QKeySequence::Find);
    connect(searchAction, &QAction::triggered, this, &MainWindow::search);
    addAction(searchAction);

    // Настройки - Ctrl+P
    QAction *settingsAction = new QAction("Настройки", this);
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    addAction(settingsAction);

    // Закрыть - Ctrl+W
    QAction *closeAction = new QAction("Закрыть", this);
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, &MainWindow::close);
    addAction(closeAction);

    // Выйти - Ctrl+Q
    QAction *quitAction = new QAction("Выйти", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    addAction(quitAction);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        currentFile = "";
        setWindowTitle("FastPad - Новый файл");
        textEdit->document()->setModified(false);
        statusBar()->showMessage("Новый файл создан", 2000);
    }
}

void MainWindow::openFile()
{
    if (!maybeSave()) return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл", "", "Все файлы (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream in(&file);
    textEdit->setPlainText(in.readAll());
    file.close();

    currentFile = fileName;
    setWindowTitle("FastPad - " + QFileInfo(fileName).fileName());
    textEdit->document()->setModified(false);
    settings->setValue("lastFile", currentFile);
    statusBar()->showMessage("Открыт: " + QFileInfo(fileName).fileName(), 2000);
}

void MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        saveAsFile();
        return;
    }

    QFile file(currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
        return;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();

    textEdit->document()->setModified(false);
    statusBar()->showMessage("💾 Сохранено: " + QFileInfo(currentFile).fileName(), 2000);
}

void MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить файл", "", "Все файлы (*)");

    if (fileName.isEmpty()) return;

    currentFile = fileName;
    QFile file(currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
        return;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();

    setWindowTitle("FastPad - " + QFileInfo(fileName).fileName());
    textEdit->document()->setModified(false);
    settings->setValue("lastFile", currentFile);
    statusBar()->showMessage("Сохранено как: " + QFileInfo(fileName).fileName(), 2000);
}

void MainWindow::search()
{
    QDialog searchDialog(this);
    searchDialog.setWindowTitle("Поиск");
    searchDialog.setModal(true);
    searchDialog.resize(400, 100);

    QVBoxLayout *layout = new QVBoxLayout(&searchDialog);
    QHBoxLayout *hLayout = new QHBoxLayout();

    QLineEdit *searchInput = new QLineEdit(&searchDialog);
    searchInput->setPlaceholderText("Введите текст для поиска...");
    hLayout->addWidget(searchInput);

    QPushButton *findButton = new QPushButton("Найти", &searchDialog);
    findButton->setDefault(true);
    hLayout->addWidget(findButton);

    QPushButton *closeButton = new QPushButton("Закрыть", &searchDialog);
    hLayout->addWidget(closeButton);

    layout->addLayout(hLayout);

    connect(findButton, &QPushButton::clicked, [this, searchInput, &searchDialog]() {
        QString text = searchInput->text();
        if (!text.isEmpty()) {
            QTextCursor cursor = textEdit->textCursor();
            cursor = textEdit->document()->find(text, cursor);
            if (!cursor.isNull()) {
                textEdit->setTextCursor(cursor);
                searchDialog.close();
            } else {
                cursor = textEdit->textCursor();
                cursor.movePosition(QTextCursor::Start);
                cursor = textEdit->document()->find(text, cursor);
                if (!cursor.isNull()) {
                    textEdit->setTextCursor(cursor);
                    searchDialog.close();
                } else {
                    QMessageBox::information(&searchDialog, "Не найдено",
                                             "Текст не найден");
                }
            }
        }
    });

    connect(closeButton, &QPushButton::clicked, &searchDialog, &QDialog::close);
    connect(searchInput, &QLineEdit::returnPressed, findButton, &QPushButton::click);

    searchDialog.exec();
}

void MainWindow::showSettings()
{
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle("Настройки FastPad");
    settingsDialog.setModal(true);
    settingsDialog.resize(450, 350);

    QVBoxLayout *layout = new QVBoxLayout(&settingsDialog);

    // Размер шрифта
    QHBoxLayout *fontSizeLayout = new QHBoxLayout();
    QLabel *fontSizeLabel = new QLabel("Размер шрифта:", &settingsDialog);
    QComboBox *fontSizeCombo = new QComboBox(&settingsDialog);
    for (int i = 8; i <= 24; i += 2) {
        fontSizeCombo->addItem(QString::number(i));
    }
    fontSizeCombo->setCurrentText(QString::number(fontSize));
    fontSizeLayout->addWidget(fontSizeLabel);
    fontSizeLayout->addWidget(fontSizeCombo);
    layout->addLayout(fontSizeLayout);

    // Тема
    QHBoxLayout *themeLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("Тема:", &settingsDialog);
    QComboBox *themeCombo = new QComboBox(&settingsDialog);
    themeCombo->addItems({"Светлая", "Тёмная"});
    themeCombo->setCurrentIndex(darkTheme ? 1 : 0);
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeCombo);
    layout->addLayout(themeLayout);

    // Кнопка "Выбрать шрифт"
    QPushButton *fontButton = new QPushButton("Выбрать другой шрифт...", &settingsDialog);
    layout->addWidget(fontButton);

    layout->addStretch();

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton("Сохранить", &settingsDialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &settingsDialog);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(fontButton, &QPushButton::clicked, [this]() {
        bool ok;
        QFont font = QFontDialog::getFont(&ok, textEdit->font(), this, "Выберите шрифт");
        if (ok) {
            fontSize = font.pointSize();
            textFont = font;
            textEdit->setFont(font);
            currentFontFamily = font.family();
            settings->setValue("customFontFamily", font.family());
        }
    });

    connect(saveButton, &QPushButton::clicked, [&]() {
        fontSize = fontSizeCombo->currentText().toInt();
        darkTheme = (themeCombo->currentIndex() == 1);

        if (currentFontFamily.isEmpty()) {
            QFontDatabase db;
            QStringList families = db.families();
            if (families.contains("JetBrains Mono")) {
                currentFontFamily = "JetBrains Mono";
            } else {
                currentFontFamily = "Monospace";
            }
        }

        textFont = QFont(currentFontFamily, fontSize);
        textEdit->setFont(textFont);
        applyTheme();

        settings->setValue("fontSize", fontSize);
        settings->setValue("darkTheme", darkTheme);
        settings->setValue("fontFamily", currentFontFamily);

        settingsDialog.accept();
        statusBar()->showMessage("✅ Настройки сохранены", 2000);
    });

    connect(cancelButton, &QPushButton::clicked, &settingsDialog, &QDialog::reject);

    settingsDialog.exec();
}

void MainWindow::loadSettings()
{
    darkTheme = settings->value("darkTheme", false).toBool();
    fontSize = settings->value("fontSize", 11).toInt();

    QString fontName = settings->value("fontFamily", "").toString();
    if (fontName.isEmpty()) {
        QFontDatabase db;
        QStringList families = db.families();
        if (families.contains("JetBrains Mono")) {
            fontName = "JetBrains Mono";
        } else {
            fontName = "Monospace";
        }
    }

    currentFontFamily = fontName;
    textFont = QFont(fontName, fontSize);
    textEdit->setFont(textFont);
}

void MainWindow::saveSettings()
{
    settings->setValue("fontSize", fontSize);
    settings->setValue("darkTheme", darkTheme);
    settings->setValue("fontFamily", currentFontFamily);
    settings->setValue("lastFile", currentFile);
    settings->sync();
}

void MainWindow::applyTheme()
{
    QString style;
    if (darkTheme) {
        style = R"(
            QMainWindow { background-color: #2b2b2b; }
            QPlainTextEdit {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: none;
            }
            QPlainTextEdit:focus { border: none; }
            QScrollBar:vertical {
                background: #2b2b2b;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background: #555;
                border-radius: 6px;
            }
            QScrollBar::handle:vertical:hover {
                background: #777;
            }
            QDialog {
                background-color: #2b2b2b;
                color: #d4d4d4;
            }
            QLabel { color: #d4d4d4; }
            QLineEdit {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: 1px solid #555;
                padding: 5px;
                border-radius: 3px;
            }
            QComboBox {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: 1px solid #555;
                padding: 5px;
                border-radius: 3px;
            }
            QPushButton {
                background-color: #3c3c3c;
                color: #d4d4d4;
                border: 1px solid #555;
                padding: 5px 15px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #4a4a4a;
            }
            QMessageBox {
                background-color: #2b2b2b;
                color: #d4d4d4;
            }
            QStatusBar {
                background-color: #1e1e1e;
                color: #888;
            }
        )";
    } else {
        style = R"(
            QMainWindow { background-color: #f0f0f0; }
            QPlainTextEdit {
                background-color: white;
                color: #000;
                border: none;
            }
            QPlainTextEdit:focus { border: none; }
            QScrollBar:vertical {
                background: #f0f0f0;
                width: 12px;
            }
            QScrollBar::handle:vertical {
                background: #c0c0c0;
                border-radius: 6px;
            }
            QScrollBar::handle:vertical:hover {
                background: #a0a0a0;
            }
            QDialog {
                background-color: #f0f0f0;
            }
            QLabel { color: #000; }
            QLineEdit {
                background-color: white;
                color: #000;
                border: 1px solid #ccc;
                padding: 5px;
                border-radius: 3px;
            }
            QComboBox {
                background-color: white;
                color: #000;
                border: 1px solid #ccc;
                padding: 5px;
                border-radius: 3px;
            }
            QPushButton {
                background-color: #e0e0e0;
                color: #000;
                border: 1px solid #ccc;
                padding: 5px 15px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #d0d0d0;
            }
            QStatusBar {
                background-color: #f0f0f0;
                color: #666;
            }
        )";
    }
    setStyleSheet(style);
}

bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified()) return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "Сохранить?",
                               "Документ был изменён. Сохранить?",
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        saveFile();
        return true;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}