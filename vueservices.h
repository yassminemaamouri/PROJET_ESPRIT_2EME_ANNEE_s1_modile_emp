#ifndef VUESERVICES_H
#define VUESERVICES_H

#include <QWidget>

class QGridLayout;
class QLabel;
class QScrollArea;
class QVBoxLayout;

/*
 * Page « Services » : nombre d'employés, statuts et effectif de chaque
 * service. Un clic sur « Voir les employés » filtre la liste correspondante.
 */
class VueServices : public QWidget
{
    Q_OBJECT

public:
    explicit VueServices(QWidget *parent = nullptr);

    void rechargerDonnees();

signals:
    void ouvrirEmployes();
    void filtrerEmployesParService(const QString &service);

private:
    void creerInterface();
    void viderCartes();

    QScrollArea *m_zone = nullptr;
    QWidget *m_contenu = nullptr;
    QGridLayout *m_grille = nullptr;
    QLabel *m_vide = nullptr;
};

#endif // VUESERVICES_H
