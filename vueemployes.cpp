#include "vueemployes.h"

#include "chartsevolution.h"
#include "chartsrepartition.h"
#include "conge.h"
#include "deleguesemployes.h"
#include "employe.h"
#include "icones.h"
#include "kpicard.h"
#include "modelemployes.h"
#include "outilsemployes.h"
#include "stylesemployes.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDir>
#include <QDoubleValidator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QIntValidator>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QTableView>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>

namespace {

QString styleEntetePage()
{
    return "QFrame#entetePage { background: transparent; "
           "border: none; border-bottom: 1px solid #D8E2E9; }"
           "QFrame#entetePage QLabel { background: transparent; border: none; }";
}

QString styleCarte()
{
    return "QFrame#carteBlanche { background-color: #FFFFFF; "
           "border-radius: 8px; border: 1px solid #D8E2E9; }";
}

QString styleLabelCle()
{
    return "border: none; color: #64748B; font-size: 10px; font-weight: bold;";
}

QString styleSaisie()
{
    return "QLineEdit, QDateEdit, QComboBox {"
           "  background: #F8FAFC;"
           "  border: 1px solid #CCD7E0;"
           "  border-radius: 4px;"
           "  padding: 4px 8px;"
           "  font-size: 11px;"
           "  color: #1B3346;"
           "}"
           "QLineEdit:focus, QDateEdit:focus, QComboBox:focus {"
           "  border-color: #179BAE;"
           "  background: #FFFFFF;"
           "}";
}

double lireMontant(const QString &texte, bool *ok)
{
    QString valeur = texte.trimmed();
    valeur.remove(' ');
    valeur.replace(',', '.');
    return valeur.toDouble(ok);
}

} // namespace

VueEmployes::VueEmployes(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageEmployes");
    setStyleSheet(StylesEmployes::feuille());
    creerInterface();

    m_modele = new ModeleEmployes(this);
    m_table->setModel(m_modele);
    configurerColonnes();
    connecterModele();

    m_table->setSortingEnabled(true);
    m_table->horizontalHeader()->setSortIndicator(ModeleEmployes::ColId,
                                                  Qt::AscendingOrder);

    m_table->setItemDelegateForColumn(ModeleEmployes::ColPhoto,
                                      new DeleguePhotoEmploye(this));
    m_table->setItemDelegateForColumn(ModeleEmployes::ColStatut,
                                      new DelegueStatutEmploye(this));

    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &VueEmployes::slotLigneCourante);
    connect(m_table, &QTableView::doubleClicked,
            this, &VueEmployes::slotDoubleClic);
    connect(m_table, &QTableView::clicked, this, [this](const QModelIndex &index) {
        if (index.column() != ModeleEmployes::ColPhoto) {
            return;
        }
        const QString photo = index.data(Qt::UserRole + 10).toString();
        if (!photo.trimmed().isEmpty()) {
            afficherPhotoAgrandie(photo);
        }
    });

    rechargerDonnees();
}

void VueEmployes::creerInterface()
{
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->setContentsMargins(0, 0, 0, 0);
    principal->setSpacing(0);

    // En-tête léger de la page : le titre et le compte administrateur restent
    // alignés sans grande barre bleu foncé au-dessus de la liste.
    QFrame *entete = new QFrame(this);
    entete->setObjectName("entetePage");
    entete->setStyleSheet(styleEntetePage());
    entete->setFixedHeight(54);
    QHBoxLayout *layoutEntete = new QHBoxLayout(entete);
    layoutEntete->setContentsMargins(20, 0, 20, 0);
    layoutEntete->setSpacing(8);

    QLabel *iconeTitre = new QLabel(entete);
    iconeTitre->setStyleSheet("background: transparent; border: none;");
    iconeTitre->setPixmap(Icones::dessiner(Icones::GroupeUtilisateurs,
                                           QColor("#179BAE"), 26));
    QLabel *titre = new QLabel("GESTION DES EMPLOYÉS - LISTE GLOBALE", entete);
    titre->setStyleSheet("background: transparent; color: #1B3346; "
                         "font-size: 15px; font-weight: bold; "
                         "letter-spacing: 0.5px; border: none;");
    QLabel *iconeAdmin = new QLabel(entete);
    iconeAdmin->setStyleSheet("background: transparent; border: none;");
    iconeAdmin->setPixmap(Icones::dessiner(Icones::Avatar,
                                           QColor("#179BAE"), 24));
    QLabel *admin = new QLabel("Administrateur", entete);
    admin->setStyleSheet("background: transparent; color: #64748B; "
                         "font-size: 12px; font-weight: 500; border: none;");

    layoutEntete->addWidget(iconeTitre);
    layoutEntete->addSpacing(8);
    layoutEntete->addWidget(titre);
    layoutEntete->addStretch();
    layoutEntete->addWidget(iconeAdmin);
    layoutEntete->addSpacing(6);
    layoutEntete->addWidget(admin);
    principal->addWidget(entete);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: #F0F4F8;");

    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #F0F4F8;");
    QVBoxLayout *layoutPage = new QVBoxLayout(page);
    layoutPage->setContentsMargins(16, 12, 16, 16);
    layoutPage->setSpacing(12);

    // Barre de recherche et filtres.
    QFrame *filtres = new QFrame(page);
    filtres->setStyleSheet("background-color: #FFFFFF; border-radius: 6px; "
                           "border: 1px solid #D8E2E9;");
    filtres->setFixedHeight(50);
    QHBoxLayout *layoutFiltres = new QHBoxLayout(filtres);
    layoutFiltres->setContentsMargins(12, 6, 12, 6);
    layoutFiltres->setSpacing(10);

    m_recherche = new QLineEdit(filtres);
    m_recherche->setPlaceholderText(
        "Rechercher un employé (nom, prénom, id, e-mail, poste...)...");
    m_recherche->setClearButtonEnabled(true);
    m_recherche->setStyleSheet(
        "QLineEdit { background: #F8FAFC; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 6px 10px; font-size: 12px; color: #1B3346; } "
        "QLineEdit:focus { border-color: #179BAE; background: #FFFFFF; }");
    connect(m_recherche, &QLineEdit::textChanged,
            this, &VueEmployes::slotRecherche);

    QLabel *lblService = new QLabel("Service :", filtres);
    lblService->setStyleSheet("border: none; color: #546E7A; font-weight: bold; "
                              "font-size: 11px;");
    m_filtreService = new QComboBox(filtres);
    m_filtreService->setStyleSheet(
        "QComboBox { background: #FFFFFF; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 4px 8px; font-size: 11px; "
        "color: #1B3346; min-width: 140px; }");
    connect(m_filtreService, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VueEmployes::slotFiltreService);

    QLabel *lblStatut = new QLabel("Statut :", filtres);
    lblStatut->setStyleSheet("border: none; color: #546E7A; font-weight: bold; "
                             "font-size: 11px;");
    m_filtreStatut = new QComboBox(filtres);
    m_filtreStatut->setStyleSheet(
        "QComboBox { background: #FFFFFF; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 4px 8px; font-size: 11px; "
        "color: #1B3346; min-width: 120px; }");
    connect(m_filtreStatut, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VueEmployes::slotFiltreStatut);

    QPushButton *btnAjouter = new QPushButton("+ AJOUTER UN EMPLOYÉ", filtres);
    btnAjouter->setCursor(Qt::PointingHandCursor);
    btnAjouter->setStyleSheet(
        "QPushButton { background-color: #179BAE; color: #FFFFFF; "
        "font-weight: bold; font-size: 11px; border-radius: 4px; "
        "padding: 7px 14px; border: none; } "
        "QPushButton:hover { background-color: #137A6E; } "
        "QPushButton:pressed { background-color: #0E534B; }");
    connect(btnAjouter, &QPushButton::clicked, this, &VueEmployes::slotAjouter);

    QPushButton *btnExporter = new QPushButton("PDF", filtres);
    btnExporter->setCursor(Qt::PointingHandCursor);
    btnExporter->setIcon(Icones::icone(Icones::DocumentPdf, Qt::white, 20));
    btnExporter->setStyleSheet(
        "QPushButton { background-color: #24384B; color: #FFFFFF; "
        "font-weight: bold; font-size: 11px; border-radius: 4px; "
        "padding: 7px 12px; border: none; } "
        "QPushButton:hover { background-color: #1B2937; }");
    connect(btnExporter, &QPushButton::clicked, this, &VueEmployes::slotExporterPDF);

    layoutFiltres->addWidget(m_recherche, 1);
    layoutFiltres->addWidget(lblService);
    layoutFiltres->addWidget(m_filtreService);
    layoutFiltres->addWidget(lblStatut);
    layoutFiltres->addWidget(m_filtreStatut);
    layoutFiltres->addWidget(btnAjouter);
    layoutFiltres->addWidget(btnExporter);
    layoutPage->addWidget(filtres);

    QHBoxLayout *milieu = new QHBoxLayout();
    milieu->setSpacing(12);
    milieu->addWidget(creerPanneauGaucheTableau(), 1);
    milieu->addWidget(creerPanneauDroitFiche(), 0, Qt::AlignTop);
    layoutPage->addLayout(milieu);

    layoutPage->addWidget(creerSectionStatistiques());

    scroll->setWidget(page);
    principal->addWidget(scroll);
}

QWidget *VueEmployes::creerPanneauGaucheTableau()
{
    QFrame *cadre = new QFrame(this);
    cadre->setObjectName("carteBlanche");
    cadre->setStyleSheet(styleCarte());
    QVBoxLayout *layout = new QVBoxLayout(cadre);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    m_table = new QTableView(cadre);
    m_table->setStyleSheet(
        "QTableView {"
        "  background-color: #FFFFFF;"
        "  alternate-background-color: #F8FAFC;"
        "  selection-background-color: #E0F2FE;"
        "  selection-color: #0369A1;"
        "  border: none;"
        "  gridline-color: #EDF2F7;"
        "  font-size: 11px;"
        "  color: #24384B;"
        "}"
        "QHeaderView::section {"
        "  background-color: #1E3A4F;"
        "  color: #FFFFFF;"
        "  font-weight: bold;"
        "  font-size: 10px;"
        "  padding: 8px 5px;"
        "  border: none;"
        "}"
        "QTableView::item:hover { background-color: #F1F5F9; }");
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(48);
    m_table->horizontalHeader()->setFixedHeight(34);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setShowGrid(false);
    m_table->setWordWrap(false);
    m_table->setTextElideMode(Qt::ElideRight);
    m_table->setMinimumHeight(460);

    m_compteur = new QLabel("Chargement des employés...", cadre);
    m_compteur->setStyleSheet("color: #7A8D9E; font-size: 11px; border: none;");

    layout->addWidget(m_table, 1);
    layout->addWidget(m_compteur);
    return cadre;
}

QWidget *VueEmployes::creerPanneauDroitFiche()
{
    QWidget *conteneur = new QWidget(this);
    conteneur->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    conteneur->setFixedWidth(320);
    QVBoxLayout *layout = new QVBoxLayout(conteneur);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Une seule fiche compacte : photo, informations et actions.
    QFrame *form = new QFrame(conteneur);
    form->setObjectName("carteBlanche");
    form->setStyleSheet(styleCarte());
    QVBoxLayout *layoutForm = new QVBoxLayout(form);
    layoutForm->setContentsMargins(12, 10, 12, 10);
    layoutForm->setSpacing(7);

    m_titreForm = new QLabel("Fiche employé", form);
    m_titreForm->setObjectName("titreFormEmploye");
    m_titreForm->setStyleSheet("border: none; color: #1B3346; font-size: 12px; "
                               "font-weight: bold;");
    layoutForm->addWidget(m_titreForm);

    QHBoxLayout *lignePhoto = new QHBoxLayout();
    lignePhoto->setSpacing(8);

    m_photoApercu = new QLabel("Aucune photo", form);
    m_photoApercu->setObjectName("apercuPhotoEmploye");
    m_photoApercu->setFixedSize(96, 96);
    m_photoApercu->setAlignment(Qt::AlignCenter);
    m_photoApercu->setStyleSheet(
        "background: #E8EEF2; border: 1px solid #CCD7E0; border-radius: 5px; "
        "color: #7A8D9E; font-size: 10px; padding: 3px;");
    lignePhoto->addWidget(m_photoApercu, 0, Qt::AlignTop);

    QVBoxLayout *actionsPhoto = new QVBoxLayout();
    actionsPhoto->setSpacing(5);

    QPushButton *btnPhoto = new QPushButton("Ajouter une photo", form);
    btnPhoto->setObjectName("boutonPhotoEmploye");
    btnPhoto->setCursor(Qt::PointingHandCursor);
    btnPhoto->setToolTip("Sélectionner une image sur l'ordinateur");
    btnPhoto->setStyleSheet(
        "QPushButton { background: #E8F5F7; color: #137A6E; border: 1px solid #B8E0E4; "
        "font-size: 10px; font-weight: bold; border-radius: 4px; padding: 4px 6px; } "
        "QPushButton:hover { background: #D8EEF1; }");
    btnPhoto->setFixedHeight(26);
    connect(btnPhoto, &QPushButton::clicked, this, &VueEmployes::slotChangerPhoto);
    actionsPhoto->addWidget(btnPhoto);

    m_btnHistorique = new QPushButton("Historique", form);
    m_btnHistorique->setObjectName("boutonHistoriqueEmploye");
    m_btnHistorique->setCursor(Qt::PointingHandCursor);
    m_btnHistorique->setToolTip("Ouvrir l'historique complet de l'employé");
    m_btnHistorique->setStyleSheet(
        "QPushButton { background: #1B3346; color: #FFFFFF; border: none; "
        "font-size: 10px; font-weight: bold; border-radius: 4px; padding: 4px 6px; } "
        "QPushButton:hover { background: #132431; } "
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; }");
    m_btnHistorique->setFixedHeight(26);
    connect(m_btnHistorique, &QPushButton::clicked,
            this, &VueEmployes::slotConsulterHistorique);
    actionsPhoto->addWidget(m_btnHistorique);

    m_btnFichePDF = new QPushButton("Fiche PDF", form);
    m_btnFichePDF->setObjectName("boutonFichePDFEmploye");
    m_btnFichePDF->setCursor(Qt::PointingHandCursor);
    m_btnFichePDF->setToolTip("Exporter la fiche de l'employé en PDF");
    m_btnFichePDF->setStyleSheet(m_btnHistorique->styleSheet());
    m_btnFichePDF->setFixedHeight(26);
    connect(m_btnFichePDF, &QPushButton::clicked,
            this, &VueEmployes::slotExporterFichePDF);
    actionsPhoto->addWidget(m_btnFichePDF);
    actionsPhoto->addStretch();

    lignePhoto->addLayout(actionsPhoto, 1);
    layoutForm->addLayout(lignePhoto);

    QFormLayout *grilleForm = new QFormLayout();
    grilleForm->setContentsMargins(0, 0, 0, 0);
    grilleForm->setHorizontalSpacing(6);
    grilleForm->setVerticalSpacing(4);
    grilleForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    grilleForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    grilleForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto label = [](const QString &texte) {
        QLabel *l = new QLabel(texte);
        l->setStyleSheet(styleLabelCle());
        return l;
    };

    m_editId = new QLineEdit(form);
    m_editId->setObjectName("champIdEmploye");
    m_editId->setReadOnly(true);
    m_editId->setStyleSheet(
        "QLineEdit { background: #EEF2F6; border: 1px solid #D8E2E9; "
        "border-radius: 4px; padding: 4px 8px; font-size: 11px; color: #64748B; }");

    m_editNom = new QLineEdit(form);
    m_editNom->setObjectName("champNomEmploye");
    m_editNom->setStyleSheet(styleSaisie());
    m_editNom->setPlaceholderText("Ex : Maamouri");
    m_editPrenom = new QLineEdit(form);
    m_editPrenom->setObjectName("champPrenomEmploye");
    m_editPrenom->setStyleSheet(styleSaisie());
    m_editPrenom->setPlaceholderText("Ex : Yassmine");
    m_editEmail = new QLineEdit(form);
    m_editEmail->setObjectName("champEmailEmploye");
    m_editEmail->setStyleSheet(styleSaisie());
    m_editEmail->setPlaceholderText("exemple@exemple.tn");
    m_editTelephone = new QLineEdit(form);
    m_editTelephone->setObjectName("champTelephoneEmploye");
    m_editTelephone->setStyleSheet(styleSaisie());
    m_editTelephone->setPlaceholderText("8 à 15 chiffres");
    m_editPoste = new QLineEdit(form);
    m_editPoste->setObjectName("champPosteEmploye");
    m_editPoste->setStyleSheet(styleSaisie());
    m_editPoste->setPlaceholderText("Ex : Administrateur");

    m_comboService = new QComboBox(form);
    m_comboService->setObjectName("champServiceEmploye");
    m_comboService->setEditable(true);
    m_comboService->setInsertPolicy(QComboBox::NoInsert);
    m_comboService->setStyleSheet(styleSaisie());

    m_editSalaire = new QLineEdit(form);
    m_editSalaire->setObjectName("champSalaireEmploye");
    m_editSalaire->setPlaceholderText("0,00");
    m_editSalaire->setValidator(new QDoubleValidator(0.0, 1000000.0, 2, m_editSalaire));
    m_editSalaire->setStyleSheet(styleSaisie());

    m_comboStatut = new QComboBox(form);
    m_comboStatut->setObjectName("champStatutEmploye");
    m_comboStatut->addItems(Employe::statuts());
    m_comboStatut->setStyleSheet(styleSaisie());

    // Champs conservés dans le modèle mais retirés du formulaire simplifié.
    m_editDate = new QDateEdit(QDate::currentDate(), form);
    m_editDate->setObjectName("champDateRecrutement");
    m_editDate->setCalendarPopup(true);
    m_editDate->setDisplayFormat("dd/MM/yyyy");
    m_editDate->setMaximumDate(QDate::currentDate());
    m_editDate->hide();

    m_editSolde = new QLineEdit("30", form);
    m_editSolde->setObjectName("champDroitAnnuel");
    m_editSolde->setValidator(new QIntValidator(0, 365, m_editSolde));
    m_editSolde->hide();

    m_editMotDePasse = new QLineEdit(form);
    m_editMotDePasse->setObjectName("champMotDePasse");
    m_editMotDePasse->setEchoMode(QLineEdit::Password);
    m_editMotDePasse->hide();

    grilleForm->addRow(label("ID"), m_editId);
    grilleForm->addRow(label("Nom"), m_editNom);
    grilleForm->addRow(label("Prénom"), m_editPrenom);
    grilleForm->addRow(label("E-mail"), m_editEmail);
    grilleForm->addRow(label("Téléphone"), m_editTelephone);
    grilleForm->addRow(label("Poste"), m_editPoste);
    grilleForm->addRow(label("Service"), m_comboService);
    grilleForm->addRow(label("Salaire (DT)"), m_editSalaire);
    grilleForm->addRow(label("Statut"), m_comboStatut);
    layoutForm->addLayout(grilleForm);

    QGridLayout *boutons = new QGridLayout();
    boutons->setHorizontalSpacing(6);
    boutons->setVerticalSpacing(6);

    m_btnEnregistrer = new QPushButton("Ajouter un employé", form);
    m_btnEnregistrer->setObjectName("boutonAjouterEmploye");
    m_btnEnregistrer->setCursor(Qt::PointingHandCursor);
    m_btnEnregistrer->setToolTip("Créer un nouvel employé");
    m_btnEnregistrer->setStyleSheet(
        "QPushButton { background-color: #137A6E; color: white; font-weight: bold; "
        "border-radius: 4px; padding: 6px 6px; font-size: 9px; border: none; } "
        "QPushButton:hover { background-color: #0E534B; }");
    connect(m_btnEnregistrer, &QPushButton::clicked, this, [this] {
        if (m_idCourant > 0) {
            slotAjouter();
        } else {
            slotEnregistrer();
        }
    });

    m_btnModifier = new QPushButton("Modifier", form);
    m_btnModifier->setObjectName("boutonModifierEmploye");
    m_btnModifier->setCursor(Qt::PointingHandCursor);
    m_btnModifier->setStyleSheet(
        "QPushButton { background-color: #1B3346; color: white; font-weight: bold; "
        "border-radius: 4px; padding: 6px 8px; font-size: 10px; border: none; } "
        "QPushButton:hover { background-color: #132431; } "
        "QPushButton:disabled { background-color: #E2E8F0; color: #94A3B8; }");
    connect(m_btnModifier, &QPushButton::clicked,
            this, &VueEmployes::slotModifier);

    m_btnDesactiver = new QPushButton("Supprimer", form);
    m_btnDesactiver->setObjectName("boutonSupprimerEmploye");
    m_btnDesactiver->setCursor(Qt::PointingHandCursor);
    m_btnDesactiver->setToolTip("Retirer l'employé de la liste active");
    m_btnDesactiver->setStyleSheet(
        "QPushButton { background-color: #EF4444; color: white; font-weight: bold; "
        "border-radius: 4px; padding: 6px 8px; font-size: 10px; border: none; } "
        "QPushButton:hover { background-color: #DC2626; } "
        "QPushButton:disabled { background: #F1F5F7; color: #A8B6C0; }");
    connect(m_btnDesactiver, &QPushButton::clicked,
            this, &VueEmployes::slotDesactiver);

    m_btnAnnuler = new QPushButton("Annuler", form);
    m_btnAnnuler->setObjectName("boutonAnnulerEmploye");
    m_btnAnnuler->setCursor(Qt::PointingHandCursor);
    m_btnAnnuler->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #475569; font-weight: bold; "
        "border-radius: 4px; padding: 6px 8px; font-size: 10px; border: none; } "
        "QPushButton:hover { background-color: #CBD5E1; }");
    connect(m_btnAnnuler, &QPushButton::clicked,
            this, &VueEmployes::slotAnnuler);

    boutons->addWidget(m_btnEnregistrer, 0, 0);
    boutons->addWidget(m_btnModifier, 0, 1);
    boutons->addWidget(m_btnDesactiver, 1, 0);
    boutons->addWidget(m_btnAnnuler, 1, 1);
    layoutForm->addLayout(boutons);

    layout->addWidget(form);

    viderFormulaire();
    return conteneur;
}

QWidget *VueEmployes::creerSectionStatistiques()
{
    QFrame *cadre = new QFrame(this);
    cadre->setStyleSheet("background-color: transparent; border: none;");
    QVBoxLayout *layout = new QVBoxLayout(cadre);
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(8);

    QFrame *banniere = new QFrame(cadre);
    banniere->setStyleSheet("background-color: #1B3346; border-radius: 6px;");
    banniere->setFixedHeight(36);
    QHBoxLayout *layoutBanniere = new QHBoxLayout(banniere);
    layoutBanniere->setContentsMargins(12, 0, 12, 0);
    layoutBanniere->setSpacing(8);
    QLabel *icone = new QLabel(banniere);
    icone->setPixmap(Icones::dessiner(Icones::GraphiqueLigne,
                                      QColor("#179BAE"), 18));
    QLabel *titre = new QLabel("Statistiques globales des employés", banniere);
    titre->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; "
                         "border: none;");
    layoutBanniere->addWidget(icone);
    layoutBanniere->addWidget(titre);
    layoutBanniere->addStretch();
    layout->addWidget(banniere);

    QHBoxLayout *kpis = new QHBoxLayout();
    kpis->setSpacing(10);
    m_kpiTotal = new KpiCard("Total des employés", "0", "Effectif global",
                             Icones::GroupeUtilisateurs, KpiCard::NavyTeal, cadre);
    m_kpiNouveaux = new KpiCard("Nouveaux employés (ce mois)", "0", "Ce mois-ci",
                                Icones::AjoutUtilisateur, KpiCard::Emerald, cadre);
    m_kpiActifs = new KpiCard("Employés actifs", "0", "0 % de l'effectif",
                              Icones::CocheCercle, KpiCard::OceanBlue, cadre);
    m_kpiInactifs = new KpiCard("Employés inactifs", "0", "0 % de l'effectif",
                                Icones::CroixCercle, KpiCard::SlateDark, cadre);
    kpis->addWidget(m_kpiTotal);
    kpis->addWidget(m_kpiNouveaux);
    kpis->addWidget(m_kpiActifs);
    kpis->addWidget(m_kpiInactifs);
    layout->addLayout(kpis);

    QHBoxLayout *graphiques = new QHBoxLayout();
    graphiques->setSpacing(10);
    m_chartEvolution = new ChartsEvolution(cadre);
    m_chartEvolution->setTitre("Recrutements par mois");
    m_chartEvolution->setUnite("recrutement(s)");
    m_chartEvolution->setCouleur(QColor("#179BAE"));
    m_chartEvolution->setMinimumHeight(250);

    m_chartRepartition = new ChartsRepartition(cadre);
    m_chartRepartition->setTitre("Répartition par statut");
    m_chartRepartition->setSousTitre("Ensemble des employés");
    m_chartRepartition->setMinimumHeight(250);

    graphiques->addWidget(m_chartEvolution, 60);
    graphiques->addWidget(m_chartRepartition, 40);
    layout->addLayout(graphiques);
    return cadre;
}

void VueEmployes::configurerColonnes()
{
    m_table->setColumnHidden(ModeleEmployes::ColPhoto, false);
    m_table->setColumnHidden(ModeleEmployes::ColActions, true);
    m_table->setColumnHidden(ModeleEmployes::ColSalaire, true);
    m_table->setColumnHidden(ModeleEmployes::ColSolde, true);
    // Date, salaire et solde restent dans la fiche : le tableau garde les
    // colonnes utiles tout en laissant la photo visible.
    m_table->setColumnHidden(ModeleEmployes::ColDateRecrutement, true);

    m_table->setColumnWidth(ModeleEmployes::ColId, 34);
    m_table->setColumnWidth(ModeleEmployes::ColPhoto, 52);
    m_table->setColumnWidth(ModeleEmployes::ColNom, 70);
    m_table->setColumnWidth(ModeleEmployes::ColPrenom, 70);
    m_table->setColumnWidth(ModeleEmployes::ColEmail, 120);
    m_table->setColumnWidth(ModeleEmployes::ColTelephone, 76);
    m_table->setColumnWidth(ModeleEmployes::ColPoste, 72);
    m_table->setColumnWidth(ModeleEmployes::ColService, 72);
    m_table->setColumnWidth(ModeleEmployes::ColStatut, 66);
    m_table->horizontalHeader()->setSectionResizeMode(ModeleEmployes::ColEmail,
                                                      QHeaderView::Stretch);
}

void VueEmployes::connecterModele()
{
    connect(m_modele, &QAbstractItemModel::modelReset, this, [this] {
        mettreAJourCompteur();
    });

    // Le tri réinitialise le modèle Qt (donc la sélection). On remet ensuite
    // la fiche précédemment affichée pour ne pas vider le panneau de droite.
    connect(m_table->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int, Qt::SortOrder) {
                const int id = m_idCourant;
                QTimer::singleShot(0, this, [this, id] {
                    if (id > 0 && m_modele->lignePourId(id) >= 0) {
                        selectionnerEmploye(id);
                    } else {
                        selectionnerPremiereLigne();
                    }
                });
            });
}

void VueEmployes::rechargerDonnees()
{
    const int idPrecedent = m_idCourant;
    m_employes = Employes::chargerTous();
    m_modele->setEmployes(m_employes);
    mettreAJourFiltres();
    mettreAJourKpis();
    mettreAJourCompteur();

    int idAConserver = idPrecedent;
    if (idAConserver > 0 && m_modele->lignePourId(idAConserver) < 0) {
        idAConserver = 0;
    }
    if (idAConserver > 0) {
        selectionnerEmploye(idAConserver);
    } else if (m_modele->rowCount() > 0) {
        selectionnerEmploye(m_modele->idLigne(0));
    } else {
        viderFormulaire();
    }
}

void VueEmployes::mettreAJourFiltres()
{
    const QString serviceCourant = m_filtreService->currentText();
    const QString statutCourant = m_filtreStatut->currentText();

    {
        const QSignalBlocker blocker(m_filtreService);
        m_filtreService->clear();
        m_filtreService->addItem("TOUS LES SERVICES");
        QStringList services = Employe::servicesParDefaut();
        for (const QString &service : Employes::servicesEnBase()) {
            if (!services.contains(service, Qt::CaseInsensitive)) {
                services << service;
            }
        }
        m_filtreService->addItems(services);
        int index = m_filtreService->findText(serviceCourant, Qt::MatchFixedString);
        m_filtreService->setCurrentIndex(index < 0 ? 0 : index);
    }

    {
        const QSignalBlocker blocker(m_filtreStatut);
        m_filtreStatut->clear();
        m_filtreStatut->addItem("TOUS LES STATUTS");
        m_filtreStatut->addItems(Employe::statuts());
        int index = m_filtreStatut->findText(statutCourant, Qt::MatchFixedString);
        m_filtreStatut->setCurrentIndex(index < 0 ? 0 : index);
    }

    m_modele->setService(m_filtreService->currentIndex() <= 0
                             ? QString() : m_filtreService->currentText());
    m_modele->setStatut(m_filtreStatut->currentIndex() <= 0
                            ? QString() : m_filtreStatut->currentText());

    // Le formulaire propose les services déjà utilisés, sans interdire une
    // nouvelle valeur saisie par l'utilisateur.
    const QString serviceFormulaire = m_comboService->currentText();
    {
        const QSignalBlocker blocker(m_comboService);
        m_comboService->clear();
        QStringList services = Employe::servicesParDefaut();
        for (const QString &service : Employes::servicesEnBase()) {
            if (!services.contains(service, Qt::CaseInsensitive)) {
                services << service;
            }
        }
        m_comboService->addItems(services);
        if (!serviceFormulaire.trimmed().isEmpty()) {
            m_comboService->setCurrentText(serviceFormulaire);
        }
    }
}

void VueEmployes::mettreAJourKpis()
{
    const StatistiquesEmployes stats = Employes::statistiques();
    m_kpiTotal->setValeur(QString::number(stats.total));
    m_kpiTotal->setEvolution("Effectif global");

    m_kpiNouveaux->setValeur(QString::number(stats.nouveauxCeMois));
    m_kpiNouveaux->setEvolution(
        QLocale(QLocale::French, QLocale::France)
            .toString(QDate::currentDate(), "MMMM yyyy"));

    m_kpiActifs->setValeur(QString::number(stats.actifs));
    m_kpiActifs->setEvolution(
        QString("%1 % de l'effectif").arg(pourcentage(stats.actifs, stats.total)));

    m_kpiInactifs->setValeur(QString::number(stats.inactifs));
    m_kpiInactifs->setEvolution(
        QString("%1 % de l'effectif").arg(pourcentage(stats.inactifs, stats.total)));

    m_chartEvolution->setDonnees(stats.recrutementsParMois);
    m_chartEvolution->setSousTitre(
        QString("Année %1 — nombre de recrutements").arg(QDate::currentDate().year()));

    const QVector<SegmentDonut> segments = {
        {"Actifs", stats.actifs, 0, QColor("#179BAE")},
        {"En congé", stats.enConge, 0, QColor("#E6A15C")},
        {"Inactifs", stats.inactifs, 0, QColor("#8A9BA8")}
    };
    m_chartRepartition->setDonnees(segments);
}

void VueEmployes::mettreAJourCompteur()
{
    if (!m_modele || !m_compteur) {
        return;
    }
    m_compteur->setText(
        QString("Affichage de %1 employé(s) sur %2")
            .arg(m_modele->rowCount())
            .arg(m_modele->nombreTotal()));
}

void VueEmployes::chargerFiche(int id)
{
    if (id <= 0) {
        viderFormulaire();
        return;
    }

    const Employe employe = Employes::charger(id);
    if (employe.id == 0) {
        viderFormulaire();
        return;
    }

    m_idCourant = employe.id;
    m_photoCourante = employe.photo;
    remplirFormulaire(employe);
    mettreAJourBoutons();
}

void VueEmployes::remplirFormulaire(const Employe &employe)
{
    m_editId->setText(QString("EMP-%1").arg(employe.id, 3, 10, QChar('0')));
    m_editNom->setText(employe.nom);
    m_editPrenom->setText(employe.prenom);
    m_editEmail->setText(employe.email);
    m_editTelephone->setText(employe.telephone);
    m_editPoste->setText(employe.poste);
    m_comboService->setCurrentText(employe.service);
    const QDate date = dateDepuisTexte(employe.dateRecrutement);
    m_editDate->setDate(date.isValid() ? date : QDate::currentDate());
    m_editSalaire->setText(QString::number(employe.salaire, 'f', 2).replace('.', ','));
    m_editSolde->setText(QString::number(employe.soldeConges));
    m_comboStatut->setCurrentText(employe.statut);
    m_editMotDePasse->clear();
    m_photoCourante = employe.photo;
    mettreAJourPhotoApercu();
    m_titreForm->setText("Fiche employé");
}

void VueEmployes::viderFormulaire()
{
    m_idCourant = 0;
    m_photoCourante.clear();

    m_editId->setText("(auto)");
    m_editNom->clear();
    m_editPrenom->clear();
    m_editEmail->clear();
    m_editTelephone->clear();
    m_editPoste->clear();
    m_comboService->setCurrentIndex(m_comboService->count() > 0 ? 0 : -1);
    m_editDate->setDate(QDate::currentDate());
    m_editSalaire->clear();
    m_editSolde->setText("30");
    m_comboStatut->setCurrentText("Actif");
    m_editMotDePasse->setText(
        QUuid::createUuid().toString(QUuid::WithoutBraces).left(10));
    m_titreForm->setText("Fiche employé");

    mettreAJourPhotoApercu();
    mettreAJourBoutons();
}

void VueEmployes::mettreAJourPhotoApercu()
{
    if (!m_photoApercu) {
        return;
    }
    if (m_photoCourante.trimmed().isEmpty()) {
        m_photoApercu->setPixmap(QPixmap());
        m_photoApercu->setText("Aucune photo");
        m_photoApercu->setToolTip(QString());
        return;
    }

    QPixmap source(m_photoCourante);
    if (source.isNull()) {
        m_photoApercu->setPixmap(QPixmap());
        m_photoApercu->setText("Aucune photo");
        m_photoApercu->setToolTip(QString());
        return;
    }

    m_photoApercu->setText(QString());
    m_photoApercu->setPixmap(source.scaled(86, 86, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
    m_photoApercu->setToolTip(QFileInfo(m_photoCourante).fileName());
}

void VueEmployes::mettreAJourBoutons()
{
    const bool selection = m_idCourant > 0;
    const Employe employe = selection ? Employes::charger(m_idCourant) : Employe();
    const bool actif = selection &&
        employe.statut.compare("Inactif", Qt::CaseInsensitive) != 0;

    // En mode sélection, « Ajouter un employé » prépare une fiche vierge ;
    // en mode création, il enregistre réellement le nouvel employé.
    m_btnEnregistrer->setEnabled(true);
    m_btnModifier->setEnabled(selection);
    m_btnDesactiver->setEnabled(actif);
    m_btnHistorique->setEnabled(selection);
    m_btnFichePDF->setEnabled(selection);
}

void VueEmployes::slotRecherche(const QString &texte)
{
    m_modele->setRecherche(texte);
    mettreAJourCompteur();
    selectionnerPremiereLigne();
}

void VueEmployes::slotFiltreService(int index)
{
    m_modele->setService(index <= 0 ? QString() : m_filtreService->currentText());
    mettreAJourCompteur();
    selectionnerPremiereLigne();
}

void VueEmployes::slotFiltreStatut(int index)
{
    m_modele->setStatut(index <= 0 ? QString() : m_filtreStatut->currentText());
    mettreAJourCompteur();
    selectionnerPremiereLigne();
}

void VueEmployes::slotReinitialiser()
{
    m_recherche->clear();
    m_filtreService->setCurrentIndex(0);
    m_filtreStatut->setCurrentIndex(0);
    m_modele->setRecherche(QString());
    m_modele->setService(QString());
    m_modele->setStatut(QString());
    mettreAJourCompteur();
    selectionnerPremiereLigne();
}

void VueEmployes::slotAjouter()
{
    m_table->clearSelection();
    viderFormulaire();
    m_editNom->setFocus();
}

void VueEmployes::slotEnregistrer()
{
    if (m_idCourant > 0) {
        QMessageBox::information(
            this, "Modification",
            "Un employé est déjà sélectionné. Utilisez le bouton « Modifier » "
            "pour enregistrer les changements, ou « Annuler » puis « + Ajouter » "
            "pour créer une nouvelle fiche.");
        return;
    }

    bool salaireOk = false;
    const double salaire = lireMontant(m_editSalaire->text(), &salaireOk);
    bool soldeOk = false;
    const int solde = m_editSolde->text().trimmed().toInt(&soldeOk);
    if (!salaireOk || !soldeOk) {
        QMessageBox::warning(this, "Saisie incomplète",
                             "Veuillez saisir un salaire et un droit annuel valides.");
        return;
    }

    Employe employe;
    employe.nom = m_editNom->text();
    employe.prenom = m_editPrenom->text();
    employe.email = m_editEmail->text();
    employe.telephone = m_editTelephone->text();
    employe.poste = m_editPoste->text();
    employe.service = m_comboService->currentText();
    employe.dateRecrutement = m_editDate->date().toString("yyyy-MM-dd");
    employe.salaire = salaire;
    employe.soldeConges = solde;
    employe.statut = m_comboStatut->currentText();
    employe.motDePasse = m_editMotDePasse->text();
    employe.photo = m_photoCourante;

    QString erreur;
    if (!Employes::ajouter(employe, &erreur)) {
        QMessageBox::warning(this, "Ajout impossible",
                             erreur.isEmpty() ? "L'employé n'a pas pu être ajouté." : erreur);
        return;
    }

    m_idCourant = employe.id;
    rechargerDonnees();
    selectionnerEmploye(employe.id);
    emit donneesModifiees();
    QMessageBox::information(this, "Employé ajouté",
                             QString("%1 %2 a été ajouté avec succès.")
                                 .arg(employe.prenom, employe.nom));
}

void VueEmployes::slotModifier()
{
    if (m_idCourant <= 0) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un employé dans le tableau.");
        return;
    }

    bool salaireOk = false;
    const double salaire = lireMontant(m_editSalaire->text(), &salaireOk);
    bool soldeOk = false;
    const int solde = m_editSolde->text().trimmed().toInt(&soldeOk);
    if (!salaireOk || !soldeOk) {
        QMessageBox::warning(this, "Saisie incomplète",
                             "Veuillez saisir un salaire et un droit annuel valides.");
        return;
    }

    Employe employe = Employes::charger(m_idCourant);
    if (employe.id == 0) {
        QMessageBox::warning(this, "Employé introuvable",
                             "La fiche sélectionnée n'existe plus dans la base.");
        rechargerDonnees();
        return;
    }

    employe.nom = m_editNom->text();
    employe.prenom = m_editPrenom->text();
    employe.email = m_editEmail->text();
    employe.telephone = m_editTelephone->text();
    employe.poste = m_editPoste->text();
    employe.service = m_comboService->currentText();
    employe.dateRecrutement = m_editDate->date().toString("yyyy-MM-dd");
    employe.salaire = salaire;
    employe.soldeConges = solde;
    employe.statut = m_comboStatut->currentText();
    employe.photo = m_photoCourante;
    employe.motDePasse = m_editMotDePasse->text(); // vide : mot de passe conservé

    QString erreur;
    if (!Employes::modifier(employe, &erreur)) {
        QMessageBox::warning(this, "Modification impossible",
                             erreur.isEmpty() ? "Les informations n'ont pas pu être enregistrées."
                                              : erreur);
        return;
    }

    rechargerDonnees();
    selectionnerEmploye(employe.id);
    emit donneesModifiees();
    QMessageBox::information(this, "Fiche mise à jour",
                             "Les informations de l'employé ont été enregistrées.");
}

void VueEmployes::slotAnnuler()
{
    if (m_idCourant > 0) {
        chargerFiche(m_idCourant);
    } else {
        viderFormulaire();
    }
}

void VueEmployes::slotDesactiver()
{
    if (m_idCourant <= 0) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un employé dans le tableau.");
        return;
    }

    const Employe employe = Employes::charger(m_idCourant);
    if (employe.id == 0) {
        QMessageBox::warning(this, "Employé introuvable",
                             "La fiche sélectionnée n'existe plus dans la base.");
        rechargerDonnees();
        return;
    }
    if (employe.statut.compare("Inactif", Qt::CaseInsensitive) == 0) {
        QMessageBox::information(this, "Employé inactif",
                                 QString("%1 %2 est déjà inactif.")
                                     .arg(employe.prenom, employe.nom));
        return;
    }

    const QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Supprimer l'employé",
        QString("Retirer %1 %2 de la liste active ?\n\n"
                "La fiche, les congés et l'historique sont conservés. "
                "Le statut de l'employé devient « Inactif » et aucune nouvelle "
                "demande de congé ne pourra être créée.")
            .arg(employe.prenom, employe.nom),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reponse != QMessageBox::Yes) {
        return;
    }

    QString erreur;
    if (!Employes::desactiver(m_idCourant, &erreur)) {
        QMessageBox::warning(this, "Suppression impossible",
                             erreur.isEmpty() ? "L'opération a échoué." : erreur);
        return;
    }

    rechargerDonnees();
    selectionnerEmploye(m_idCourant);
    emit donneesModifiees();
    QMessageBox::information(this, "Employé supprimé de la liste active",
                             "La fiche a été conservée et l'employé est maintenant inactif.");
}

void VueEmployes::slotConsulter()
{
    if (m_idCourant <= 0) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un employé dans le tableau.");
        return;
    }
    emit ouvrirFicheDetaillee(m_idCourant);
}

void VueEmployes::slotConsulterHistorique()
{
    if (m_idCourant <= 0) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un employé dans le tableau.");
        return;
    }
    emit ouvrirHistoriqueEmploye(m_idCourant);
}

void VueEmployes::slotExporterFichePDF()
{
    if (m_idCourant <= 0) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un employé dans le tableau.");
        return;
    }
    emit exporterFicheEmploye(m_idCourant);
}

void VueEmployes::slotDoubleClic(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    const int id = m_modele->idLigne(index.row());
    if (id > 0) {
        emit ouvrirFicheDetaillee(id);
    }
}

void VueEmployes::slotChangerPhoto()
{
    const QString chemin = QFileDialog::getOpenFileName(
        this, "Sélectionner une photo", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (chemin.isEmpty()) {
        return;
    }

    const QString destination = copierPhoto(chemin);
    if (destination.isEmpty()) {
        return;
    }

    m_photoCourante = destination;
    mettreAJourPhotoApercu();
}

void VueEmployes::slotExporterPDF()
{
    if (m_modele->rowCount() == 0) {
        QMessageBox::information(this, "Export PDF",
                                 "Aucun employé ne correspond aux critères actuels.");
        return;
    }

    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString nom = dossier + "/Liste_employes_"
        + QDate::currentDate().toString("yyyyMMdd") + ".pdf";
    QString chemin = QFileDialog::getSaveFileName(
        this, "Exporter la liste des employés", nom, "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty()) {
        return;
    }
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive)) {
        chemin += ".pdf";
    }

    QStringList criteres;
    if (!m_recherche->text().trimmed().isEmpty()) {
        criteres << "Recherche : " + m_recherche->text().trimmed();
    }
    if (m_filtreService->currentIndex() > 0) {
        criteres << "Service : " + m_filtreService->currentText();
    }
    if (m_filtreStatut->currentIndex() > 0) {
        criteres << "Statut : " + m_filtreStatut->currentText();
    }
    if (criteres.isEmpty()) {
        criteres << "Tous les employés";
    }

    if (!Employes::exporterListePDF(m_modele->employesFiltres(), chemin,
                                    criteres.join(" — "))) {
        afficherErreur("Le fichier PDF n'a pas pu être créé.");
        return;
    }

    QMessageBox::information(this, "Export PDF",
                             QString("La liste a été exportée :\n\n%1").arg(chemin));
}

void VueEmployes::slotLigneCourante(const QModelIndex &courant,
                                    const QModelIndex &precedent)
{
    Q_UNUSED(precedent);
    if (!courant.isValid()) {
        viderFormulaire();
        return;
    }
    const int id = m_modele->idLigne(courant.row());
    if (id > 0) {
        chargerFiche(id);
    }
}

int VueEmployes::idSelectionne() const
{
    if (!m_table->selectionModel() ||
        m_table->selectionModel()->selectedRows().isEmpty()) {
        return 0;
    }
    const QModelIndex index = m_table->selectionModel()->selectedRows().first();
    return m_modele->idLigne(index.row());
}

QString VueEmployes::copierPhoto(const QString &source)
{
    const QFileInfo info(source);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, "Photo", "Le fichier image est introuvable.");
        return QString();
    }

    const QString dossier = Employes::dossierPhotos();
    QDir().mkpath(dossier);
    const QString nom = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz")
        + "_" + info.fileName();
    const QString destination = dossier + "/" + nom;
    if (!QFile::copy(source, destination)) {
        QMessageBox::warning(this, "Photo",
                             "La photo n'a pas pu être copiée dans le dossier de l'application.");
        return QString();
    }
    return destination;
}

void VueEmployes::filtrerParService(const QString &service)
{
    if (m_employes.isEmpty()) {
        rechargerDonnees();
    }
    if (service.trimmed().isEmpty()) {
        slotReinitialiser();
        return;
    }

    int index = m_filtreService->findText(service, Qt::MatchFixedString);
    if (index < 0) {
        const QSignalBlocker blocker(m_filtreService);
        m_filtreService->addItem(service);
        index = m_filtreService->count() - 1;
        m_filtreService->setCurrentIndex(index);
    } else {
        m_filtreService->setCurrentIndex(index);
    }
    m_modele->setService(service);
    mettreAJourCompteur();
}

void VueEmployes::afficherTousLesEmployes()
{
    slotReinitialiser();
}

void VueEmployes::selectionnerPremiereLigne()
{
    if (!m_modele || m_modele->rowCount() == 0) {
        viderFormulaire();
        return;
    }
    selectionnerEmploye(m_modele->idLigne(0));
}

void VueEmployes::selectionnerEmploye(int id)
{
    const int ligne = m_modele->lignePourId(id);
    if (ligne < 0) {
        return;
    }
    m_table->selectRow(ligne);
    const QModelIndex index = m_modele->index(ligne, ModeleEmployes::ColNom);
    m_table->scrollTo(index, QAbstractItemView::PositionAtCenter);
    chargerFiche(id);
}

void VueEmployes::afficherPhotoAgrandie(const QString &chemin)
{
    const QPixmap originale(chemin);
    if (originale.isNull()) {
        QMessageBox::warning(this, "Photo",
                             "La photo de cet employé est introuvable.");
        return;
    }

    const QScreen *ecran = screen();
    if (!ecran) {
        ecran = QGuiApplication::primaryScreen();
    }
    const QSize tailleMax = ecran
        ? ecran->availableGeometry().size() * 0.8
        : QSize(900, 700);
    const QPixmap apercu = originale.scaled(
        tailleMax, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QDialog dialogue(this);
    dialogue.setWindowTitle("Photo de l'employé");
    dialogue.setStyleSheet("QDialog { background-color: #162836; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialogue);
    layout->setContentsMargins(16, 16, 16, 12);
    layout->setSpacing(10);

    QPushButton *image = new QPushButton(&dialogue);
    image->setCursor(Qt::PointingHandCursor);
    image->setFlat(true);
    image->setIcon(QIcon(apercu));
    image->setIconSize(apercu.size());
    image->setFixedSize(apercu.size());
    image->setStyleSheet(
        "QPushButton { background: transparent; border: none; padding: 0; }");
    image->setToolTip("Cliquer sur la photo pour fermer");
    connect(image, &QPushButton::clicked, &dialogue, &QDialog::accept);
    layout->addWidget(image, 0, Qt::AlignCenter);

    QPushButton *fermer = new QPushButton("Fermer", &dialogue);
    fermer->setCursor(Qt::PointingHandCursor);
    fermer->setStyleSheet(
        "QPushButton { background-color: #179BAE; color: #FFFFFF; "
        "font-weight: bold; font-size: 11px; border-radius: 4px; "
        "padding: 7px 24px; border: none; } "
        "QPushButton:hover { background-color: #137A6E; }");
    connect(fermer, &QPushButton::clicked, &dialogue, &QDialog::accept);
    layout->addWidget(fermer, 0, Qt::AlignCenter);

    dialogue.exec();
}

void VueEmployes::afficherErreur(const QString &message)
{
    QMessageBox::warning(this, "Employés", message);
}
