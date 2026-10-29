#ifndef CONNECTION_H
#define CONNECTION_H

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

/*
 * Connexion à la base de données.
 *
 * L'application essaye d'abord Oracle (via le pilote ODBC), puis se replie
 * automatiquement sur une base SQLite locale si Oracle n'est pas disponible.
 *
 * Mettre SWM_ESSAYER_ORACLE à 0 pour travailler uniquement en SQLite.
 */
#ifndef SWM_ESSAYER_ORACLE
#define SWM_ESSAYER_ORACLE 1
#endif

class Connection
{
public:
    Connection() : m_oracle(false) {}

    /* Ouvre la connexion et crée / met à jour les tables du module Employés. */
    bool createConnection()
    {
        m_erreur.clear();
        m_oracle = false;

        bool ouverte = false;

#if SWM_ESSAYER_ORACLE
        if (QSqlDatabase::isDriverAvailable("QODBC")) {
            ouverte = ouvrirOracle();
            if (ouverte) {
                m_oracle = true;
            }
        } else {
            m_erreur = "Pilote QODBC indisponible. ";
        }
#endif

        if (!ouverte) {
            ouverte = ouvrirSqlite();
        }

        if (!ouverte) {
            return false;
        }

        return creerEtMettreAJourTables();
    }

    bool estOracle() const { return m_oracle; }

    QString messageErreur() const { return m_erreur; }

    QString description() const
    {
        return m_oracle ? QString("Oracle (ODBC)") : QString("SQLite (fichier local)");
    }

    /* Dossier d'écriture de l'application (base SQLite, photos de profil). */
    static QString dossierDonnees()
    {
        QString dossier = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dossier.isEmpty()) {
            dossier = QCoreApplication::applicationDirPath();
        }
        QDir().mkpath(dossier);
        return dossier;
    }

    /* Vrai si la connexion courante est Oracle (utile pour le SQL spécifique). */
    static bool pilotageOracle()
    {
        return QSqlDatabase::database().driverName().startsWith("QODBC");
    }

    /* Vrai si la table contient déjà la colonne demandée. */
    static bool colonneExiste(const QString &table, const QString &colonne)
    {
        if (pilotageOracle()) {
            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS "
                      "WHERE TABLE_NAME = :table AND COLUMN_NAME = :colonne");
            q.bindValue(":table", table.toUpper());
            q.bindValue(":colonne", colonne.toUpper());
            if (!q.exec() || !q.next()) {
                return false;
            }
            return q.value(0).toInt() > 0;
        }

        QSqlQuery q;
        if (!q.exec("PRAGMA table_info(" + table + ")")) {
            return false;
        }
        while (q.next()) {
            if (q.value(1).toString().compare(colonne, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
        return false;
    }

private:
    bool ouvrirOracle()
    {
        /* À adapter : nom de la source ODBC, utilisateur et mot de passe Oracle. */
        const QString sourceOdbc = "Source_Projet2A";
        const QString utilisateur = "projet2a";
        const QString motDePasse = "mp2a";

        bool ok = false;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
            db.setDatabaseName(sourceOdbc);
            db.setUserName(utilisateur);
            db.setPassword(motDePasse);
            db.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=3");
            ok = db.open();
            if (!ok) {
                m_erreur += "Oracle : " + db.lastError().text() + " ";
            }
        }
        if (!ok) {
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        }
        return ok;
    }

    bool ouvrirSqlite()
    {
        bool ok = false;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(dossierDonnees() + "/smartweather.db");
            ok = db.open();
            if (!ok) {
                m_erreur += "SQLite : " + db.lastError().text();
            }
        }
        if (!ok) {
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        }
        return ok;
    }

    /* Ajoute une colonne manquante (migration automatique des anciennes bases). */
    bool ajouterColonne(const QString &table, const QString &colonne,
                        const QString &typeSqlite, const QString &typeOracle)
    {
        if (colonneExiste(table, colonne)) {
            return true;
        }

        QString sql;
        if (pilotageOracle()) {
            sql = QString("ALTER TABLE %1 ADD (%2 %3)").arg(table, colonne, typeOracle);
        } else {
            sql = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, colonne, typeSqlite);
        }

        QSqlQuery q;
        if (!q.exec(sql)) {
            m_erreur = "Migration de " + table + "." + colonne + " : " + q.lastError().text();
            return false;
        }
        return true;
    }

    bool creerEtMettreAJourTables()
    {
        // ── Tables du module Clients (existantes, à conserver) ───────────────
        if (!tableExiste("CLIENTS")) {
            QSqlQuery q;
            const bool ok = q.exec(
                "CREATE TABLE CLIENTS ("
                "ID INTEGER PRIMARY KEY, "
                "CODE VARCHAR(20) NOT NULL, "
                "NOM VARCHAR(50) NOT NULL, "
                "PRENOM VARCHAR(50) NOT NULL, "
                "EMAIL VARCHAR(100) NOT NULL, "
                "TELEPHONE VARCHAR(20), "
                "SERVICE VARCHAR(60), "
                "STATUT VARCHAR(20) DEFAULT 'Actif', "
                "DATE_INSCRIPTION VARCHAR(20), "
                "ADRESSE VARCHAR(150), "
                "DATE_NAISSANCE VARCHAR(20), "
                "METIER VARCHAR(60), "
                "PHOTO VARCHAR(255))");
            if (!ok) {
                m_erreur = "Création de la table CLIENTS : " + q.lastError().text();
                return false;
            }
        }

        if (!tableExiste("OPERATIONS_CLIENTS")) {
            QSqlQuery q;
            const bool ok = q.exec(
                "CREATE TABLE OPERATIONS_CLIENTS ("
                "ID INTEGER PRIMARY KEY, "
                "ID_CLIENT INTEGER NOT NULL, "
                "DATE_OP VARCHAR(20), "
                "TYPE_OP VARCHAR(100), "
                "DESCRIPTION VARCHAR(255), "
                "STATUT VARCHAR(20), "
                "NOTES VARCHAR(255))");
            if (!ok) {
                m_erreur = "Création de la table OPERATIONS_CLIENTS : " + q.lastError().text();
                return false;
            }
        }

        // ── Table EMPLOYES ───────────────────────────────────────────────────
        if (!tableExiste("EMPLOYES")) {
            QSqlQuery q;
            const bool ok = q.exec(
                "CREATE TABLE EMPLOYES ("
                "ID INTEGER PRIMARY KEY, "
                "NOM VARCHAR(50) NOT NULL, "
                "PRENOM VARCHAR(50) NOT NULL, "
                "EMAIL VARCHAR(100) NOT NULL, "
                "TELEPHONE VARCHAR(25) NOT NULL, "
                "POSTE VARCHAR(60) NOT NULL, "
                "MOT_DE_PASSE VARCHAR(100), "
                "SERVICE VARCHAR(60) NOT NULL, "
                "DATE_RECRUTEMENT VARCHAR(20) NOT NULL, "
                "SALAIRE FLOAT NOT NULL, "
                "SOLDE_CONGES INTEGER DEFAULT 30 NOT NULL, "
                "STATUT VARCHAR(20) DEFAULT 'Actif' NOT NULL, "
                "PHOTO VARCHAR(255))");
            if (!ok) {
                m_erreur = "Création de la table EMPLOYES : " + q.lastError().text();
                return false;
            }
        } else {
            // Migration des bases créées par l'ancienne version du projet.
            if (!ajouterColonne("EMPLOYES", "MOT_DE_PASSE", "VARCHAR(100)", "VARCHAR2(100)")) return false;
            if (!ajouterColonne("EMPLOYES", "DATE_RECRUTEMENT", "VARCHAR(20)", "VARCHAR2(20)")) return false;
            if (!ajouterColonne("EMPLOYES", "SOLDE_CONGES", "INTEGER DEFAULT 30", "NUMBER(3) DEFAULT 30")) return false;
        }

        // ── Table CONGES ─────────────────────────────────────────────────────
        if (!tableExiste("CONGES")) {
            QSqlQuery q;
            const bool ok = q.exec(
                "CREATE TABLE CONGES ("
                "ID INTEGER PRIMARY KEY, "
                "EMPLOYE_ID INTEGER NOT NULL, "
                "TYPE_CONGE VARCHAR(50) NOT NULL, "
                "DATE_DEBUT VARCHAR(20) NOT NULL, "
                "DATE_FIN VARCHAR(20) NOT NULL, "
                "NB_JOURS INTEGER NOT NULL, "
                "STATUT VARCHAR(20) DEFAULT 'En attente' NOT NULL, "
                "DATE_DEMANDE VARCHAR(25) NOT NULL, "
                "MOTIF VARCHAR(255), "
                "DATE_DECISION VARCHAR(25), "
                "VALIDE_PAR VARCHAR(50))");
            if (!ok) {
                m_erreur = "Création de la table CONGES : " + q.lastError().text();
                return false;
            }
        } else {
            if (!ajouterColonne("CONGES", "DATE_DECISION", "VARCHAR(25)", "VARCHAR2(25)")) return false;
            if (!ajouterColonne("CONGES", "VALIDE_PAR", "VARCHAR(50)", "VARCHAR2(50)")) return false;
        }

        // ── Table HISTORIQUE_EMPLOYES ────────────────────────────────────────
        if (!tableExiste("HISTORIQUE_EMPLOYES")) {
            QSqlQuery q;
            const bool ok = q.exec(
                "CREATE TABLE HISTORIQUE_EMPLOYES ("
                "ID INTEGER PRIMARY KEY, "
                "EMPLOYE_ID INTEGER NOT NULL DEFAULT 0, "
                "DATE_ACTION VARCHAR(25) NOT NULL, "
                "TYPE_ACTION VARCHAR(30) NOT NULL, "
                "DESCRIPTION VARCHAR(500), "
                "ANCIENNE_VALEUR VARCHAR(500), "
                "NOUVELLE_VALEUR VARCHAR(500), "
                "UTILISATEUR VARCHAR(50) DEFAULT 'Administrateur', "
                "MODULE VARCHAR(20) DEFAULT 'EMPLOYE')");
            if (!ok) {
                m_erreur = "Création de la table HISTORIQUE_EMPLOYES : " + q.lastError().text();
                return false;
            }
        } else {
            // Les anciennes lignes (module Clients) restent marquées CLIENTS et
            // ne seront pas affichées dans l'historique des employés.
            if (!ajouterColonne("HISTORIQUE_EMPLOYES", "MODULE",
                                "VARCHAR(20) DEFAULT 'CLIENT'",
                                "VARCHAR2(20) DEFAULT 'CLIENT'")) return false;
        }

        return true;
    }

    bool tableExiste(const QString &nom) const
    {
        const QStringList tables = QSqlDatabase::database().tables(QSql::Tables);
        for (const QString &t : tables) {
            if (t.compare(nom, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
        return false;
    }

    bool m_oracle;
    QString m_erreur;
};

#endif // CONNECTION_H
