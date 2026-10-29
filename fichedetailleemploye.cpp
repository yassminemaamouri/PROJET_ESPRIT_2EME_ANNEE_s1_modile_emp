#include "fichedetailleemploye.h"

#include "chartsbarres.h"
#include "chartsevolution.h"
#include "deleguesemployes.h"
#include "dialogemploye.h"
#include "employe.h"
#include "historiqueemploye.h"
#include "icones.h"
#include "outilsemployes.h"
#include "stylesemployes.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

namespace {

QPixmap avatarRond(const QString &chemin, const QString &nom,
                   const QString &prenom, int taille)
{
    QPixmap resultat(taille, taille);
    resultat.fill(Qt::transparent);

    QPainter painter(&resultat);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath cheminRond;
    cheminRond.addEllipse(0, 0, taille, taille);
    painter.setClipPath(cheminRond);

    QPixmap source(chemin);
    if (!chemin.trimmed().isEmpty() && !source.isNull()) {
        const QPixmap redimensionne = source.scaled(
            taille, taille, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap((taille - redimensionne.width()) / 2,
                           (taille - redimensionne.height()) / 2,
                           redimensionne);
    } else {
        painter.fillRect(QRect(0, 0, taille, taille),
                         couleurAvatar(nom, prenom));
        QFont police = painter.font();
        police.setBold(true);
        police.setPointSize(qMax(10, taille / 3));
        painter.setFont(police);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, 0, taille, taille), Qt::AlignCenter,
                         initiales(nom, prenom));
    }

    painter.setClipping(false);
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(1, 1, taille - 2, taille - 2);
    return resultat;
}

void styliserBadge(QLabel *badge, const QString &statut)
{
    QString fond = "#EEF3F7";
    QString texte = "#5B6B79";
    if (statut.compare("Actif", Qt::CaseInsensitive) == 0) {
        fond = "#E5F5EC";
        texte = "#137A4B";
    } else if (statut.compare("En congé", Qt::CaseInsensitive) == 0) {
        fond = "#FFF4DC";
        texte = "#A66A00";
    } else if (statut.compare("Inactif", Qt::CaseInsensitive) == 0) {
        fond = "#F1F4F7";
        texte = "#6B7C8C";
    }
    badge->setText(statut);
    badge->setStyleSheet(
        QString("background:%1; color:%2; border-radius:11px; "
                "padding:4px 12px; font-weight:600; font-size:8pt;")
            .arg(fond, texte));
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(24);
    badge->setMinimumWidth(84);
}

QString libelleTypeAction(const QString &type)
{
    if (type == "AJOUT") return "Création";
    if (type == "MODIFICATION") return "Modification";
    if (type == "STATUT") return "Changement de statut";
    if (type == "DESACTIVATION") return "Désactivation";
    if (type == "CONGE") return "Demande de congé";
    if (type == "CONGE_ACCEPTE") return "Congé accepté";
    if (type == "CONGE_REFUSE") return "Congé refusé";
    if (type == "CONGE_ANNULE") return "Congé annulé";
    return type;
}

QTableWidgetItem *itemLecture(const QString &texte,
                              Qt::Alignment alignement = Qt::AlignLeft | Qt::AlignVCenter)
{
    QTableWidgetItem *item = new QTableWidgetItem(texte);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(alignement);
    return item;
}

} // namespace

FicheDetailleeEmploye::FicheDetailleeEmploye(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageFiche");
    setStyleSheet(StylesEmployes::feuille());
    creerInterface();
}

void FicheDetailleeEmploye::creerInterface()
{
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->setContentsMargins(0, 0, 0, 0);
    principal->setSpacing(0);

    // Bannière supérieure : même hauteur et mêmes couleurs que l'ancien shell.
    QFrame *entete = new QFrame(this);
    entete->setFixedHeight(54);
    entete->setStyleSheet("background-color: #1B3346; "
                          "border-bottom: 2px solid #142735;");
    QHBoxLayout *layoutEntete = new QHBoxLayout(entete);
    layoutEntete->setContentsMargins(14, 0, 14, 0);
    layoutEntete->setSpacing(8);

    QPushButton *retour = new QPushButton("←  Retour", entete);
    retour->setCursor(Qt::PointingHandCursor);
    retour->setStyleSheet(
        "QPushButton { background: transparent; color: #A0B4C6; "
        "font-size: 11px; font-weight: bold; border: none; padding: 5px 8px; } "
        "QPushButton:hover { color: #FFFFFF; background: #203546; "
        "border-radius: 4px; }");
    connect(retour, &QPushButton::clicked, this, &FicheDetailleeEmploye::slotRetour);
    layoutEntete->addWidget(retour);

    QLabel *iconeTitre = new QLabel(entete);
    iconeTitre->setPixmap(Icones::dessiner(Icones::GroupeUtilisateurs,
                                           QColor("#179BAE"), 24));
    layoutEntete->addWidget(iconeTitre);

    m_titreFiche = new QLabel("FICHE EMPLOYÉ - DÉTAIL & HISTORIQUE", entete);
    m_titreFiche->setObjectName("titrePageFiche");
    m_titreFiche->setStyleSheet("color: #FFFFFF; font-size: 15px; "
                                "font-weight: bold; letter-spacing: 0.5px; "
                                "border: none;");
    layoutEntete->addWidget(m_titreFiche);
    layoutEntete->addStretch();

    m_btnPrecedent = new QPushButton("◀", entete);
    m_btnPrecedent->setFixedSize(28, 28);
    m_btnPrecedent->setCursor(Qt::PointingHandCursor);
    m_btnPrecedent->setToolTip("Employé précédent");
    m_btnPrecedent->setStyleSheet(
        "QPushButton { background: #24384B; color: #FFFFFF; border: none; "
        "border-radius: 4px; font-size: 11px; } "
        "QPushButton:hover { background: #2E4A62; } "
        "QPushButton:disabled { background: #203546; color: #557083; }");
    connect(m_btnPrecedent, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotPrecedent);
    layoutEntete->addWidget(m_btnPrecedent);

    m_btnSuivant = new QPushButton("▶", entete);
    m_btnSuivant->setFixedSize(28, 28);
    m_btnSuivant->setCursor(Qt::PointingHandCursor);
    m_btnSuivant->setToolTip("Employé suivant");
    m_btnSuivant->setStyleSheet(m_btnPrecedent->styleSheet());
    connect(m_btnSuivant, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotSuivant);
    layoutEntete->addWidget(m_btnSuivant);
    layoutEntete->addSpacing(6);

    QPushButton *exporter = new QPushButton("PDF", entete);
    exporter->setCursor(Qt::PointingHandCursor);
    exporter->setIcon(Icones::icone(Icones::DocumentPdf, Qt::white, 20));
    exporter->setStyleSheet(
        "QPushButton { background: #24384B; color: #FFFFFF; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 5px 10px; } QPushButton:hover { background: #2E4A62; }");
    connect(exporter, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotExporterFiche);
    layoutEntete->addWidget(exporter);

    QPushButton *modifier = new QPushButton("Modifier", entete);
    modifier->setCursor(Qt::PointingHandCursor);
    modifier->setIcon(Icones::icone(Icones::Crayon, Qt::white, 20));
    modifier->setStyleSheet(
        "QPushButton { background: #179BAE; color: #FFFFFF; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 5px 10px; } QPushButton:hover { background: #137A6E; }");
    connect(modifier, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotModifier);
    layoutEntete->addWidget(modifier);

    principal->addWidget(entete);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: #F0F4F8;");

    QWidget *contenu = new QWidget();
    contenu->setStyleSheet("background-color: #F0F4F8;");
    QVBoxLayout *layoutContenu = new QVBoxLayout(contenu);
    layoutContenu->setContentsMargins(16, 14, 16, 16);
    layoutContenu->setSpacing(12);

    // Carte de profil, dans l'esprit de l'ancienne fiche client.
    QFrame *profil = new QFrame(contenu);
    profil->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                          "border-radius: 8px;");
    profil->setMinimumHeight(138);
    QHBoxLayout *layoutProfil = new QHBoxLayout(profil);
    layoutProfil->setContentsMargins(20, 14, 20, 14);
    layoutProfil->setSpacing(18);

    m_avatar = new QLabel(profil);
    m_avatar->setFixedSize(96, 96);
    m_avatar->setAlignment(Qt::AlignCenter);
    layoutProfil->addWidget(m_avatar);

    QVBoxLayout *identite = new QVBoxLayout();
    identite->setSpacing(4);
    QHBoxLayout *ligneNom = new QHBoxLayout();
    ligneNom->setSpacing(12);
    m_nom = new QLabel(profil);
    m_nom->setStyleSheet("border: none; font-size: 19px; font-weight: bold; "
                         "color: #1B3346;");
    ligneNom->addWidget(m_nom);
    m_badge = new QLabel(profil);
    ligneNom->addWidget(m_badge);
    ligneNom->addStretch();
    identite->addLayout(ligneNom);

    m_sousTitre = new QLabel(profil);
    m_sousTitre->setStyleSheet("border: none; color: #64748B; font-size: 11px;");
    identite->addWidget(m_sousTitre);

    m_contact = new QLabel(profil);
    m_contact->setStyleSheet("border: none; color: #64748B; font-size: 11px;");
    m_contact->setTextInteractionFlags(Qt::TextSelectableByMouse);
    identite->addWidget(m_contact);

    m_btnPhoto = new QPushButton("Changer la photo", profil);
    m_btnPhoto->setCursor(Qt::PointingHandCursor);
    m_btnPhoto->setStyleSheet(
        "QPushButton { background: #F1F5F9; color: #475569; font-size: 10px; "
        "font-weight: bold; border: 1px solid #D8E2E9; border-radius: 4px; "
        "padding: 4px 8px; } QPushButton:hover { background: #E2E8F0; }");
    connect(m_btnPhoto, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotChangerPhoto);
    identite->addWidget(m_btnPhoto, 0, Qt::AlignLeft);
    identite->addStretch();
    layoutProfil->addLayout(identite, 1);

    QFrame *separateur = new QFrame(profil);
    separateur->setFrameShape(QFrame::VLine);
    separateur->setStyleSheet("color: #E1E8EE;");
    layoutProfil->addWidget(separateur);

    auto ajouterResumeProfil = [&](const QString &titre, QLabel *&valeur) {
        QFrame *carte = new QFrame(profil);
        carte->setStyleSheet("background: #F8FAFC; border: 1px solid #E2E8F0; "
                             "border-radius: 6px;");
        carte->setMinimumWidth(112);
        QVBoxLayout *colonne = new QVBoxLayout(carte);
        colonne->setContentsMargins(12, 8, 12, 8);
        colonne->setSpacing(2);
        QLabel *label = new QLabel(titre, carte);
        label->setStyleSheet("border: none; color: #64748B; font-size: 10px; "
                             "font-weight: bold;");
        valeur = new QLabel("—", carte);
        valeur->setStyleSheet("border: none; font-size: 20px; font-weight: bold; "
                              "color: #1B3346;");
        colonne->addWidget(label);
        colonne->addWidget(valeur);
        layoutProfil->addWidget(carte);
    };
    ajouterResumeProfil("Droit annuel", m_profilDroit);
    ajouterResumeProfil("Jours restants", m_profilRestants);

    layoutContenu->addWidget(profil);

    m_onglets = new QTabWidget(contenu);
    m_onglets->setObjectName("ongletsFiche");
    m_onglets->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #D8E2E9; background: #FFFFFF; "
        "border-radius: 6px; top: -1px; } "
        "QTabBar::tab { background: #E8EEF2; color: #1B3346; font-size: 11px; "
        "font-weight: bold; padding: 8px 20px; margin-right: 2px; "
        "border-top-left-radius: 4px; border-top-right-radius: 4px; } "
        "QTabBar::tab:selected { background: #1B3346; color: #FFFFFF; } "
        "QTabBar::tab:hover:!selected { background: #D8E2E9; }");
    m_ongletConges = creerOngletConges();
    m_ongletHistorique = creerOngletActivite();
    m_onglets->addTab(creerOngletInformations(), "Informations");
    m_onglets->addTab(m_ongletConges, "Congés de l'employé");
    m_onglets->addTab(creerOngletStatistiques(), "Statistiques");
    m_onglets->addTab(m_ongletHistorique, "Activité & historique");
    connect(m_onglets, &QTabWidget::currentChanged,
            this, &FicheDetailleeEmploye::slotOngletChange);
    layoutContenu->addWidget(m_onglets, 1);

    scroll->setWidget(contenu);
    principal->addWidget(scroll, 1);
}

QWidget *FicheDetailleeEmploye::creerChamp(const QString &libelle,
                                           QLabel *&valeur, QWidget *parent)
{
    QWidget *bloc = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(bloc);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    QLabel *titre = new QLabel(libelle, bloc);
    titre->setStyleSheet("border: none; color: #64748B; font-size: 10px; "
                         "font-weight: bold;");
    valeur = new QLabel("—", bloc);
    valeur->setStyleSheet("border: none; color: #1B3346; font-size: 11px; "
                          "font-weight: 600;");
    valeur->setWordWrap(true);
    valeur->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(titre);
    layout->addWidget(valeur);
    return bloc;
}

QWidget *FicheDetailleeEmploye::creerOngletInformations()
{
    QWidget *onglet = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(onglet);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    QFrame *carte = new QFrame(onglet);
    carte->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                         "border-radius: 6px;");
    QGridLayout *grille = new QGridLayout(carte);
    grille->setContentsMargins(22, 20, 22, 20);
    grille->setHorizontalSpacing(34);
    grille->setVerticalSpacing(18);
    grille->setColumnStretch(0, 1);
    grille->setColumnStretch(1, 1);

    grille->addWidget(creerChamp("Identifiant", m_valId, carte), 0, 0);
    grille->addWidget(creerChamp("Nom", m_valNom, carte), 0, 1);
    grille->addWidget(creerChamp("Prénom", m_valPrenom, carte), 1, 0);
    grille->addWidget(creerChamp("E-mail", m_valEmail, carte), 1, 1);
    grille->addWidget(creerChamp("Téléphone", m_valTelephone, carte), 2, 0);
    grille->addWidget(creerChamp("Poste", m_valPoste, carte), 2, 1);
    grille->addWidget(creerChamp("Service", m_valService, carte), 3, 0);
    grille->addWidget(creerChamp("Date de recrutement", m_valDate, carte), 3, 1);
    grille->addWidget(creerChamp("Salaire", m_valSalaire, carte), 4, 0);
    grille->addWidget(creerChamp("Droit annuel de congés", m_valDroit, carte), 4, 1);
    grille->addWidget(creerChamp("Statut", m_valStatut, carte), 5, 0);
    grille->addWidget(creerChamp("Mot de passe", m_valMotDePasse, carte), 5, 1);

    layout->addWidget(carte);

    QFrame *note = new QFrame(onglet);
    note->setStyleSheet("background: #F8FAFC; border: 1px solid #E2E8F0; "
                        "border-radius: 6px;");
    QVBoxLayout *layoutNote = new QVBoxLayout(note);
    layoutNote->setContentsMargins(18, 14, 18, 14);
    layoutNote->setSpacing(4);
    QLabel *titreNote = new QLabel("Règles de gestion appliquées", note);
    titreNote->setStyleSheet("border: none; color: #1B3346; font-size: 12px; "
                             "font-weight: bold;");
    QLabel *texteNote = new QLabel(
        "Le solde enregistré correspond au droit annuel. Les jours utilisés sont "
        "recalculés à partir des congés acceptés : une acceptation ne modifie jamais "
        "le droit annuel, ce qui évite toute double déduction. Les demandes en attente "
        "ou refusées ne sont pas déduites. Le mot de passe est conservé sous forme "
        "de hachage SHA-256 et n'est jamais affiché.",
        note);
    texteNote->setStyleSheet("border: none; color: #64748B; font-size: 11px;");
    texteNote->setWordWrap(true);
    layoutNote->addWidget(titreNote);
    layoutNote->addWidget(texteNote);
    layout->addWidget(note);
    layout->addStretch();
    return onglet;
}

QWidget *FicheDetailleeEmploye::creerCarteResume(const QString &titre,
                                                 const QString &sousTitre,
                                                 QLabel *&valeur, QWidget *parent)
{
    QFrame *carte = new QFrame(parent);
    carte->setStyleSheet("background: #F8FAFC; border: 1px solid #D8E2E9; "
                         "border-radius: 6px;");
    carte->setMinimumHeight(72);
    carte->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout *layout = new QVBoxLayout(carte);
    layout->setContentsMargins(14, 9, 14, 9);
    layout->setSpacing(1);

    QLabel *label = new QLabel(titre, carte);
    label->setStyleSheet("border: none; color: #64748B; font-size: 10px; "
                         "font-weight: bold;");
    valeur = new QLabel("—", carte);
    valeur->setStyleSheet("border: none; font-size: 18px; font-weight: bold; "
                          "color: #1B3346;");
    QLabel *sous = new QLabel(sousTitre, carte);
    sous->setStyleSheet("border: none; color: #7A8D9E; font-size: 9px;");

    layout->addWidget(label);
    layout->addWidget(valeur);
    layout->addWidget(sous);
    return carte;
}

QWidget *FicheDetailleeEmploye::creerOngletConges()
{
    QWidget *onglet = new QWidget(this);
    onglet->setStyleSheet("background-color: #E8EEF2;");
    QVBoxLayout *layout = new QVBoxLayout(onglet);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    // Barre de solde en haut
    QFrame *carteSolde = new QFrame(onglet);
    carteSolde->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                              "border-radius: 6px;");
    QHBoxLayout *layoutSolde = new QHBoxLayout(carteSolde);
    layoutSolde->setContentsMargins(16, 10, 16, 10);
    layoutSolde->setSpacing(16);

    auto addSoldeItem = [&](const QString &titre, QLabel **valeurOut, const QString &sousTitre) {
        QVBoxLayout *col = new QVBoxLayout();
        col->setSpacing(2);
        QLabel *titreLbl = new QLabel(titre, carteSolde);
        titreLbl->setStyleSheet("border: none; color: #64748B; font-size: 10px; font-weight: bold;");
        QLabel *val = new QLabel("—", carteSolde);
        val->setStyleSheet("border: none; color: #24384B; font-size: 18px; font-weight: bold;");
        QLabel *sous = new QLabel(sousTitre, carteSolde);
        sous->setStyleSheet("border: none; color: #94A3B8; font-size: 9px;");
        col->addWidget(titreLbl);
        col->addWidget(val);
        col->addWidget(sous);
        layoutSolde->addLayout(col);
        if (valeurOut) *valeurOut = val;
    };

    addSoldeItem("Droit annuel de congés", &m_valDroitConge, "Jours par an");
    addSoldeItem("Jours utilisés", &m_valUtilises, QString("Année %1").arg(QDate::currentDate().year()));
    addSoldeItem("Jours restants", &m_valRestants, "Disponibles");
    addSoldeItem("En attente de validation", &m_valAttente, "À valider");
    m_valDroitConge->setObjectName("balanceDroitAnnuel");
    m_valUtilises->setObjectName("balanceJoursUtilises");
    m_valRestants->setObjectName("balanceJoursRestants");
    m_valAttente->setObjectName("balanceJoursAttente");
    layoutSolde->addStretch();
    layout->addWidget(carteSolde);

    QHBoxLayout *milieu = new QHBoxLayout();
    milieu->setSpacing(12);

    // Formulaire de demande intégré
    QFrame *carteDemande = new QFrame(onglet);
    carteDemande->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                                "border-radius: 6px;");
    QVBoxLayout *layoutDemande = new QVBoxLayout(carteDemande);
    layoutDemande->setContentsMargins(14, 12, 14, 12);
    layoutDemande->setSpacing(8);

    QLabel *titreDemande = new QLabel("Demande de congé", carteDemande);
    titreDemande->setStyleSheet("border: none; color: #24384B; font-size: 12px; "
                                "font-weight: bold;");
    layoutDemande->addWidget(titreDemande);

    QGridLayout *formulaire = new QGridLayout();
    formulaire->setContentsMargins(0, 0, 0, 0);
    formulaire->setHorizontalSpacing(10);
    formulaire->setVerticalSpacing(7);
    formulaire->setColumnStretch(1, 1);
    formulaire->setColumnStretch(3, 1);

    auto labelFormulaire = [](const QString &texte) {
        QLabel *label = new QLabel(texte);
        label->setStyleSheet("border: none; color: #24384B; font-size: 10px; "
                             "font-weight: bold;");
        return label;
    };

    m_typeConge = new QComboBox(carteDemande);
    m_typeConge->setObjectName("champTypeConge");
    m_typeConge->setEditable(true);
    m_typeConge->addItems(Conge::types());
    m_typeConge->setCurrentText("Congé annuel");
    m_typeConge->setStyleSheet(
        "QComboBox { background: #F8FAFC; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 5px 8px; font-size: 11px; color: #1B3346; } "
        "QComboBox:focus { border-color: #179BAE; background: #FFFFFF; }");

    m_debutConge = new QDateEdit(QDate::currentDate(), carteDemande);
    m_debutConge->setObjectName("champDateDebutConge");
    m_debutConge->setDisplayFormat("dd/MM/yyyy");
    m_debutConge->setCalendarPopup(true);
    m_debutConge->setToolTip("Cliquez sur le calendrier pour choisir la date de début.");
    m_debutConge->setStyleSheet(
        "QDateEdit { background: #F8FAFC; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 5px 8px; font-size: 11px; color: #1B3346; } "
        "QDateEdit:focus { border-color: #179BAE; background: #FFFFFF; }");

    m_finConge = new QDateEdit(QDate::currentDate(), carteDemande);
    m_finConge->setObjectName("champDateFinConge");
    m_finConge->setDisplayFormat("dd/MM/yyyy");
    m_finConge->setCalendarPopup(true);
    m_finConge->setToolTip("Cliquez sur le calendrier pour choisir la date de fin.");
    m_finConge->setStyleSheet(m_debutConge->styleSheet());

    formulaire->addWidget(labelFormulaire("Type de congé"), 0, 0);
    formulaire->addWidget(m_typeConge, 0, 1, 1, 3);
    formulaire->addWidget(labelFormulaire("Date de début"), 1, 0);
    formulaire->addWidget(m_debutConge, 1, 1);
    formulaire->addWidget(labelFormulaire("Date de fin"), 1, 2);
    formulaire->addWidget(m_finConge, 1, 3);

    m_raisonConge = new QTextEdit(carteDemande);
    m_raisonConge->setObjectName("champRaisonConge");
    m_raisonConge->setPlaceholderText("Raison ou motif de la demande…");
    m_raisonConge->setFixedHeight(54);
    m_raisonConge->setStyleSheet(
        "QTextEdit { background: #F8FAFC; border: 1px solid #CCD7E0; "
        "border-radius: 4px; padding: 5px 8px; font-size: 11px; color: #1B3346; } "
        "QTextEdit:focus { border-color: #179BAE; background: #FFFFFF; }");
    formulaire->addWidget(labelFormulaire("Raison"), 2, 0);
    formulaire->addWidget(m_raisonConge, 2, 1, 1, 3);

    m_joursConge = new QLabel(carteDemande);
    m_joursConge->setObjectName("resumeJoursConge");
    m_joursConge->setStyleSheet("border: none; color: #137A6E; font-size: 10px; "
                                "font-weight: bold;");
    formulaire->addWidget(m_joursConge, 3, 1, 1, 3);

    layoutDemande->addLayout(formulaire);

    QHBoxLayout *boutonsDemande = new QHBoxLayout();
    boutonsDemande->setSpacing(8);
    boutonsDemande->addStretch();

    m_btnReinitialiser = new QPushButton("Annuler", carteDemande);
    m_btnReinitialiser->setObjectName("boutonAnnulerConge");
    m_btnReinitialiser->setCursor(Qt::PointingHandCursor);
    m_btnReinitialiser->setStyleSheet(
        "QPushButton { background: #E2E8F0; color: #475569; font-weight: bold; "
        "font-size: 10px; border: none; border-radius: 4px; padding: 6px 14px; } "
        "QPushButton:hover { background: #CBD5E1; }");
    connect(m_btnReinitialiser, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotReinitialiserDemande);
    boutonsDemande->addWidget(m_btnReinitialiser);

    m_btnSoumettre = new QPushButton("Soumettre la demande", carteDemande);
    m_btnSoumettre->setObjectName("boutonSoumettreConge");
    m_btnSoumettre->setCursor(Qt::PointingHandCursor);
    m_btnSoumettre->setStyleSheet(
        "QPushButton { background: #208A9A; color: #FFFFFF; font-weight: bold; "
        "font-size: 10px; border: none; border-radius: 4px; padding: 6px 16px; } "
        "QPushButton:hover { background: #1B7280; } "
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; }");
    connect(m_btnSoumettre, &QPushButton::clicked,
            this, &FicheDetailleeEmploye::slotSoumettreConge);
    boutonsDemande->addWidget(m_btnSoumettre);
    layoutDemande->addLayout(boutonsDemande);

    QLabel *aide = new QLabel(
        "Les jours ouvrables uniquement (lundi - vendredi). La demande est "
        "enregistrée avec le statut « En attente ».", carteDemande);
    aide->setStyleSheet("border: none; color: #7A8D9E; font-size: 9px;");
    aide->setWordWrap(true);
    layoutDemande->addWidget(aide);

    milieu->addWidget(carteDemande, 1);
    layout->addLayout(milieu);

    QHBoxLayout *barre = new QHBoxLayout();
    barre->setSpacing(8);
    QLabel *titreTable = new QLabel("Historique des congés de cet employé", onglet);
    titreTable->setStyleSheet("border: none; color: #24384B; font-size: 12px; "
                              "font-weight: bold;");
    barre->addWidget(titreTable);
    barre->addStretch();

    m_btnAccepter = new QPushButton("Accepter", onglet);
    m_btnAccepter->setObjectName("boutonAccepterConge");
    m_btnAccepter->setEnabled(false);
    m_btnAccepter->setCursor(Qt::PointingHandCursor);
    m_btnAccepter->setStyleSheet(
        "QPushButton { background: #208A9A; color: #FFFFFF; font-weight: bold; "
        "font-size: 10px; border: none; border-radius: 4px; padding: 6px 11px; } "
        "QPushButton:hover { background: #1B7280; } "
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; }");
    connect(m_btnAccepter, &QPushButton::clicked, this, &FicheDetailleeEmploye::slotAccepterConge);
    barre->addWidget(m_btnAccepter);

    m_btnRefuser = new QPushButton("Refuser", onglet);
    m_btnRefuser->setObjectName("boutonRefuserConge");
    m_btnRefuser->setEnabled(false);
    m_btnRefuser->setCursor(Qt::PointingHandCursor);
    m_btnRefuser->setStyleSheet(
        "QPushButton { background: #E6A15C; color: #FFFFFF; font-weight: bold; "
        "font-size: 10px; border: none; border-radius: 4px; padding: 6px 11px; } "
        "QPushButton:hover { background: #D08C44; } "
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; }");
    connect(m_btnRefuser, &QPushButton::clicked, this, &FicheDetailleeEmploye::slotRefuserConge);
    barre->addWidget(m_btnRefuser);

    m_btnAnnuler = new QPushButton("Annuler la demande", onglet);
    m_btnAnnuler->setObjectName("boutonAnnulerDemandeConge");
    m_btnAnnuler->setEnabled(false);
    m_btnAnnuler->setCursor(Qt::PointingHandCursor);
    m_btnAnnuler->setStyleSheet(
        "QPushButton { background: #E2E8F0; color: #475569; font-weight: bold; "
        "font-size: 10px; border: none; border-radius: 4px; padding: 6px 11px; } "
        "QPushButton:hover { background: #CBD5E1; } "
        "QPushButton:disabled { background: #F1F5F9; color: #A8B6C0; }");
    connect(m_btnAnnuler, &QPushButton::clicked, this, &FicheDetailleeEmploye::slotAnnulerConge);
    barre->addWidget(m_btnAnnuler);

    layout->addLayout(barre);

    QLabel *aideTable = new QLabel(
        "Sélectionnez une ligne pour accepter, refuser ou annuler une demande. "
        "L'annulation d'un congé accepté restaure automatiquement le solde calculé.",
        onglet);
    aideTable->setStyleSheet("border: none; color: #64748B; font-size: 10px;");
    aideTable->setWordWrap(true);
    layout->addWidget(aideTable);

    m_congeVide = new QLabel("Aucune demande de congé enregistrée pour cet employé.", onglet);
    m_congeVide->setStyleSheet("border: none; color: #94A3B8; font-size: 11px; "
                               "font-style: italic;");
    m_congeVide->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_congeVide);

    m_tableConges = new QTableWidget(onglet);
    m_tableConges->setObjectName("tableConges");
    m_tableConges->setStyleSheet(
        "QTableWidget { background: #FFFFFF; alternate-background-color: #F8FAFC; "
        "selection-background-color: #E0F2FE; selection-color: #0369A1; "
        "border: 1px solid #D8E2E9; border-radius: 4px; gridline-color: #EDF2F7; "
        "font-size: 11px; color: #24384B; } "
        "QHeaderView::section { background: #24384B; color: #FFFFFF; "
        "font-weight: bold; font-size: 10px; padding: 7px 5px; border: none; }");
    m_tableConges->setColumnCount(8);
    m_tableConges->setHorizontalHeaderLabels(
        {"ID", "Type", "Début", "Fin", "Jours", "Statut", "Décision", "Motif"});
    m_tableConges->verticalHeader()->hide();
    m_tableConges->setAlternatingRowColors(true);
    m_tableConges->setShowGrid(false);
    m_tableConges->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableConges->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableConges->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableConges->setWordWrap(false);
    m_tableConges->setTextElideMode(Qt::ElideRight);
    m_tableConges->verticalHeader()->setDefaultSectionSize(40);
    m_tableConges->horizontalHeader()->setFixedHeight(38);
    m_tableConges->horizontalHeader()->setHighlightSections(false);
    m_tableConges->setColumnHidden(0, true);
    m_tableConges->setColumnWidth(1, 180);
    m_tableConges->setColumnWidth(2, 96);
    m_tableConges->setColumnWidth(3, 96);
    m_tableConges->setColumnWidth(4, 60);
    m_tableConges->setColumnWidth(5, 110);
    m_tableConges->setColumnWidth(6, 145);
    m_tableConges->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    m_tableConges->setItemDelegateForColumn(5, new DelegueStatutEmploye(this));
    m_tableConges->setMinimumHeight(210);
    connect(m_tableConges, &QTableWidget::itemSelectionChanged,
            this, &FicheDetailleeEmploye::slotSelectionConge);
    layout->addWidget(m_tableConges, 1);

    connect(m_debutConge, &QDateEdit::dateChanged,
            this, &FicheDetailleeEmploye::slotActualiserJoursDemande);
    connect(m_finConge, &QDateEdit::dateChanged,
            this, &FicheDetailleeEmploye::slotActualiserJoursDemande);

    m_btnSoumettre->setEnabled(false);
    slotActualiserJoursDemande();
    return onglet;
}

QWidget *FicheDetailleeEmploye::creerOngletStatistiques()
{
    QWidget *onglet = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(onglet);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    m_resumeStatistiques = new QLabel(onglet);
    m_resumeStatistiques->setStyleSheet("border: none; color: #64748B; "
                                        "font-size: 11px;");
    m_resumeStatistiques->setWordWrap(true);
    layout->addWidget(m_resumeStatistiques);

    QHBoxLayout *charts = new QHBoxLayout();
    charts->setSpacing(12);
    m_chartConges = new ChartsEvolution(onglet);
    m_chartConges->setTitre("Évolution des congés");
    m_chartConges->setUnite("jours");
    m_chartConges->setCouleur(QColor("#C47A24"));
    m_chartConges->setMinimumHeight(280);

    m_chartTypes = new ChartsBarres(onglet);
    m_chartTypes->setTitre("Jours par type de congé");
    m_chartTypes->setSousTitre("Congés acceptés");
    m_chartTypes->setCouleur(QColor("#7A5AA6"));
    m_chartTypes->setMinimumHeight(280);

    charts->addWidget(m_chartConges, 3);
    charts->addWidget(m_chartTypes, 2);
    layout->addLayout(charts, 1);

    QLabel *note = new QLabel(
        "Les jours sont calculés en jours ouvrables (lundi à vendredi). "
        "Seuls les congés acceptés sont comptabilisés dans l'évolution.",
        onglet);
    note->setStyleSheet("border: none; color: #64748B; font-size: 10px;");
    note->setWordWrap(true);
    layout->addWidget(note);
    return onglet;
}

QWidget *FicheDetailleeEmploye::creerOngletActivite()
{
    QWidget *onglet = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(onglet);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);

    QLabel *titre = new QLabel("Historique complet des actions", onglet);
    titre->setStyleSheet("border: none; color: #1B3346; font-size: 12px; "
                         "font-weight: bold;");
    QLabel *sousTitre = new QLabel(
        "Chaque création, modification, désactivation et décision de congé est "
        "enregistrée durablement dans la base.", onglet);
    sousTitre->setStyleSheet("border: none; color: #64748B; font-size: 10px;");
    sousTitre->setWordWrap(true);
    layout->addWidget(titre);
    layout->addWidget(sousTitre);

    m_histoVide = new QLabel("Aucune activité enregistrée pour cet employé.", onglet);
    m_histoVide->setStyleSheet("border: none; color: #94A3B8; font-size: 11px; "
                               "font-style: italic;");
    m_histoVide->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_histoVide);

    QFrame *carte = new QFrame(onglet);
    carte->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                         "border-radius: 4px;");
    QVBoxLayout *layoutCarte = new QVBoxLayout(carte);
    layoutCarte->setContentsMargins(6, 6, 6, 6);

    m_tableActivite = new QTableWidget(carte);
    m_tableActivite->setObjectName("tableActivite");
    m_tableActivite->setStyleSheet(
        "QTableWidget { background: #FFFFFF; alternate-background-color: #F8FAFC; "
        "selection-background-color: #E0F2FE; selection-color: #0369A1; "
        "border: none; gridline-color: #EDF2F7; font-size: 11px; "
        "color: #24384B; } "
        "QHeaderView::section { background: #24384B; color: #FFFFFF; "
        "font-weight: bold; font-size: 10px; padding: 7px 5px; border: none; }");
    m_tableActivite->setColumnCount(6);
    m_tableActivite->setHorizontalHeaderLabels(
        {"Date", "Type", "Description", "Ancienne valeur", "Nouvelle valeur", "Utilisateur"});
    m_tableActivite->verticalHeader()->hide();
    m_tableActivite->setAlternatingRowColors(true);
    m_tableActivite->setShowGrid(false);
    m_tableActivite->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableActivite->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableActivite->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableActivite->setWordWrap(false);
    m_tableActivite->setTextElideMode(Qt::ElideRight);
    m_tableActivite->verticalHeader()->setDefaultSectionSize(40);
    m_tableActivite->horizontalHeader()->setFixedHeight(38);
    m_tableActivite->horizontalHeader()->setHighlightSections(false);
    m_tableActivite->setColumnWidth(0, 136);
    m_tableActivite->setColumnWidth(1, 140);
    m_tableActivite->setColumnWidth(3, 150);
    m_tableActivite->setColumnWidth(4, 150);
    m_tableActivite->setColumnWidth(5, 112);
    m_tableActivite->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    layoutCarte->addWidget(m_tableActivite);

    QHBoxLayout *corps = new QHBoxLayout();
    corps->setSpacing(12);
    corps->addWidget(carte, 3);

    QFrame *carteIndicateurs = new QFrame(onglet);
    carteIndicateurs->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                                    "border-radius: 4px;");
    carteIndicateurs->setMinimumWidth(220);
    carteIndicateurs->setMaximumWidth(270);
    QVBoxLayout *layoutIndicateurs = new QVBoxLayout(carteIndicateurs);
    layoutIndicateurs->setContentsMargins(10, 10, 10, 10);
    layoutIndicateurs->setSpacing(8);

    QLabel *titreIndicateurs = new QLabel("Activité & Indicateurs",
                                          carteIndicateurs);
    titreIndicateurs->setStyleSheet("border: none; color: #24384B; "
                                    "font-size: 12px; font-weight: bold;");
    layoutIndicateurs->addWidget(titreIndicateurs);
    layoutIndicateurs->addWidget(
        creerCarteResume("Actions enregistrées", "historique complet",
                         m_histoTotal, carteIndicateurs));
    layoutIndicateurs->addWidget(
        creerCarteResume("Types d'actions", "distincts", m_histoTypes,
                         carteIndicateurs));
    layoutIndicateurs->addWidget(
        creerCarteResume("Demandes de congé", "enregistrées", m_histoConges,
                         carteIndicateurs));
    layoutIndicateurs->addWidget(
        creerCarteResume("Dernière action", "date du journal", m_histoDerniere,
                         carteIndicateurs));
    layoutIndicateurs->addStretch();

    corps->addWidget(carteIndicateurs, 0);
    layout->addLayout(corps, 1);
    return onglet;
}

bool FicheDetailleeEmploye::chargerEmploye(int id)
{
    m_id = id;
    m_employe = Employes::charger(id);
    if (m_employe.id == 0) {
        return false;
    }
    rafraichirTout();
    slotReinitialiserDemande();
    emit employeAffiche(m_id);
    return true;
}

void FicheDetailleeEmploye::exporterFichePDF()
{
    slotExporterFiche();
}

void FicheDetailleeEmploye::afficherOngletHistorique()
{
    if (!m_onglets || !m_ongletHistorique) {
        return;
    }
    m_onglets->setCurrentWidget(m_ongletHistorique);
    mettreAJourTitre();
}

void FicheDetailleeEmploye::rafraichirTout()
{
    rafraichirEntete();
    rafraichirInformations();
    rafraichirConges();
    rafraichirStatistiques();
    rafraichirActivite();
}

void FicheDetailleeEmploye::rafraichirEntete()
{
    m_avatar->setPixmap(avatarRond(m_employe.photo, m_employe.nom,
                                   m_employe.prenom, 96));
    m_nom->setText(QString("%1 %2").arg(m_employe.prenom, m_employe.nom).trimmed());
    m_sousTitre->setText(QString("Employé #%1  ·  %2  ·  %3")
                             .arg(m_employe.id)
                             .arg(m_employe.poste.isEmpty() ? "Poste non renseigné" : m_employe.poste)
                             .arg(m_employe.service.isEmpty() ? "Service non renseigné" : m_employe.service));
    m_contact->setText(QString("%1   ·   %2")
                           .arg(m_employe.email.isEmpty() ? "E-mail non renseigné" : m_employe.email,
                                m_employe.telephone.isEmpty() ? "Téléphone non renseigné" : m_employe.telephone));
    styliserBadge(m_badge, m_employe.statut);

    mettreAJourTitre();

    const int droit = Conge::droitAnnuel(m_id);
    const int restants = Conge::joursRestants(m_id);
    m_profilDroit->setText(QString("%1 j").arg(droit));
    m_profilRestants->setText(QString("%1 j").arg(restants));

    // Navigation réelle entre les fiches existantes, dans l'ordre des identifiants.
    if (m_btnPrecedent && m_btnSuivant) {
        const QVector<Employe> tous = Employes::chargerTous();
        int position = -1;
        for (int i = 0; i < tous.size(); ++i) {
            if (tous.at(i).id == m_id) {
                position = i;
                break;
            }
        }
        m_btnPrecedent->setEnabled(position > 0);
        m_btnSuivant->setEnabled(position >= 0 && position + 1 < tous.size());
    }
}

void FicheDetailleeEmploye::rafraichirInformations()
{
    m_valId->setText(QString("#%1").arg(m_employe.id));
    m_valNom->setText(m_employe.nom.isEmpty() ? "—" : m_employe.nom);
    m_valPrenom->setText(m_employe.prenom.isEmpty() ? "—" : m_employe.prenom);
    m_valEmail->setText(m_employe.email.isEmpty() ? "—" : m_employe.email);
    m_valTelephone->setText(m_employe.telephone.isEmpty() ? "—" : m_employe.telephone);
    m_valPoste->setText(m_employe.poste.isEmpty() ? "—" : m_employe.poste);
    m_valService->setText(m_employe.service.isEmpty() ? "—" : m_employe.service);
    m_valDate->setText(m_employe.dateRecrutement.isEmpty()
                           ? "—" : dateAffichage(m_employe.dateRecrutement));
    m_valSalaire->setText(salaireTexte(m_employe.salaire));
    m_valDroit->setText(QString("%1 jours").arg(m_employe.soldeConges));
    m_valStatut->setText(m_employe.statut);
    m_valMotDePasse->setText(m_employe.motDePasse.trimmed().isEmpty()
                                 ? "Non défini"
                                 : "Défini (haché SHA-256)");
}

void FicheDetailleeEmploye::rafraichirConges()
{
    m_conges = Conge::chargerPourEmploye(m_id);

    m_tableConges->setRowCount(0);
    m_tableConges->setRowCount(m_conges.size());

    for (int i = 0; i < m_conges.size(); ++i) {
        const Conge &c = m_conges.at(i);
        QTableWidgetItem *id = itemLecture(QString::number(c.id), Qt::AlignCenter);
        id->setData(Qt::UserRole, c.id);
        m_tableConges->setItem(i, 0, id);
        m_tableConges->setItem(i, 1, itemLecture(c.typeConge.isEmpty() ? "—" : c.typeConge));
        m_tableConges->setItem(i, 2, itemLecture(dateAffichage(c.dateDebut), Qt::AlignCenter));
        m_tableConges->setItem(i, 3, itemLecture(dateAffichage(c.dateFin), Qt::AlignCenter));
        m_tableConges->setItem(i, 4, itemLecture(QString::number(c.nbJours), Qt::AlignCenter));

        QTableWidgetItem *statut = itemLecture(statutCongeNormalise(c.statut), Qt::AlignCenter);
        m_tableConges->setItem(i, 5, statut);
        m_tableConges->setItem(i, 6, itemLecture(
            c.dateDecision.trimmed().isEmpty() ? "—" : dateHeureAffichage(c.dateDecision),
            Qt::AlignCenter));
        if (!c.validePar.trimmed().isEmpty()) {
            m_tableConges->item(i, 6)->setToolTip("Validé par : " + c.validePar);
        }
        QTableWidgetItem *motif = itemLecture(c.motif.trimmed().isEmpty() ? "—" : c.motif);
        motif->setToolTip(c.motif);
        m_tableConges->setItem(i, 7, motif);
        m_tableConges->setRowHeight(i, 42);
    }

    m_congeVide->setVisible(m_conges.isEmpty());

    const int droit = Conge::droitAnnuel(m_id);
    const int utilises = Conge::joursUtilises(m_id);
    const int restants = qMax(0, droit - utilises);
    const int attente = Conge::joursEnAttente(m_id);
    m_valDroitConge->setText(QString("%1 j").arg(droit));
    m_valUtilises->setText(QString("%1 j").arg(utilises));
    m_valRestants->setText(QString("%1 j").arg(restants));
    m_valAttente->setText(QString("%1 j").arg(attente));

    m_idCongeSelectionne = 0;
    m_tableConges->clearSelection();
    m_tableConges->setCurrentCell(-1, -1);
    mettreAJourBoutonsConges();
    slotActualiserJoursDemande();
}

void FicheDetailleeEmploye::rafraichirStatistiques()
{
    const int annee = QDate::currentDate().year();
    QVector<int> mois(12, 0);
    QVector<QPair<QString, int>> types;
    int totalAccepte = 0;
    int enAttente = 0;

    for (const Conge &c : m_conges) {
        const QString statut = statutCongeNormalise(c.statut);
        if (statut == "En attente") {
            enAttente += c.nbJours;
        }
        if (statut != "Accepté" || !c.dateDebut.isValid() ||
            c.dateDebut.year() != annee) {
            continue;
        }
        totalAccepte += c.nbJours;
        const int indice = c.dateDebut.month() - 1;
        if (indice >= 0 && indice < 12) {
            mois[indice] += c.nbJours;
        }

        bool trouve = false;
        for (auto &paire : types) {
            if (paire.first.compare(c.typeConge, Qt::CaseInsensitive) == 0) {
                paire.second += c.nbJours;
                trouve = true;
                break;
            }
        }
        if (!trouve) {
            types.append({c.typeConge.trimmed().isEmpty()
                              ? QString("Non renseigné") : c.typeConge, c.nbJours});
        }
    }

    std::sort(types.begin(), types.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first.localeAwareCompare(b.first) < 0;
              });

    m_chartConges->setDonnees(mois);
    m_chartConges->setSousTitre(QString("Année %1 — jours ouvrables acceptés").arg(annee));
    m_chartTypes->setDonnees(types);
    m_chartTypes->setSousTitre(QString("Congés acceptés en %1").arg(annee));

    m_resumeStatistiques->setText(
        QString("Année %1 : %2 jour(s) accepté(s), %3 jour(s) en attente. "
                "Droit annuel : %4 jour(s), dont %5 jour(s) restant(s).")
            .arg(annee)
            .arg(totalAccepte)
            .arg(enAttente)
            .arg(Conge::droitAnnuel(m_id))
            .arg(Conge::joursRestants(m_id)));
}

void FicheDetailleeEmploye::rafraichirActivite()
{
    const QVector<EntreeHistorique> historique =
        HistoriqueEmploye::chargerPourEmploye(m_id);

    m_tableActivite->setRowCount(0);
    m_tableActivite->setRowCount(historique.size());

    QSet<QString> typesActions;
    QString derniereAction;

    for (int i = 0; i < historique.size(); ++i) {
        const EntreeHistorique &h = historique.at(i);
        typesActions.insert(h.typeAction);
        if (derniereAction.isEmpty() || h.dateAction > derniereAction) {
            derniereAction = h.dateAction;
        }
        QTableWidgetItem *date = itemLecture(dateHeureAffichage(h.dateAction), Qt::AlignCenter);
        date->setToolTip(h.dateAction);
        m_tableActivite->setItem(i, 0, date);
        m_tableActivite->setItem(i, 1, itemLecture(libelleTypeAction(h.typeAction)));
        QTableWidgetItem *description = itemLecture(h.description);
        description->setToolTip(h.description);
        m_tableActivite->setItem(i, 2, description);
        QTableWidgetItem *ancienne = itemLecture(h.ancienneValeur.trimmed().isEmpty()
                                                    ? "—" : h.ancienneValeur);
        ancienne->setToolTip(h.ancienneValeur);
        m_tableActivite->setItem(i, 3, ancienne);
        QTableWidgetItem *nouvelle = itemLecture(h.nouvelleValeur.trimmed().isEmpty()
                                                   ? "—" : h.nouvelleValeur);
        nouvelle->setToolTip(h.nouvelleValeur);
        m_tableActivite->setItem(i, 4, nouvelle);
        m_tableActivite->setItem(i, 5, itemLecture(h.utilisateur));
        m_tableActivite->setRowHeight(i, 40);
    }

    m_histoTotal->setText(QString::number(historique.size()));
    m_histoTypes->setText(QString::number(typesActions.size()));
    m_histoConges->setText(QString::number(m_conges.size()));
    m_histoDerniere->setText(derniereAction.isEmpty()
                                 ? "—" : dateAffichage(derniereAction));

    m_histoVide->setVisible(historique.isEmpty());
}

int FicheDetailleeEmploye::idCongeSelectionne() const
{
    if (!m_tableConges->selectionModel() ||
        m_tableConges->selectionModel()->selectedRows().isEmpty()) {
        return 0;
    }
    const int ligne = m_tableConges->currentRow();
    if (ligne < 0) {
        return 0;
    }
    const QTableWidgetItem *item = m_tableConges->item(ligne, 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

void FicheDetailleeEmploye::mettreAJourBoutonsConges()
{
    int id = m_idCongeSelectionne;
    QString statut;
    if (id > 0) {
        const int ligne = m_tableConges->currentRow();
        if (ligne >= 0 && m_tableConges->item(ligne, 5)) {
            statut = m_tableConges->item(ligne, 5)->text();
        }
    }

    const bool enAttente = id > 0 && statut == "En attente";
    const bool annulable = id > 0 && (statut == "En attente" || statut == "Accepté");
    m_btnAccepter->setEnabled(enAttente);
    m_btnRefuser->setEnabled(enAttente);
    m_btnAnnuler->setEnabled(annulable);

    const bool employeActif = m_id > 0 &&
        m_employe.statut.compare("Inactif", Qt::CaseInsensitive) != 0;
    m_btnSoumettre->setEnabled(employeActif);
}

void FicheDetailleeEmploye::mettreAJourTitre()
{
    if (!m_titreFiche) {
        return;
    }

    if (m_onglets && m_ongletConges &&
        m_onglets->currentWidget() == m_ongletConges) {
        m_titreFiche->setText("GESTION DES EMPLOYÉS - GESTION DES CONGÉS");
        return;
    }

    if (m_employe.id > 0) {
        m_titreFiche->setText(
            QString("FICHE EMPLOYÉ - %1 %2")
                .arg(m_employe.prenom, m_employe.nom).toUpper());
    } else {
        m_titreFiche->setText("FICHE EMPLOYÉ - DÉTAIL & HISTORIQUE");
    }
}

void FicheDetailleeEmploye::slotOngletChange(int index)
{
    Q_UNUSED(index);
    mettreAJourTitre();
}

void FicheDetailleeEmploye::slotSelectionConge()
{
    m_idCongeSelectionne = idCongeSelectionne();
    mettreAJourBoutonsConges();
}

void FicheDetailleeEmploye::slotRetour()
{
    emit retourListe();
}

void FicheDetailleeEmploye::slotPrecedent()
{
    const QVector<Employe> tous = Employes::chargerTous();
    for (int i = 1; i < tous.size(); ++i) {
        if (tous.at(i).id == m_id) {
            chargerEmploye(tous.at(i - 1).id);
            return;
        }
    }
}

void FicheDetailleeEmploye::slotSuivant()
{
    const QVector<Employe> tous = Employes::chargerTous();
    for (int i = 0; i + 1 < tous.size(); ++i) {
        if (tous.at(i).id == m_id) {
            chargerEmploye(tous.at(i + 1).id);
            return;
        }
    }
}

void FicheDetailleeEmploye::slotChangerPhoto()
{
    if (m_id <= 0) {
        return;
    }

    const QString source = QFileDialog::getOpenFileName(
        this, "Changer la photo de l'employé", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (source.isEmpty()) {
        return;
    }

    const QFileInfo info(source);
    QDir().mkpath(Employes::dossierPhotos());
    const QString nom = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz")
        + "_" + info.fileName();
    const QString destination = Employes::dossierPhotos() + "/" + nom;
    if (!QFile::copy(source, destination)) {
        QMessageBox::warning(this, "Photo",
                             "La photo n'a pas pu être copiée dans le dossier de l'application.");
        return;
    }

    m_employe.photo = destination;
    m_employe.motDePasse.clear();
    QString erreur;
    if (!Employes::modifier(m_employe, &erreur)) {
        QMessageBox::warning(this, "Photo",
                             erreur.isEmpty() ? "La photo n'a pas pu être enregistrée."
                                              : erreur);
        return;
    }

    chargerEmploye(m_id);
    emit employeModifie();
}

void FicheDetailleeEmploye::slotModifier()
{
    if (m_id <= 0) {
        return;
    }
    DialogEmploye dialogue(this);
    dialogue.chargerEmploye(m_employe);
    if (dialogue.exec() != QDialog::Accepted) {
        return;
    }

    m_employe = Employes::charger(m_id);
    rafraichirTout();
    emit employeModifie();
}

void FicheDetailleeEmploye::slotReinitialiserDemande()
{
    if (!m_typeConge || !m_debutConge || !m_finConge || !m_raisonConge) {
        return;
    }

    m_typeConge->setCurrentText("Congé annuel");
    const QDate aujourdHui = QDate::currentDate();
    m_debutConge->setDate(aujourdHui);
    m_finConge->setDate(aujourdHui);
    m_raisonConge->clear();
    slotActualiserJoursDemande();
}

void FicheDetailleeEmploye::slotActualiserJoursDemande()
{
    if (!m_joursConge || !m_debutConge || !m_finConge) {
        return;
    }

    const QDate debut = m_debutConge->date();
    const QDate fin = m_finConge->date();
    const int jours = Conge::joursOuvrables(debut, fin);
    const int restants = m_id > 0 ? Conge::joursRestants(m_id) : 0;

    QString texte;
    QString couleur = "#137A6E";
    if (fin < debut) {
        texte = "La date de fin doit être postérieure ou égale à la date de début.";
        couleur = "#B3261E";
    } else if (jours <= 0) {
        texte = "La période ne contient aucun jour ouvrable.";
        couleur = "#B3261E";
    } else if (jours > restants) {
        texte = QString("%1 jour(s) demandé(s) — solde insuffisant (%2 jour(s) disponible(s)).")
                    .arg(jours)
                    .arg(restants);
        couleur = "#B45309";
    } else {
        texte = QString("%1 jour(s) ouvrable(s) — %2 jour(s) disponible(s) sur le solde.")
                    .arg(jours)
                    .arg(restants);
    }

    m_joursConge->setText(texte);
    m_joursConge->setStyleSheet(
        QString("border: none; color: %1; font-size: 10px; font-weight: bold;")
            .arg(couleur));
}

void FicheDetailleeEmploye::slotSoumettreConge()
{
    if (m_id <= 0 || !m_typeConge || !m_debutConge || !m_finConge ||
        !m_raisonConge) {
        return;
    }
    if (m_employe.statut.compare("Inactif", Qt::CaseInsensitive) == 0) {
        QMessageBox::information(
            this, "Demande de congé",
            "Un employé inactif ne peut pas recevoir de nouvelle demande de congé.");
        return;
    }

    const QDate debut = m_debutConge->date();
    const QDate fin = m_finConge->date();
    if (fin < debut) {
        QMessageBox::warning(this, "Demande de congé",
                             "La date de fin doit être postérieure ou égale à la date de début.");
        return;
    }

    Conge conge;
    conge.employeId = m_id;
    conge.typeConge = m_typeConge->currentText().trimmed();
    conge.dateDebut = debut;
    conge.dateFin = fin;
    conge.motif = m_raisonConge->toPlainText().trimmed();

    QString erreur;
    if (!conge.ajouter(&erreur)) {
        QMessageBox::warning(this, "Demande de congé",
                             erreur.isEmpty() ? "La demande n'a pas pu être enregistrée."
                                              : erreur);
        return;
    }

    rafraichirTout();
    emit employeModifie();
    QMessageBox::information(
        this, "Demande enregistrée",
        QString("La demande de %1 jour(s) du %2 au %3 a été enregistrée "
                "avec le statut « En attente ».")
            .arg(conge.nbJours)
            .arg(dateAffichage(conge.dateDebut), dateAffichage(conge.dateFin)));
    slotReinitialiserDemande();
}

void FicheDetailleeEmploye::slotExporterFiche()
{
    if (m_id <= 0) {
        return;
    }
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString nom = QString("Fiche_%1_%2_%3.pdf")
        .arg(m_employe.nom, m_employe.prenom,
             QDate::currentDate().toString("yyyyMMdd"));
    QString chemin = QFileDialog::getSaveFileName(
        this, "Exporter la fiche employé", dossier + "/" + nom, "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty()) {
        return;
    }
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive)) {
        chemin += ".pdf";
    }

    if (!Employes::exporterFichePDF(m_employe, chemin)) {
        QMessageBox::warning(this, "Export PDF",
                             "La fiche n'a pas pu être exportée.");
        return;
    }
    QMessageBox::information(this, "Export PDF",
                             QString("La fiche a été exportée :\n\n%1").arg(chemin));
}

void FicheDetailleeEmploye::slotAccepterConge()
{
    const int id = m_idCongeSelectionne;
    if (id <= 0) {
        return;
    }
    const Conge c = Conge::charger(id);
    if (c.id == 0) {
        QMessageBox::warning(this, "Congé", "Demande introuvable.");
        return;
    }

    const QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Accepter la demande",
        QString("Accepter le congé « %1 » du %2 au %3 (%4 jour(s)) ?\n\n"
                "Le solde sera recalculé à partir des congés acceptés.")
            .arg(c.typeConge, dateAffichage(c.dateDebut), dateAffichage(c.dateFin))
            .arg(c.nbJours),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reponse != QMessageBox::Yes) {
        return;
    }

    QString erreur;
    if (!Conge::accepter(id, &erreur)) {
        QMessageBox::warning(this, "Acceptation impossible",
                             erreur.isEmpty() ? "Une erreur est survenue." : erreur);
        return;
    }
    rafraichirTout();
    emit employeModifie();
}

void FicheDetailleeEmploye::slotRefuserConge()
{
    const int id = m_idCongeSelectionne;
    if (id <= 0) {
        return;
    }

    bool ok = false;
    const QString motif = QInputDialog::getMultiLineText(
        this, "Refuser la demande",
        "Motif du refus (optionnel) :", QString(), &ok);
    if (!ok) {
        return;
    }

    QString erreur;
    if (!Conge::refuser(id, motif, &erreur)) {
        QMessageBox::warning(this, "Refus impossible",
                             erreur.isEmpty() ? "Une erreur est survenue." : erreur);
        return;
    }
    rafraichirTout();
    emit employeModifie();
}

void FicheDetailleeEmploye::slotAnnulerConge()
{
    const int id = m_idCongeSelectionne;
    if (id <= 0) {
        return;
    }
    const Conge c = Conge::charger(id);
    if (c.id == 0) {
        QMessageBox::warning(this, "Congé", "Demande introuvable.");
        return;
    }

    const QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Annuler le congé",
        QString("Annuler le congé « %1 » du %2 au %3 ?\n\n"
                "S'il était accepté, les jours seront de nouveau disponibles.")
            .arg(c.typeConge, dateAffichage(c.dateDebut), dateAffichage(c.dateFin)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reponse != QMessageBox::Yes) {
        return;
    }

    QString erreur;
    if (!Conge::annuler(id, &erreur)) {
        QMessageBox::warning(this, "Annulation impossible",
                             erreur.isEmpty() ? "Une erreur est survenue." : erreur);
        return;
    }
    rafraichirTout();
    emit employeModifie();
}
