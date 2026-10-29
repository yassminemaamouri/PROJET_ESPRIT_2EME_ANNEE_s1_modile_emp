#include "conge.h"

#include "historiqueemploye.h"
#include "outilsemployes.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <algorithm>

namespace {

QString erreurSql(const QSqlQuery &q)
{
    return q.lastError().text().trimmed();
}

Conge depuisRequete(const QSqlQuery &q, bool avecEmploye)
{
    Conge c;
    c.id = q.value(0).toInt();
    c.employeId = q.value(1).toInt();
    c.typeConge = q.value(2).toString();
    c.dateDebut = dateDepuisTexte(q.value(3).toString());
    c.dateFin = dateDepuisTexte(q.value(4).toString());
    c.nbJours = q.value(5).toInt();
    c.statut = statutCongeNormalise(q.value(6).toString());
    c.dateDemande = q.value(7).toString();
    c.motif = q.value(8).toString();
    c.dateDecision = q.value(9).toString();
    c.validePar = q.value(10).toString();
    if (avecEmploye) {
        c.employeNom = q.value(11).toString();
        c.employePrenom = q.value(12).toString();
    }
    if (c.nbJours <= 0) {
        c.nbJours = ::joursOuvrables(c.dateDebut, c.dateFin);
    }
    return c;
}

bool chargerAvecJointure(const QString &where, int id, QVector<Conge> &resultat)
{
    QSqlQuery q;
    QString sql =
        "SELECT C.ID, C.EMPLOYE_ID, C.TYPE_CONGE, C.DATE_DEBUT, C.DATE_FIN, "
        "C.NB_JOURS, C.STATUT, C.DATE_DEMANDE, C.MOTIF, C.DATE_DECISION, "
        "C.VALIDE_PAR, E.NOM, E.PRENOM "
        "FROM CONGES C LEFT JOIN EMPLOYES E ON E.ID = C.EMPLOYE_ID ";
    if (!where.isEmpty()) {
        sql += where + " ";
    }
    sql += "ORDER BY C.ID DESC";

    q.prepare(sql);
    if (id > 0) {
        q.bindValue(":id", id);
    }
    if (!q.exec()) {
        return false;
    }
    while (q.next()) {
        resultat.append(depuisRequete(q, true));
    }
    return true;
}

} // namespace

QVector<Conge> Conge::chargerTous()
{
    QVector<Conge> resultat;
    chargerAvecJointure(QString(), 0, resultat);
    return resultat;
}

QVector<Conge> Conge::chargerEnAttente()
{
    QVector<Conge> tous = chargerTous();
    QVector<Conge> resultat;
    for (const Conge &c : tous) {
        if (statutCongeNormalise(c.statut) == "En attente") {
            resultat.append(c);
        }
    }
    return resultat;
}

QVector<Conge> Conge::chargerPourEmploye(int employeId)
{
    QVector<Conge> resultat;
    if (employeId <= 0) {
        return resultat;
    }
    chargerAvecJointure("WHERE C.EMPLOYE_ID = :id", employeId, resultat);

    std::sort(resultat.begin(), resultat.end(),
              [](const Conge &a, const Conge &b) {
                  if (a.dateDebut != b.dateDebut) {
                      return a.dateDebut > b.dateDebut;
                  }
                  return a.id > b.id;
              });
    return resultat;
}

Conge Conge::charger(int id)
{
    QVector<Conge> resultat;
    if (id <= 0) {
        return Conge();
    }
    chargerAvecJointure("WHERE C.ID = :id", id, resultat);
    return resultat.isEmpty() ? Conge() : resultat.first();
}

bool Conge::ajouter(QString *erreur)
{
    if (employeId <= 0) {
        if (erreur) *erreur = "Aucun employé sélectionné.";
        return false;
    }

    QSqlQuery employe;
    employe.prepare("SELECT NOM, PRENOM, STATUT, SOLDE_CONGES "
                    "FROM EMPLOYES WHERE ID = :id");
    employe.bindValue(":id", employeId);
    if (!employe.exec() || !employe.next()) {
        if (erreur) *erreur = "Employé introuvable.";
        return false;
    }
    const QString statutEmploye = employe.value(2).toString();
    if (statutEmploye.compare("Inactif", Qt::CaseInsensitive) == 0) {
        if (erreur) *erreur = "Un employé inactif ne peut pas demander de congé.";
        return false;
    }

    const QDate debut = dateDebut;
    const QDate fin = dateFin;
    if (!debut.isValid() || !fin.isValid()) {
        if (erreur) *erreur = "Les dates de début et de fin sont obligatoires et doivent être valides.";
        return false;
    }
    if (fin < debut) {
        if (erreur) *erreur = "La date de fin doit être postérieure ou égale à la date de début.";
        return false;
    }

    const int jours = ::joursOuvrables(debut, fin);
    if (jours <= 0) {
        if (erreur) *erreur = "La période ne contient aucun jour ouvrable (lundi à vendredi).";
        return false;
    }

    if (typeConge.trimmed().isEmpty()) {
        typeConge = "Congé annuel";
    }

    if (chevauche(employeId, debut, fin, 0, true)) {
        if (erreur) *erreur = "Cette période chevauche une demande ou un congé déjà enregistré.";
        return false;
    }

    const int droit = employe.value(3).isNull() ? 30 : employe.value(3).toInt();
    const int utilises = joursUtilises(employeId, debut.year());
    if (utilises + jours > droit) {
        if (erreur) {
            *erreur = QString("Solde insuffisant : %1 jour(s) demandé(s), %2 jour(s) restant(s) "
                              "sur un droit annuel de %3 jour(s).")
                          .arg(jours)
                          .arg(qMax(0, droit - utilises))
                          .arg(droit);
        }
        return false;
    }

    QSqlQuery idQuery;
    if (!idQuery.exec("SELECT COALESCE(MAX(ID), 0) + 1 FROM CONGES") || !idQuery.next()) {
        if (erreur) *erreur = "Impossible de générer l'identifiant du congé.";
        return false;
    }
    const int nouvelId = idQuery.value(0).toInt();

    const QString demande = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    QSqlQuery q;
    q.prepare("INSERT INTO CONGES "
              "(ID, EMPLOYE_ID, TYPE_CONGE, DATE_DEBUT, DATE_FIN, NB_JOURS, "
              " STATUT, DATE_DEMANDE, MOTIF, DATE_DECISION, VALIDE_PAR) "
              "VALUES (:id, :eid, :type, :debut, :fin, :jours, "
              "        'En attente', :demande, :motif, '', '')");
    q.bindValue(":id", nouvelId);
    q.bindValue(":eid", employeId);
    q.bindValue(":type", typeConge.trimmed());
    q.bindValue(":debut", dateIso(debut));
    q.bindValue(":fin", dateIso(fin));
    q.bindValue(":jours", jours);
    q.bindValue(":demande", demande);
    q.bindValue(":motif", motif.trimmed());
    if (!q.exec()) {
        if (erreur) *erreur = "Enregistrement impossible : " + erreurSql(q);
        return false;
    }

    id = nouvelId;
    dateDebut = debut;
    dateFin = fin;
    nbJours = jours;
    statut = "En attente";
    dateDemande = demande;
    dateDecision.clear();
    validePar.clear();

    HistoriqueEmploye::enregistrer(
        employeId, "CONGE",
        QString("Demande de congé (%1) du %2 au %3")
            .arg(typeConge, dateAffichage(debut), dateAffichage(fin)),
        QString(),
        QString("%1 jour(s) — En attente").arg(jours));
    return true;
}

bool Conge::accepter(int id, QString *erreur, const QString &validePar)
{
    Conge c = charger(id);
    if (c.id == 0) {
        if (erreur) *erreur = "Congé introuvable.";
        return false;
    }
    if (statutCongeNormalise(c.statut) == "Accepté") {
        return true;
    }
    if (statutCongeNormalise(c.statut) != "En attente") {
        if (erreur) *erreur = "Seule une demande en attente peut être acceptée.";
        return false;
    }
    if (chevauche(c.employeId, c.dateDebut, c.dateFin, c.id, false)) {
        if (erreur) *erreur = "Cette période chevauche un congé déjà accepté.";
        return false;
    }

    const int droit = droitAnnuel(c.employeId);
    const int utilises = joursUtilises(c.employeId, c.dateDebut.year());
    if (utilises + c.nbJours > droit) {
        if (erreur) {
            *erreur = QString("Solde insuffisant : %1 jour(s) restant(s) pour %2 jour(s) demandé(s).")
                          .arg(qMax(0, droit - utilises))
                          .arg(c.nbJours);
        }
        return false;
    }

    const QString decision = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    QSqlQuery q;
    q.prepare("UPDATE CONGES SET STATUT = 'Accepté', DATE_DECISION = :decision, "
              "VALIDE_PAR = :par WHERE ID = :id AND STATUT = 'En attente'");
    q.bindValue(":decision", decision);
    q.bindValue(":par", validePar);
    q.bindValue(":id", id);
    if (!q.exec()) {
        if (erreur) *erreur = "Acceptation impossible : " + erreurSql(q);
        return false;
    }

    HistoriqueEmploye::enregistrer(
        c.employeId, "CONGE_ACCEPTE",
        QString("Congé accepté (%1) du %2 au %3")
            .arg(c.typeConge, dateAffichage(c.dateDebut), dateAffichage(c.dateFin)),
        "En attente", QString("Accepté — %1 jour(s) — par %2").arg(c.nbJours).arg(validePar));
    return true;
}

bool Conge::refuser(int id, const QString &motifRefus, QString *erreur,
                    const QString &validePar)
{
    Conge c = charger(id);
    if (c.id == 0) {
        if (erreur) *erreur = "Congé introuvable.";
        return false;
    }
    if (statutCongeNormalise(c.statut) != "En attente") {
        if (erreur) *erreur = "Seule une demande en attente peut être refusée.";
        return false;
    }

    const QString decision = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    QSqlQuery q;
    q.prepare("UPDATE CONGES SET STATUT = 'Refusé', DATE_DECISION = :decision, "
              "VALIDE_PAR = :par WHERE ID = :id AND STATUT = 'En attente'");
    q.bindValue(":decision", decision);
    q.bindValue(":par", validePar);
    q.bindValue(":id", id);
    if (!q.exec()) {
        if (erreur) *erreur = "Refus impossible : " + erreurSql(q);
        return false;
    }

    QString description = QString("Congé refusé (%1) du %2 au %3")
        .arg(c.typeConge, dateAffichage(c.dateDebut), dateAffichage(c.dateFin));
    if (!motifRefus.trimmed().isEmpty()) {
        description += " — Motif : " + motifRefus.trimmed();
    }
    HistoriqueEmploye::enregistrer(
        c.employeId, "CONGE_REFUSE", description,
        "En attente", QString("Refusé — par %1").arg(validePar));
    return true;
}

bool Conge::annuler(int id, QString *erreur)
{
    Conge c = charger(id);
    if (c.id == 0) {
        if (erreur) *erreur = "Congé introuvable.";
        return false;
    }
    if (statutCongeNormalise(c.statut) == "Annulé") {
        return true;
    }

    QSqlQuery q;
    q.prepare("UPDATE CONGES SET STATUT = 'Annulé', DATE_DECISION = :decision, "
              "VALIDE_PAR = 'Administrateur' WHERE ID = :id");
    q.bindValue(":decision", QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));
    q.bindValue(":id", id);
    if (!q.exec()) {
        if (erreur) *erreur = "Annulation impossible : " + erreurSql(q);
        return false;
    }

    HistoriqueEmploye::enregistrer(
        c.employeId, "CONGE_ANNULE",
        QString("Congé annulé (%1) du %2 au %3")
            .arg(c.typeConge, dateAffichage(c.dateDebut), dateAffichage(c.dateFin)),
        c.statut, "Annulé");
    return true;
}

int Conge::droitAnnuel(int employeId)
{
    QSqlQuery q;
    q.prepare("SELECT SOLDE_CONGES FROM EMPLOYES WHERE ID = :id");
    q.bindValue(":id", employeId);
    if (q.exec() && q.next()) {
        return q.value(0).isNull() ? 30 : q.value(0).toInt();
    }
    return 30;
}

int Conge::joursUtilises(int employeId, int annee)
{
    if (annee == 0) {
        annee = QDate::currentDate().year();
    }
    int total = 0;
    for (const Conge &c : chargerPourEmploye(employeId)) {
        if (statutCongeNormalise(c.statut) != "Accepté") {
            continue;
        }
        if (c.dateDebut.isValid() && c.dateDebut.year() == annee) {
            total += c.nbJours > 0 ? c.nbJours : ::joursOuvrables(c.dateDebut, c.dateFin);
        }
    }
    return total;
}

int Conge::joursEnAttente(int employeId)
{
    int total = 0;
    for (const Conge &c : chargerPourEmploye(employeId)) {
        if (statutCongeNormalise(c.statut) == "En attente") {
            total += c.nbJours > 0 ? c.nbJours : ::joursOuvrables(c.dateDebut, c.dateFin);
        }
    }
    return total;
}

int Conge::joursRestants(int employeId, int annee)
{
    if (annee == 0) {
        annee = QDate::currentDate().year();
    }
    return qMax(0, droitAnnuel(employeId) - joursUtilises(employeId, annee));
}

bool Conge::chevauche(int employeId, const QDate &debut, const QDate &fin,
                      int idExclu, bool inclureEnAttente)
{
    if (!debut.isValid() || !fin.isValid() || fin < debut) {
        return false;
    }
    for (const Conge &c : chargerPourEmploye(employeId)) {
        if (idExclu > 0 && c.id == idExclu) {
            continue;
        }
        const QString statut = statutCongeNormalise(c.statut);
        if (statut != "Accepté" && !(inclureEnAttente && statut == "En attente")) {
            continue;
        }
        if (!c.dateDebut.isValid() || !c.dateFin.isValid()) {
            continue;
        }
        if (debut <= c.dateFin && fin >= c.dateDebut) {
            return true;
        }
    }
    return false;
}

int Conge::joursOuvrables(const QDate &debut, const QDate &fin)
{
    return ::joursOuvrables(debut, fin);
}

QStringList Conge::types()
{
    return {"Congé annuel", "Annuel", "Congé maladie", "Congé sans solde",
            "Congé exceptionnel", "Maternité/Paternité", "Formation"};
}

QStringList Conge::statuts()
{
    return {"En attente", "Accepté", "Refusé", "Annulé"};
}
