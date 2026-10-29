#include "connection.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("SWM");
    QCoreApplication::setApplicationName("SmartWeatherManagement");

    // Police par défaut moderne et épurée
    QFont appFont("Segoe UI", 9);
    a.setFont(appFont);

    Connection connexion;
    if (!connexion.createConnection()) {
        QMessageBox::critical(nullptr, "Connexion à la base de données",
                              "Impossible d'ouvrir la base de données :\n\n"
                                  + connexion.messageErreur());
        return 1;
    }

    MainWindow fenetre;
    fenetre.setWindowTitle("Smart Weather Management — Gestion des Employés  ["
                           + connexion.description() + "]");
    fenetre.show();

    return a.exec();
}
