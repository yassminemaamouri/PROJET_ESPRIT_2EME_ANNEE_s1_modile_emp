#ifndef VUEACCUEIL_H
#define VUEACCUEIL_H

#include <QWidget>

/*
 * Page d'accueil volontairement vide.
 *
 * L'utilisateur ne souhaite pas de tableau de bord pour le moment : la
 * structure de l'application et la barre latérale restent inchangées.
 */
class VueAccueil : public QWidget
{
    Q_OBJECT

public:
    explicit VueAccueil(QWidget *parent = nullptr);

    void rechargerDonnees();

signals:
    void ouvrirEmploye(int id);
    void ouvrirEmployes();
};

#endif // VUEACCUEIL_H
