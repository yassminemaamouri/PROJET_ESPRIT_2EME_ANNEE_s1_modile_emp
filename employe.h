#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

/*
 * Employe : structure de données simple qui représente un employé.
 * Aucune requête SQL ici : c'est le rôle de la classe Employes.
 */
struct Employe
{
    int id = 0;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString poste;
    QString motDePasse;      // mot de passe haché (SHA-256), jamais affiché
    QString service;
    QString dateRecrutement; // format jj/MM/aaaa
    double salaire = 0.0;
    int soldeConges = 30;
    QString statut = "Actif";
    QString photo;           // chemin complet du fichier image

    QString nomComplet() const { return nom + " " + prenom; }

    static QStringList statuts();
    static QStringList postesParDefaut();
    static QStringList servicesParDefaut();

    static bool nomValide(const QString &texte);
    static bool emailValide(const QString &email);
    static bool telephoneValide(const QString &telephone);
    static bool dateValide(const QString &date);
    static bool salaireValide(double salaire);
    static bool soldeValide(int solde);
};

/* Résultat des statistiques calculées depuis la base de données. */
struct StatistiquesEmployes
{
    int total = 0;
    int actifs = 0;
    int enConge = 0;
    int inactifs = 0;
    int nouveauxCeMois = 0;
    QVector<int> recrutementsParMois;              // 12 valeurs : janvier → décembre
    QVector<QPair<QString, int>> parService;
    QVector<QPair<QString, int>> parPoste;
};

/*
 * Employes : accès à la base de données (CRUD, validation, statistiques, PDF).
 * Toutes les méthodes sont statiques : pas besoin d'instancier la classe.
 */
class Employes
{
public:
    static QVector<Employe> chargerTous();
    static Employe charger(int id);
    static bool ajouter(Employe &employe, QString *erreur = nullptr);
    static bool modifier(const Employe &employe, QString *erreur = nullptr);
    static bool desactiver(int id, QString *erreur = nullptr);

    static bool emailExiste(const QString &email, int idExclu = 0);
    static int prochainId();

    static StatistiquesEmployes statistiques();
    static QStringList servicesEnBase();
    static QStringList postesEnBase();

    static QString hasherMotDePasse(const QString &motDePasseClair);
    static QString dossierPhotos();

    static bool exporterListePDF(const QVector<Employe> &employes,
                                 const QString &cheminFichier,
                                 const QString &filtres = QString());
    static bool exporterFichePDF(const Employe &employe, const QString &cheminFichier);
};

#endif // EMPLOYE_H
