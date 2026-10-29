#ifndef VUEEMPLOYES_H
#define VUEEMPLOYES_H

#include "employe.h"

#include <QWidget>

class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableView;
class ModeleEmployes;
class KpiCard;
class ChartsEvolution;
class ChartsRepartition;

/*
 * Page « Employés » dans la présentation historique du module Clients :
 * en-tête bleu nuit, barre de filtres, tableau à gauche, fiche et
 * formulaire à droite, puis statistiques réelles en bas de page.
 *
 * Toutes les actions passent par la couche Employes (SQLite) : aucune
 * valeur d'exemple et aucun bouton décoratif.
 */
class VueEmployes : public QWidget
{
    Q_OBJECT

public:
    explicit VueEmployes(QWidget *parent = nullptr);

    void rechargerDonnees();
    void filtrerParService(const QString &service);
    void afficherTousLesEmployes();
    void selectionnerEmploye(int id);

signals:
    void ouvrirFicheDetaillee(int id);
    void ouvrirHistoriqueEmploye(int id);
    void exporterFicheEmploye(int id);
    void donneesModifiees();

private slots:
    void slotRecherche(const QString &texte);
    void slotFiltreService(int index);
    void slotFiltreStatut(int index);
    void slotReinitialiser();
    void slotAjouter();
    void slotEnregistrer();
    void slotModifier();
    void slotAnnuler();
    void slotDesactiver();
    void slotConsulter();
    void slotConsulterHistorique();
    void slotExporterFichePDF();
    void slotDoubleClic(const QModelIndex &index);
    void slotChangerPhoto();
    void slotExporterPDF();
    void slotLigneCourante(const QModelIndex &courant, const QModelIndex &precedent);

private:
    void creerInterface();
    QWidget *creerPanneauGaucheTableau();
    QWidget *creerPanneauDroitFiche();
    QWidget *creerSectionStatistiques();
    void configurerColonnes();
    void connecterModele();
    void mettreAJourKpis();
    void mettreAJourFiltres();
    void mettreAJourCompteur();
    void selectionnerPremiereLigne();
    void chargerFiche(int id);
    void viderFormulaire();
    void remplirFormulaire(const Employe &employe);
    void mettreAJourPhotoApercu();
    void mettreAJourBoutons();
    int idSelectionne() const;
    QString copierPhoto(const QString &source);
    void afficherPhotoAgrandie(const QString &chemin);
    void afficherErreur(const QString &message);

    ModeleEmployes *m_modele = nullptr;
    QTableView *m_table = nullptr;

    QLineEdit *m_recherche = nullptr;
    QComboBox *m_filtreService = nullptr;
    QComboBox *m_filtreStatut = nullptr;
    QLabel *m_compteur = nullptr;

    QLabel *m_titreForm = nullptr;
    QLabel *m_photoApercu = nullptr;

    QLineEdit *m_editId = nullptr;
    QLineEdit *m_editNom = nullptr;
    QLineEdit *m_editPrenom = nullptr;
    QLineEdit *m_editEmail = nullptr;
    QLineEdit *m_editTelephone = nullptr;
    QLineEdit *m_editPoste = nullptr;
    QComboBox *m_comboService = nullptr;
    QDateEdit *m_editDate = nullptr;
    QLineEdit *m_editSalaire = nullptr;
    QLineEdit *m_editSolde = nullptr;
    QComboBox *m_comboStatut = nullptr;
    QLineEdit *m_editMotDePasse = nullptr;
    QPushButton *m_btnEnregistrer = nullptr;
    QPushButton *m_btnModifier = nullptr;
    QPushButton *m_btnAnnuler = nullptr;
    QPushButton *m_btnDesactiver = nullptr;
    QPushButton *m_btnHistorique = nullptr;
    QPushButton *m_btnFichePDF = nullptr;

    KpiCard *m_kpiTotal = nullptr;
    KpiCard *m_kpiNouveaux = nullptr;
    KpiCard *m_kpiActifs = nullptr;
    KpiCard *m_kpiInactifs = nullptr;
    ChartsEvolution *m_chartEvolution = nullptr;
    ChartsRepartition *m_chartRepartition = nullptr;

    QVector<Employe> m_employes;
    int m_idCourant = 0;
    QString m_photoCourante;
};

#endif // VUEEMPLOYES_H
