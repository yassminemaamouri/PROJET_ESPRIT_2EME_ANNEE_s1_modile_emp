#include "employe.h"

#include "connection.h"
#include "historiqueemploye.h"
#include "outilsemployes.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextDocument>
#include <QVariant>

#include <algorithm>

namespace {

QString champ(const QString &valeur)
{
    return valeur.trimmed();
}

QString erreurSql(const QSqlQuery &q)
{
    return q.lastError().text().trimmed();
}

void affecterErreur(QString *destination, const QString &message)
{
    if (destination) {
        *destination = message;
    }
}

QString dateNormalisee(const QString &texte)
{
    return dateIso(dateDepuisTexte(texte));
}

QString htmlEchappe(const QString &texte)
{
    return texte.toHtmlEscaped();
}

bool ecrirePdf(const QString &chemin, QTextDocument &document, bool paysage)
{
    if (chemin.trimmed().isEmpty()) {
        return false;
    }

    const QFileInfo info(chemin);
    if (!QDir().mkpath(info.absolutePath())) {
        return false;
    }

    QPdfWriter writer(chemin);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(paysage ? QPageLayout::Landscape : QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    writer.setResolution(300);
    writer.setTitle("Smart Weather Management");
    writer.setCreator("Smart Weather Management");

    document.setPageSize(QSizeF(writer.width(), writer.height()));
    document.print(&writer);
    return QFileInfo::exists(chemin);
}

} // namespace

QStringList Employe::statuts()
{
    return {"Actif", "En congé", "Inactif"};
}

QStringList Employe::postesParDefaut()
{
    return {"Ingénieur", "Responsable RH", "Administrateur", "Technicien",
            "Agent d'observation", "Chef de service", "Assistant", "Stagiaire"};
}

QStringList Employe::servicesParDefaut()
{
    return {"Informatique et données", "Ressources humaines",
            "Observation et réseau de stations", "Climatologie",
            "Météorologie", "Prévisions", "Direction"};
}

bool Employe::nomValide(const QString &texte)
{
    const QString valeur = texte.trimmed();
    if (valeur.size() < 2 || valeur.size() > 50) {
        return false;
    }
    static const QRegularExpression regex(
        QStringLiteral("^[\\p{L}][\\p{L}\\p{M} '\\-]{1,49}$"));
    return regex.match(valeur).hasMatch();
}

bool Employe::emailValide(const QString &email)
{
    const QString valeur = email.trimmed();
    if (valeur.size() > 100) {
        return false;
    }
    static const QRegularExpression regex(
        QStringLiteral("^[A-Za-z0-9._%+\\-]+@[A-Za-z0-9.\\-]+\\.[A-Za-z]{2,}$"));
    return regex.match(valeur).hasMatch();
}

bool Employe::telephoneValide(const QString &telephone)
{
    const QString valeur = telephone.trimmed();
    if (valeur.size() < 8 || valeur.size() > 25) {
        return false;
    }
    static const QRegularExpression regex(
        QStringLiteral("^[0-9+()\\- .]+$"));
    if (!regex.match(valeur).hasMatch()) {
        return false;
    }

    int chiffres = 0;
    for (const QChar c : valeur) {
        if (c.isDigit()) {
            ++chiffres;
        }
    }
    return chiffres >= 8 && chiffres <= 15;
}

bool Employe::dateValide(const QString &date)
{
    const QDate valeur = dateDepuisTexte(date);
    return valeur.isValid() && valeur <= QDate::currentDate();
}

bool Employe::salaireValide(double salaire)
{
    return salaire > 0.0 && salaire <= 1000000.0;
}

bool Employe::soldeValide(int solde)
{
    return solde >= 0 && solde <= 365;
}

QVector<Employe> Employes::chargerTous()
{
    QVector<Employe> resultat;
    QSqlQuery q;
    if (!q.exec("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, POSTE, MOT_DE_PASSE, "
                "SERVICE, DATE_RECRUTEMENT, SALAIRE, SOLDE_CONGES, STATUT, PHOTO "
                "FROM EMPLOYES ORDER BY ID")) {
        return resultat;
    }

    while (q.next()) {
        Employe e;
        e.id = q.value(0).toInt();
        e.nom = q.value(1).toString();
        e.prenom = q.value(2).toString();
        e.email = q.value(3).toString();
        e.telephone = q.value(4).toString();
        e.poste = q.value(5).toString();
        e.motDePasse = q.value(6).toString();
        e.service = q.value(7).toString();
        e.dateRecrutement = q.value(8).toString();
        e.salaire = q.value(9).toDouble();
        e.soldeConges = q.value(10).isNull() ? 30 : q.value(10).toInt();
        e.statut = q.value(11).toString().trimmed().isEmpty()
                   ? QString("Actif") : q.value(11).toString();
        e.photo = q.value(12).toString();
        resultat.append(e);
    }
    return resultat;
}

Employe Employes::charger(int id)
{
    Employe e;
    if (id <= 0) {
        return e;
    }

    QSqlQuery q;
    q.prepare("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, POSTE, MOT_DE_PASSE, "
              "SERVICE, DATE_RECRUTEMENT, SALAIRE, SOLDE_CONGES, STATUT, PHOTO "
              "FROM EMPLOYES WHERE ID = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        return e;
    }

    e.id = q.value(0).toInt();
    e.nom = q.value(1).toString();
    e.prenom = q.value(2).toString();
    e.email = q.value(3).toString();
    e.telephone = q.value(4).toString();
    e.poste = q.value(5).toString();
    e.motDePasse = q.value(6).toString();
    e.service = q.value(7).toString();
    e.dateRecrutement = q.value(8).toString();
    e.salaire = q.value(9).toDouble();
    e.soldeConges = q.value(10).isNull() ? 30 : q.value(10).toInt();
    e.statut = q.value(11).toString().trimmed().isEmpty()
               ? QString("Actif") : q.value(11).toString();
    e.photo = q.value(12).toString();
    return e;
}

bool Employes::ajouter(Employe &employe, QString *erreur)
{
    employe.nom = champ(employe.nom);
    employe.prenom = champ(employe.prenom);
    employe.email = champ(employe.email);
    employe.telephone = champ(employe.telephone);
    employe.poste = champ(employe.poste);
    employe.service = champ(employe.service);
    employe.statut = champ(employe.statut).isEmpty() ? "Actif" : champ(employe.statut);
    employe.photo = champ(employe.photo);

    if (!Employe::nomValide(employe.nom)) {
        affecterErreur(erreur, "Le nom doit contenir au moins 2 lettres, sans chiffres.");
        return false;
    }
    if (!Employe::nomValide(employe.prenom)) {
        affecterErreur(erreur, "Le prénom doit contenir au moins 2 lettres, sans chiffres.");
        return false;
    }
    if (!Employe::emailValide(employe.email)) {
        affecterErreur(erreur, "L'adresse e-mail n'est pas valide.");
        return false;
    }
    if (emailExiste(employe.email)) {
        affecterErreur(erreur, "Cette adresse e-mail est déjà utilisée par un employé.");
        return false;
    }
    if (!Employe::telephoneValide(employe.telephone)) {
        affecterErreur(erreur, "Le téléphone doit contenir de 8 à 15 chiffres.");
        return false;
    }
    if (employe.poste.isEmpty()) {
        affecterErreur(erreur, "Le poste est obligatoire.");
        return false;
    }
    if (employe.service.isEmpty()) {
        affecterErreur(erreur, "Le service est obligatoire.");
        return false;
    }
    if (!Employe::dateValide(employe.dateRecrutement)) {
        affecterErreur(erreur, "La date de recrutement est invalide ou se trouve dans le futur.");
        return false;
    }
    if (!Employe::salaireValide(employe.salaire)) {
        affecterErreur(erreur, "Le salaire doit être supérieur à 0 et inférieur à 1 000 000 DT.");
        return false;
    }
    if (!Employe::soldeValide(employe.soldeConges)) {
        affecterErreur(erreur, "Le solde de congés doit être compris entre 0 et 365 jours.");
        return false;
    }
    if (employe.motDePasse.trimmed().isEmpty()) {
        affecterErreur(erreur, "Le mot de passe est obligatoire pour un nouvel employé.");
        return false;
    }

    employe.id = prochainId();
    const QString hash = hasherMotDePasse(employe.motDePasse);
    const QString dateIsoRecrutement = dateNormalisee(employe.dateRecrutement);

    QSqlQuery q;
    q.prepare("INSERT INTO EMPLOYES "
              "(ID, NOM, PRENOM, EMAIL, TELEPHONE, POSTE, MOT_DE_PASSE, SERVICE, "
              " DATE_RECRUTEMENT, SALAIRE, SOLDE_CONGES, STATUT, PHOTO) "
              "VALUES (:id, :nom, :prenom, :email, :tel, :poste, :mdp, :service, "
              "        :date, :salaire, :solde, :statut, :photo)");
    q.bindValue(":id", employe.id);
    q.bindValue(":nom", employe.nom);
    q.bindValue(":prenom", employe.prenom);
    q.bindValue(":email", employe.email);
    q.bindValue(":tel", employe.telephone);
    q.bindValue(":poste", employe.poste);
    q.bindValue(":mdp", hash);
    q.bindValue(":service", employe.service);
    q.bindValue(":date", dateIsoRecrutement);
    q.bindValue(":salaire", employe.salaire);
    q.bindValue(":solde", employe.soldeConges);
    q.bindValue(":statut", employe.statut);
    q.bindValue(":photo", employe.photo);

    if (!q.exec()) {
        affecterErreur(erreur, "Ajout impossible : " + erreurSql(q));
        return false;
    }

    employe.motDePasse = hash;
    employe.dateRecrutement = dateIsoRecrutement;

    HistoriqueEmploye::enregistrer(
        employe.id, "AJOUT",
        QString("Ajout de l'employé %1 %2").arg(employe.prenom, employe.nom),
        QString(),
        QString("Poste : %1 | Service : %2 | Statut : %3")
            .arg(employe.poste, employe.service, employe.statut));
    return true;
}

bool Employes::modifier(const Employe &employe, QString *erreur)
{
    Employe ancien = charger(employe.id);
    if (ancien.id == 0) {
        affecterErreur(erreur, "Employé introuvable.");
        return false;
    }

    Employe nouveau = employe;
    nouveau.nom = champ(nouveau.nom);
    nouveau.prenom = champ(nouveau.prenom);
    nouveau.email = champ(nouveau.email);
    nouveau.telephone = champ(nouveau.telephone);
    nouveau.poste = champ(nouveau.poste);
    nouveau.service = champ(nouveau.service);
    nouveau.statut = champ(nouveau.statut).isEmpty() ? "Actif" : champ(nouveau.statut);
    nouveau.photo = champ(nouveau.photo);

    if (!Employe::nomValide(nouveau.nom)) {
        affecterErreur(erreur, "Le nom doit contenir au moins 2 lettres, sans chiffres.");
        return false;
    }
    if (!Employe::nomValide(nouveau.prenom)) {
        affecterErreur(erreur, "Le prénom doit contenir au moins 2 lettres, sans chiffres.");
        return false;
    }
    if (!Employe::emailValide(nouveau.email)) {
        affecterErreur(erreur, "L'adresse e-mail n'est pas valide.");
        return false;
    }
    if (emailExiste(nouveau.email, nouveau.id)) {
        affecterErreur(erreur, "Cette adresse e-mail est déjà utilisée par un autre employé.");
        return false;
    }
    if (!Employe::telephoneValide(nouveau.telephone)) {
        affecterErreur(erreur, "Le téléphone doit contenir de 8 à 15 chiffres.");
        return false;
    }
    if (nouveau.poste.isEmpty()) {
        affecterErreur(erreur, "Le poste est obligatoire.");
        return false;
    }
    if (nouveau.service.isEmpty()) {
        affecterErreur(erreur, "Le service est obligatoire.");
        return false;
    }
    if (!Employe::dateValide(nouveau.dateRecrutement)) {
        affecterErreur(erreur, "La date de recrutement est invalide ou se trouve dans le futur.");
        return false;
    }
    if (!Employe::salaireValide(nouveau.salaire)) {
        affecterErreur(erreur, "Le salaire doit être supérieur à 0 et inférieur à 1 000 000 DT.");
        return false;
    }
    if (!Employe::soldeValide(nouveau.soldeConges)) {
        affecterErreur(erreur, "Le solde de congés doit être compris entre 0 et 365 jours.");
        return false;
    }

    const bool motDePasseModifie = !nouveau.motDePasse.trimmed().isEmpty();
    const QString hash = motDePasseModifie
        ? hasherMotDePasse(nouveau.motDePasse)
        : ancien.motDePasse;
    const QString dateIsoRecrutement = dateNormalisee(nouveau.dateRecrutement);

    QSqlQuery q;
    q.prepare("UPDATE EMPLOYES SET "
              "NOM = :nom, PRENOM = :prenom, EMAIL = :email, TELEPHONE = :tel, "
              "POSTE = :poste, MOT_DE_PASSE = :mdp, SERVICE = :service, "
              "DATE_RECRUTEMENT = :date, SALAIRE = :salaire, "
              "SOLDE_CONGES = :solde, STATUT = :statut, PHOTO = :photo "
              "WHERE ID = :id");
    q.bindValue(":nom", nouveau.nom);
    q.bindValue(":prenom", nouveau.prenom);
    q.bindValue(":email", nouveau.email);
    q.bindValue(":tel", nouveau.telephone);
    q.bindValue(":poste", nouveau.poste);
    q.bindValue(":mdp", hash);
    q.bindValue(":service", nouveau.service);
    q.bindValue(":date", dateIsoRecrutement);
    q.bindValue(":salaire", nouveau.salaire);
    q.bindValue(":solde", nouveau.soldeConges);
    q.bindValue(":statut", nouveau.statut);
    q.bindValue(":photo", nouveau.photo);
    q.bindValue(":id", nouveau.id);

    if (!q.exec()) {
        affecterErreur(erreur, "Modification impossible : " + erreurSql(q));
        return false;
    }

    QStringList changements;
    if (ancien.nom != nouveau.nom) changements << "nom";
    if (ancien.prenom != nouveau.prenom) changements << "prénom";
    if (ancien.email != nouveau.email) changements << "e-mail";
    if (ancien.telephone != nouveau.telephone) changements << "téléphone";
    if (ancien.poste != nouveau.poste) changements << "poste";
    if (ancien.service != nouveau.service) changements << "service";
    if (dateDepuisTexte(ancien.dateRecrutement) != dateDepuisTexte(nouveau.dateRecrutement)) {
        changements << "date de recrutement";
    }
    if (!qFuzzyCompare(ancien.salaire + 1.0, nouveau.salaire + 1.0)) {
        changements << "salaire";
    }
    if (ancien.soldeConges != nouveau.soldeConges) changements << "solde de congés";
    if (motDePasseModifie) changements << "mot de passe";

    if (!changements.isEmpty() || ancien.statut != nouveau.statut) {
        HistoriqueEmploye::enregistrer(
            nouveau.id, "MODIFICATION",
            QString("Modification de la fiche de %1 %2")
                .arg(nouveau.prenom, nouveau.nom),
            QString(),
            "Champs modifiés : " + (changements.isEmpty()
                                     ? QString("statut")
                                     : changements.join(", ")));
    }
    if (ancien.statut != nouveau.statut) {
        HistoriqueEmploye::enregistrer(
            nouveau.id, "STATUT", "Changement de statut",
            ancien.statut, nouveau.statut);
    }
    return true;
}

bool Employes::desactiver(int id, QString *erreur)
{
    const Employe e = charger(id);
    if (e.id == 0) {
        affecterErreur(erreur, "Employé introuvable.");
        return false;
    }
    if (e.statut.compare("Inactif", Qt::CaseInsensitive) == 0) {
        return true;
    }

    QSqlQuery q;
    q.prepare("UPDATE EMPLOYES SET STATUT = 'Inactif' WHERE ID = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        affecterErreur(erreur, "Désactivation impossible : " + erreurSql(q));
        return false;
    }

    HistoriqueEmploye::enregistrer(
        id, "DESACTIVATION",
        QString("Désactivation de l'employé %1 %2").arg(e.prenom, e.nom),
        e.statut, "Inactif");
    return true;
}

bool Employes::emailExiste(const QString &email, int idExclu)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM EMPLOYES "
              "WHERE LOWER(EMAIL) = LOWER(:email) AND ID <> :id");
    q.bindValue(":email", email.trimmed());
    q.bindValue(":id", idExclu);
    if (!q.exec() || !q.next()) {
        return false;
    }
    return q.value(0).toInt() > 0;
}

int Employes::prochainId()
{
    QSqlQuery q;
    if (q.exec("SELECT COALESCE(MAX(ID), 0) + 1 FROM EMPLOYES") && q.next()) {
        return q.value(0).toInt();
    }
    return 1;
}

StatistiquesEmployes Employes::statistiques()
{
    StatistiquesEmployes stats;
    stats.recrutementsParMois = QVector<int>(12, 0);

    const QVector<Employe> employes = chargerTous();
    stats.total = employes.size();

    const QDate aujourdHui = QDate::currentDate();
    for (const Employe &e : employes) {
        if (e.statut.compare("Actif", Qt::CaseInsensitive) == 0) {
            ++stats.actifs;
        } else if (e.statut.compare("En congé", Qt::CaseInsensitive) == 0 ||
                   e.statut.compare("En conge", Qt::CaseInsensitive) == 0) {
            ++stats.enConge;
        } else {
            ++stats.inactifs;
        }

        const QDate recrutement = dateDepuisTexte(e.dateRecrutement);
        if (recrutement.isValid() && recrutement.year() == aujourdHui.year()) {
            const int mois = recrutement.month() - 1;
            if (mois >= 0 && mois < 12) {
                ++stats.recrutementsParMois[mois];
                if (mois == aujourdHui.month() - 1) {
                    ++stats.nouveauxCeMois;
                }
            }
        }
    }

    QVector<QPair<QString, int>> services;
    QVector<QPair<QString, int>> postes;
    for (const Employe &e : employes) {
        const QString service = e.service.trimmed().isEmpty()
            ? QString("Non renseigné") : e.service.trimmed();
        const QString poste = e.poste.trimmed().isEmpty()
            ? QString("Non renseigné") : e.poste.trimmed();

        bool trouve = false;
        for (auto &paire : services) {
            if (paire.first == service) {
                ++paire.second;
                trouve = true;
                break;
            }
        }
        if (!trouve) services.append({service, 1});

        trouve = false;
        for (auto &paire : postes) {
            if (paire.first == poste) {
                ++paire.second;
                trouve = true;
                break;
            }
        }
        if (!trouve) postes.append({poste, 1});
    }

    std::sort(services.begin(), services.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first.localeAwareCompare(b.first) < 0;
              });
    std::sort(postes.begin(), postes.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first.localeAwareCompare(b.first) < 0;
              });

    stats.parService = services;
    stats.parPoste = postes;
    return stats;
}

QStringList Employes::servicesEnBase()
{
    QStringList resultat;
    QSqlQuery q;
    if (q.exec("SELECT DISTINCT SERVICE FROM EMPLOYES "
               "WHERE SERVICE IS NOT NULL AND SERVICE <> '' "
               "ORDER BY SERVICE")) {
        while (q.next()) {
            resultat << q.value(0).toString();
        }
    }
    return resultat;
}

QStringList Employes::postesEnBase()
{
    QStringList resultat;
    QSqlQuery q;
    if (q.exec("SELECT DISTINCT POSTE FROM EMPLOYES "
               "WHERE POSTE IS NOT NULL AND POSTE <> '' "
               "ORDER BY POSTE")) {
        while (q.next()) {
            resultat << q.value(0).toString();
        }
    }
    return resultat;
}

QString Employes::hasherMotDePasse(const QString &motDePasseClair)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(motDePasseClair.toUtf8(),
                                 QCryptographicHash::Sha256).toHex());
}

QString Employes::dossierPhotos()
{
    const QString dossier = Connection::dossierDonnees() + "/photos";
    QDir().mkpath(dossier);
    return dossier;
}

bool Employes::exporterListePDF(const QVector<Employe> &employes,
                                const QString &cheminFichier,
                                const QString &filtres)
{
    QString lignes;
    for (const Employe &e : employes) {
        lignes += QString(
            "<tr>"
            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>"
            "<td>%6</td><td>%7</td><td>%8</td><td>%9</td>"
            "<td>%10</td><td>%11</td>"
            "</tr>")
            .arg(QString::number(e.id),
                 htmlEchappe(e.nom), htmlEchappe(e.prenom),
                 htmlEchappe(e.email), htmlEchappe(e.telephone),
                 htmlEchappe(e.poste), htmlEchappe(e.service),
                 htmlEchappe(dateAffichage(e.dateRecrutement)),
                 htmlEchappe(salaireTexte(e.salaire)),
                 QString::number(e.soldeConges) + " j",
                 htmlEchappe(e.statut));
    }

    const QString sousTitre = filtres.trimmed().isEmpty()
        ? QString("Tous les employés")
        : filtres.trimmed();

    QTextDocument document;
    document.setHtml(QString(
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body "
        "style=\"font-family: Segoe UI, Arial; color:#1B3346;\">"
        "<h1 style=\"color:#183B4E; margin-bottom:2px;\">Smart Weather Management</h1>"
        "<h2 style=\"color:#179BAE; margin-top:0;\">Liste des employés</h2>"
        "<p style=\"color:#5B6B79;\">%1 — %2 employé(s) — généré le %3</p>"
        "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"5\" "
        "style=\"border-collapse:collapse; font-size:8pt;\">"
        "<thead><tr style=\"background:#183B4E; color:white;\">"
        "<th>ID</th><th>Nom</th><th>Prénom</th><th>E-mail</th><th>Téléphone</th>"
        "<th>Poste</th><th>Service</th><th>Recrutement</th><th>Salaire</th>"
        "<th>Solde</th><th>Statut</th></tr></thead><tbody>%4</tbody></table>"
        "</body></html>")
        .arg(htmlEchappe(sousTitre))
        .arg(employes.size())
        .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"))
        .arg(lignes));
    return ecrirePdf(cheminFichier, document, true);
}

bool Employes::exporterFichePDF(const Employe &employe, const QString &cheminFichier)
{
    auto ligne = [](const QString &label, const QString &valeur) {
        return QString("<tr><td style=\"color:#5B6B79; width:35%%;\">%1</td>"
                       "<td style=\"font-weight:bold;\">%2</td></tr>")
            .arg(htmlEchappe(label), htmlEchappe(valeur));
    };

    QTextDocument document;
    document.setHtml(QString(
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body "
        "style=\"font-family: Segoe UI, Arial; color:#1B3346;\">"
        "<h1 style=\"color:#183B4E; margin-bottom:2px;\">Smart Weather Management</h1>"
        "<h2 style=\"color:#179BAE; margin-top:0;\">Fiche employé</h2>"
        "<h3>%1 %2</h3>"
        "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"6\" "
        "style=\"border-collapse:collapse; font-size:10pt;\">"
        "%3%4%5%6%7%8%9%10%11%12%13"
        "</table>"
        "<p style=\"margin-top:18px; color:#5B6B79; font-size:8pt;\">"
        "Document généré le %14.</p>"
        "</body></html>")
        .arg(htmlEchappe(employe.prenom), htmlEchappe(employe.nom),
             ligne("Identifiant", QString::number(employe.id)),
             ligne("E-mail", employe.email),
             ligne("Téléphone", employe.telephone),
             ligne("Poste", employe.poste),
             ligne("Service", employe.service),
             ligne("Date de recrutement", dateAffichage(employe.dateRecrutement)),
             ligne("Salaire", salaireTexte(employe.salaire)),
             ligne("Solde de congés (droit annuel)", QString::number(employe.soldeConges) + " jours"),
             ligne("Statut", employe.statut),
             ligne("Photo", employe.photo.isEmpty() ? "Non renseignée" : employe.photo),
             ligne("Mot de passe", employe.motDePasse.isEmpty() ? "Non défini" : "Défini (haché)"),
             QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")));
    return ecrirePdf(cheminFichier, document, false);
}
