#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Настройки приложения
    app.setOrganizationName("MrMeRKa");
    app.setApplicationName("FastPad");

    // Загружаем шрифт из ресурсов
    int fontId = QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");

    // Если шрифт загрузился, устанавливаем его как шрифт по умолчанию
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        // Применяем как шрифт по умолчанию для всего приложения
        QFont defaultFont(fontFamily, 11);
        app.setFont(defaultFont);
    } else {
        // Если загрузить не удалось, используем Monospace как запасной вариант
        app.setFont(QFont("Monospace", 11));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
