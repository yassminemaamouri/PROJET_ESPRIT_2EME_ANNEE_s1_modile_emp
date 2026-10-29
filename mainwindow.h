#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "icones.h"

#include <QMainWindow>

class QPushButton;
class QStackedWidget;
class VueAccueil;
class VueEmployes;
class VueServices;
class VueRapports;
class FicheDetailleeEmploye;

/*
 * Fenêtre principale du module Employés.
 *
 * La présentation reprend le shell historique de Smart Weather Management :
 * barre latérale bleu nuit de 220 px, navigation verticale et pages internes.
 * Les cinq modules demandés sont visibles ; ceux qui n'ont pas encore de page
 * dédiée sont signalés comme indisponibles au lieu d'être simulés.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void slotOuvrirFiche(int id);
    void slotRetourEmployes();
    void slotDonneesModifiees();
    void slotFiltrerEmployes(const QString &service);

private:
    enum PageInterne {
        PageAccueil = 0,
        PageEmployes = 1,
        PageFiche = 2,
        PageServices = 3,
        PageRapports = 4
    };

    QWidget *creerBarreLaterale();
    QPushButton *creerBoutonMenu(const QString &texte, Icones::Type iconeType);
    void afficherPage(int page);
    void mettreAJourNavigation(int page);
    void rafraichirPagesSecondaires();

    QStackedWidget *m_pages = nullptr;
    VueAccueil *m_pageAccueil = nullptr;
    VueEmployes *m_pageEmployes = nullptr;
    FicheDetailleeEmploye *m_pageFiche = nullptr;
    VueServices *m_pageServices = nullptr;
    VueRapports *m_pageRapports = nullptr;

    QPushButton *m_btnAccueil = nullptr;
    QPushButton *m_btnEmployes = nullptr;
    QPushButton *m_btnStation = nullptr;
    QPushButton *m_btnEquipement = nullptr;
    QPushButton *m_btnIntervention = nullptr;

    int m_idFicheCourante = 0;
};

#endif // MAINWINDOW_H
