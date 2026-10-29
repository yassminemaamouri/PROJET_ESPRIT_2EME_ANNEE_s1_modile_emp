#include "vueaccueil.h"

#include "stylesemployes.h"

VueAccueil::VueAccueil(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageAccueil");
    setStyleSheet(StylesEmployes::feuille());
}

void VueAccueil::rechargerDonnees()
{
    // Page d'accueil vide : aucune donnée n'est affichée pour le moment.
}
