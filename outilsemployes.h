#ifndef OUTILSEMPLOYES_H
#define OUTILSEMPLOYES_H

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVector>

/*
 * Petites fonctions partagées par le module Employés.
 * Elles sont volontairement simples et sans dépendance à QtWidgets.
 */

/* Accepte les dates ISO (2026-10-07), françaises (07/10/2026) et leurs
 * variantes avec une heure. Les anciennes données du projet sont dans les
 * deux formats : on les normalise à la lecture. */
inline QDate dateDepuisTexte(const QString &texte)
{
    const QString valeur = texte.trimmed();
    if (valeur.isEmpty()) {
        return QDate();
    }

    const QString dateSeule = valeur.section(' ', 0, 0).trimmed();
    const QStringList formats = {
        "yyyy-MM-dd", "dd/MM/yyyy", "yyyy/MM/dd", "dd-MM-yyyy", "dd.MM.yyyy"
    };

    for (const QString &format : formats) {
        const QDate date = QDate::fromString(dateSeule, format);
        if (date.isValid()) {
            return date;
        }
    }
    return QDate();
}

inline QString dateIso(const QDate &date)
{
    return date.isValid() ? date.toString("yyyy-MM-dd") : QString();
}

inline QString dateAffichage(const QString &texte)
{
    const QDate date = dateDepuisTexte(texte);
    return date.isValid() ? date.toString("dd/MM/yyyy") : texte;
}

inline QString dateAffichage(const QDate &date)
{
    return date.isValid() ? date.toString("dd/MM/yyyy") : QString();
}

inline QString dateHeureAffichage(const QString &texte)
{
    const QString valeur = texte.trimmed();
    const QStringList formats = {
        "yyyy-MM-dd HH:mm:ss", "dd/MM/yyyy HH:mm:ss",
        "yyyy-MM-dd HH:mm", "dd/MM/yyyy HH:mm"
    };
    for (const QString &format : formats) {
        const QDateTime moment = QDateTime::fromString(valeur, format);
        if (moment.isValid()) {
            return moment.toString("dd/MM/yyyy HH:mm");
        }
    }
    return dateAffichage(valeur);
}

/* Jours ouvrables (lundi à vendredi), bornes incluses. */
inline int joursOuvrables(const QDate &debut, const QDate &fin)
{
    if (!debut.isValid() || !fin.isValid() || fin < debut) {
        return 0;
    }

    int total = 0;
    QDate jour = debut;
    int securite = 0;
    while (jour <= fin && securite < 3700) {
        if (jour.dayOfWeek() <= 5) {
            ++total;
        }
        jour = jour.addDays(1);
        ++securite;
    }
    return total;
}

/* Le projet "files" utilise "Approuvé" ; le module actuel utilise
 * "Accepté". On harmonise uniquement pour l'affichage et les calculs. */
inline QString statutCongeNormalise(const QString &statut)
{
    const QString valeur = statut.trimmed();
    if (valeur.compare("Approuvé", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Approuvee", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Approuvée", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Accepte", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Acceptée", Qt::CaseInsensitive) == 0) {
        return "Accepté";
    }
    if (valeur.compare("Refusee", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Refusée", Qt::CaseInsensitive) == 0) {
        return "Refusé";
    }
    if (valeur.compare("Annulee", Qt::CaseInsensitive) == 0 ||
        valeur.compare("Annulée", Qt::CaseInsensitive) == 0) {
        return "Annulé";
    }
    return valeur;
}

inline bool estCongeActif(const QString &statut)
{
    const QString normalise = statutCongeNormalise(statut);
    return normalise == "Accepté" || normalise == "Approuvé";
}

inline QString salaireTexte(double salaire)
{
    const QLocale francais(QLocale::French, QLocale::France);
    return francais.toString(salaire, 'f', 2) + " DT";
}

inline QString initiales(const QString &nom, const QString &prenom)
{
    QString resultat;
    const QString n = nom.trimmed();
    const QString p = prenom.trimmed();
    if (!n.isEmpty()) resultat += n.at(0).toUpper();
    if (!p.isEmpty()) resultat += p.at(0).toUpper();
    return resultat.isEmpty() ? QString("?") : resultat;
}

inline QColor couleurAvatar(const QString &nom, const QString &prenom)
{
    static const QVector<QColor> palette = {
        QColor("#179BAE"), QColor("#137A6E"), QColor("#2E6F9E"),
        QColor("#7A5AA6"), QColor("#C47A24"), QColor("#1B3346")
    };
    const int index = int(qHash(nom + "|" + prenom) % quint32(palette.size()));
    return palette.at(index);
}

inline int pourcentage(int partie, int total)
{
    return total > 0 ? qRound(partie * 100.0 / total) : 0;
}

inline QStringList fusionnerListes(const QStringList &premiere,
                                   const QStringList &seconde)
{
    QStringList resultat = premiere;
    for (const QString &valeur : seconde) {
        if (!valeur.trimmed().isEmpty() &&
            !resultat.contains(valeur, Qt::CaseInsensitive)) {
            resultat << valeur;
        }
    }
    return resultat;
}

#endif // OUTILSEMPLOYES_H
