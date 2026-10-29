#ifndef HISTORIQUEEMPLOYE_H
#define HISTORIQUEEMPLOYE_H

#include <QString>
#include <QVector>

/*
 * Une ligne du journal d'activité du module Employés.
 */
struct EntreeHistorique
{
    int id = 0;
    int employeId = 0;
    QString dateAction;
    QString typeAction;
    QString description;
    QString ancienneValeur;
    QString nouvelleValeur;
    QString utilisateur;
};

/*
 * Journal durable des actions sur les employés.
 *
 * Les lignes créées par le module Employés portent MODULE = 'EMPLOYE'.
 * Les anciennes lignes du module Clients (MODULE = 'CLIENT') sont ignorées
 * pour ne pas mélanger deux modules qui ont partagé la même base.
 */
class HistoriqueEmploye
{
public:
    static bool enregistrer(int employeId, const QString &typeAction,
                            const QString &description,
                            const QString &ancienneValeur = QString(),
                            const QString &nouvelleValeur = QString(),
                            const QString &utilisateur = QString("Administrateur"));

    static QVector<EntreeHistorique> chargerPourEmploye(int employeId);
    static QVector<EntreeHistorique> chargerTous(int limite = 0);

private:
    static bool assurerSchema();
};

#endif // HISTORIQUEEMPLOYE_H
