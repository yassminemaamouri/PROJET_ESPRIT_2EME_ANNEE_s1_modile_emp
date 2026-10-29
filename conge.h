#ifndef CONGE_H
#define CONGE_H

#include <QDate>
#include <QString>
#include <QStringList>
#include <QVector>

/*
 * Demande de congé d'un employé.
 *
 * Le solde stocké dans EMPLOYES.SOLDE_CONGES est le DROIT ANNUEL.
 * Les jours utilisés sont recalculés à partir des congés au statut
 * « Accepté » (ou « Approuvé » pour les anciennes données), ce qui évite
 * toute double déduction lors d'une acceptation.
 */
struct Conge
{
    int id = 0;
    int employeId = 0;
    QString typeConge;
    QDate dateDebut;
    QDate dateFin;
    int nbJours = 0;               // jours ouvrables (lundi → vendredi)
    QString statut = "En attente";
    QString dateDemande;
    QString motif;
    QString dateDecision;
    QString validePar;

    /* Renseignés par les requêtes avec jointure, pour le tableau de bord. */
    QString employeNom;
    QString employePrenom;

    bool ajouter(QString *erreur = nullptr);

    static QVector<Conge> chargerTous();
    static QVector<Conge> chargerEnAttente();
    static QVector<Conge> chargerPourEmploye(int employeId);
    static Conge charger(int id);

    static bool accepter(int id, QString *erreur = nullptr,
                         const QString &validePar = QString("Administrateur"));
    static bool refuser(int id, const QString &motifRefus = QString(),
                        QString *erreur = nullptr,
                        const QString &validePar = QString("Administrateur"));
    static bool annuler(int id, QString *erreur = nullptr);

    static int droitAnnuel(int employeId);
    static int joursUtilises(int employeId, int annee = 0);
    static int joursEnAttente(int employeId);
    static int joursRestants(int employeId, int annee = 0);

    static bool chevauche(int employeId, const QDate &debut, const QDate &fin,
                          int idExclu = 0, bool inclureEnAttente = true);
    static int joursOuvrables(const QDate &debut, const QDate &fin);

    static QStringList types();
    static QStringList statuts();
};

#endif // CONGE_H
