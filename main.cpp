#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Afficher les pilotes disponibles (utile pour debug)
    qDebug() << "Pilotes disponibles :" << QSqlDatabase::drivers();

    // Connexion Oracle via ODBC
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC"); // 🔹 Utilise bien QODBC
    db.setDatabaseName("test-bd");                        // 🔹 DSN configuré dans ODBC
    db.setUserName("wael");                             // 🔹 Utilisateur Oracle
    db.setPassword("wael");                             // 🔹 Mot de passe Oracle

    if (!db.open()) {
        QMessageBox::critical(nullptr, "Erreur de connexion",
                              "Connexion à Oracle échouée :\n" + db.lastError().text());
        return -1;
    }

    QMessageBox::information(nullptr, "Connexion réussie",
                             "Connexion à Oracle réussie via ODBC (DSN: test-bd)");

    MainWindow w;
    w.show();
    return a.exec();
}
