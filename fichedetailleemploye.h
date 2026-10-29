#ifndef FICHEDETAILLEEEMPLOYE_H
#define FICHEDETAILLEEEMPLOYE_H

#include "conge.h"
#include "employe.h"

#include <QVector>
#include <QWidget>

class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class ChartsEvolution;
class ChartsBarres;

/*
 * Fiche détaillée d'un employé : informations, congés, statistiques et
 * historique d'activité. Les actions (accepter / refuser / annuler) sont
 * réellement enregistrées dans la base.
 */
class FicheDetailleeEmploye : public QWidget
{
    Q_OBJECT

public:
    explicit FicheDetailleeEmploye(QWidget *parent = nullptr);

    bool chargerEmploye(int id);
    void exporterFichePDF();
    void afficherOngletHistorique();

signals:
    void retourListe();
    void employeModifie();
    void employeAffiche(int id);

private slots:
    void slotRetour();
    void slotModifier();
    void slotSoumettreConge();
    void slotReinitialiserDemande();
    void slotActualiserJoursDemande();
    void slotExporterFiche();
    void slotPrecedent();
    void slotSuivant();
    void slotChangerPhoto();
    void slotAccepterConge();
    void slotRefuserConge();
    void slotAnnulerConge();
    void slotSelectionConge();
    void slotOngletChange(int index);

private:
    void creerInterface();
    QWidget *creerChamp(const QString &libelle, QLabel *&valeur, QWidget *parent);
    QWidget *creerOngletInformations();
    QWidget *creerOngletConges();
    QWidget *creerOngletStatistiques();
    QWidget *creerOngletActivite();
    QWidget *creerCarteResume(const QString &titre, const QString &sousTitre,
                              QLabel *&valeur, QWidget *parent);

    void rafraichirTout();
    void rafraichirEntete();
    void rafraichirInformations();
    void rafraichirConges();
    void rafraichirStatistiques();
    void rafraichirActivite();
    void mettreAJourBoutonsConges();
    void mettreAJourTitre();
    int idCongeSelectionne() const;

    int m_id = 0;
    Employe m_employe;
    QVector<Conge> m_conges;
    int m_idCongeSelectionne = 0;

    QLabel *m_avatar = nullptr;
    QLabel *m_nom = nullptr;
    QLabel *m_sousTitre = nullptr;
    QLabel *m_badge = nullptr;
    QLabel *m_contact = nullptr;
    QLabel *m_profilDroit = nullptr;
    QLabel *m_profilRestants = nullptr;
    QLabel *m_titreFiche = nullptr;
    QPushButton *m_btnPrecedent = nullptr;
    QPushButton *m_btnSuivant = nullptr;
    QPushButton *m_btnPhoto = nullptr;

    QLabel *m_valId = nullptr;
    QLabel *m_valNom = nullptr;
    QLabel *m_valPrenom = nullptr;
    QLabel *m_valEmail = nullptr;
    QLabel *m_valTelephone = nullptr;
    QLabel *m_valPoste = nullptr;
    QLabel *m_valService = nullptr;
    QLabel *m_valDate = nullptr;
    QLabel *m_valSalaire = nullptr;
    QLabel *m_valDroit = nullptr;
    QLabel *m_valStatut = nullptr;
    QLabel *m_valMotDePasse = nullptr;

    QLabel *m_valDroitConge = nullptr;
    QLabel *m_valUtilises = nullptr;
    QLabel *m_valRestants = nullptr;
    QLabel *m_valAttente = nullptr;
    QComboBox *m_typeConge = nullptr;
    QDateEdit *m_debutConge = nullptr;
    QDateEdit *m_finConge = nullptr;
    QTextEdit *m_raisonConge = nullptr;
    QLabel *m_joursConge = nullptr;
    QPushButton *m_btnSoumettre = nullptr;
    QPushButton *m_btnReinitialiser = nullptr;
    QTableWidget *m_tableConges = nullptr;
    QLabel *m_congeVide = nullptr;
    QPushButton *m_btnAccepter = nullptr;
    QPushButton *m_btnRefuser = nullptr;
    QPushButton *m_btnAnnuler = nullptr;

    QLabel *m_resumeStatistiques = nullptr;
    ChartsEvolution *m_chartConges = nullptr;
    ChartsBarres *m_chartTypes = nullptr;

    QTableWidget *m_tableActivite = nullptr;
    QLabel *m_histoVide = nullptr;
    QLabel *m_histoTotal = nullptr;
    QLabel *m_histoTypes = nullptr;
    QLabel *m_histoConges = nullptr;
    QLabel *m_histoDerniere = nullptr;

    QTabWidget *m_onglets = nullptr;
    QWidget *m_ongletConges = nullptr;
    QWidget *m_ongletHistorique = nullptr;
};

#endif // FICHEDETAILLEEEMPLOYE_H
