#include "historiqueemploye.h"
#include "connection.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

bool HistoriqueEmploye::assurerSchema()
{
    static bool dejaVerifie = false;
    if (dejaVerifie) {
        return true;
    }

    if (Connection::colonneExiste("HISTORIQUE_EMPLOYES", "MODULE")) {
        dejaVerifie = true;
        return true;
    }

    const QString sql = Connection::pilotageOracle()
        ? "ALTER TABLE HISTORIQUE_EMPLOYES ADD (MODULE VARCHAR2(20) DEFAULT 'CLIENT')"
        : "ALTER TABLE HISTORIQUE_EMPLOYES ADD COLUMN MODULE VARCHAR(20) DEFAULT 'CLIENT'";

    QSqlQuery q;
    if (!q.exec(sql)) {
        return false;
    }
    dejaVerifie = true;
    return true;
}

bool HistoriqueEmploye::enregistrer(int employeId, const QString &typeAction,
                                    const QString &description,
                                    const QString &ancienneValeur,
                                    const QString &nouvelleValeur,
                                    const QString &utilisateur)
{
    if (employeId <= 0 || typeAction.trimmed().isEmpty()) {
        return false;
    }
    if (!assurerSchema()) {
        return false;
    }

    QSqlQuery q;
    q.prepare("INSERT INTO HISTORIQUE_EMPLOYES "
              "(EMPLOYE_ID, DATE_ACTION, TYPE_ACTION, DESCRIPTION, "
              " ANCIENNE_VALEUR, NOUVELLE_VALEUR, UTILISATEUR, MODULE) "
              "VALUES (:eid, :date, :type, :desc, :anc, :nouv, :util, 'EMPLOYE')");
    q.bindValue(":eid", employeId);
    q.bindValue(":date", QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));
    q.bindValue(":type", typeAction.trimmed().toUpper());
    q.bindValue(":desc", description);
    q.bindValue(":anc", ancienneValeur);
    q.bindValue(":nouv", nouvelleValeur);
    q.bindValue(":util", utilisateur);
    return q.exec();
}

static EntreeHistorique ligneDepuisRequete(const QSqlQuery &q)
{
    EntreeHistorique e;
    e.id = q.value(0).toInt();
    e.employeId = q.value(1).toInt();
    e.dateAction = q.value(2).toString();
    e.typeAction = q.value(3).toString();
    e.description = q.value(4).toString();
    e.ancienneValeur = q.value(5).toString();
    e.nouvelleValeur = q.value(6).toString();
    e.utilisateur = q.value(7).toString();
    return e;
}

QVector<EntreeHistorique> HistoriqueEmploye::chargerPourEmploye(int employeId)
{
    QVector<EntreeHistorique> resultat;
    if (employeId <= 0 || !assurerSchema()) {
        return resultat;
    }

    QSqlQuery q;
    q.prepare("SELECT ID, EMPLOYE_ID, DATE_ACTION, TYPE_ACTION, DESCRIPTION, "
              "ANCIENNE_VALEUR, NOUVELLE_VALEUR, UTILISATEUR "
              "FROM HISTORIQUE_EMPLOYES "
              "WHERE EMPLOYE_ID = :eid AND MODULE = 'EMPLOYE' "
              "ORDER BY ID DESC");
    q.bindValue(":eid", employeId);

    if (q.exec()) {
        while (q.next()) {
            resultat.append(ligneDepuisRequete(q));
        }
    }
    return resultat;
}

QVector<EntreeHistorique> HistoriqueEmploye::chargerTous(int limite)
{
    QVector<EntreeHistorique> resultat;
    if (!assurerSchema()) {
        return resultat;
    }

    QSqlQuery q;
    q.prepare("SELECT ID, EMPLOYE_ID, DATE_ACTION, TYPE_ACTION, DESCRIPTION, "
              "ANCIENNE_VALEUR, NOUVELLE_VALEUR, UTILISATEUR "
              "FROM HISTORIQUE_EMPLOYES "
              "WHERE MODULE = 'EMPLOYE' "
              "ORDER BY ID DESC");
    if (q.exec()) {
        while (q.next()) {
            resultat.append(ligneDepuisRequete(q));
            if (limite > 0 && resultat.size() >= limite) {
                break;
            }
        }
    }
    return resultat;
}
