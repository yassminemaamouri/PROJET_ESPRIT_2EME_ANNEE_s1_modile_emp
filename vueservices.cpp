#include "vueservices.h"

#include "employe.h"
#include "icones.h"
#include "outilsemployes.h"
#include "stylesemployes.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <algorithm>

namespace {

struct GroupeService
{
    QString nom;
    QVector<Employe> employes;
    int actifs = 0;
    int enConge = 0;
    int inactifs = 0;
};

} // namespace

VueServices::VueServices(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageServices");
    setStyleSheet(StylesEmployes::feuille());
    creerInterface();
    rechargerDonnees();
}

void VueServices::creerInterface()
{
    QVBoxLayout *racine = new QVBoxLayout(this);
    racine->setContentsMargins(0, 0, 0, 0);
    racine->setSpacing(0);

    QFrame *banniere = new QFrame(this);
    banniere->setFixedHeight(54);
    banniere->setStyleSheet("background-color: #1B3346; "
                            "border-bottom: 2px solid #142735;");
    QHBoxLayout *layoutBanniere = new QHBoxLayout(banniere);
    layoutBanniere->setContentsMargins(20, 0, 20, 0);
    layoutBanniere->setSpacing(8);
    QLabel *iconeBanniere = new QLabel(banniere);
    iconeBanniere->setPixmap(Icones::dessiner(Icones::Services,
                                               QColor("#179BAE"), 26));
    QLabel *titreBanniere = new QLabel("GESTION DES EMPLOYÉS - SERVICES",
                                       banniere);
    titreBanniere->setStyleSheet("color: #FFFFFF; font-size: 15px; "
                                 "font-weight: bold; letter-spacing: 0.5px; "
                                 "border: none;");
    QLabel *iconeAdmin = new QLabel(banniere);
    iconeAdmin->setPixmap(Icones::dessiner(Icones::Avatar,
                                            QColor("#179BAE"), 24));
    QLabel *admin = new QLabel("Administrateur", banniere);
    admin->setStyleSheet("color: #A0B4C6; font-size: 12px; font-weight: 500; "
                         "border: none;");
    layoutBanniere->addWidget(iconeBanniere);
    layoutBanniere->addSpacing(8);
    layoutBanniere->addWidget(titreBanniere);
    layoutBanniere->addStretch();
    layoutBanniere->addWidget(iconeAdmin);
    layoutBanniere->addSpacing(6);
    layoutBanniere->addWidget(admin);
    racine->addWidget(banniere);

    QWidget *page = new QWidget(this);
    page->setStyleSheet("background-color: #F0F4F8;");
    QVBoxLayout *principal = new QVBoxLayout(page);
    principal->setContentsMargins(16, 14, 16, 16);
    principal->setSpacing(12);
    racine->addWidget(page, 1);

    QHBoxLayout *entete = new QHBoxLayout();
    QLabel *sousTitre = new QLabel(
        "Répartition des employés par service et accès direct aux effectifs.", page);
    sousTitre->setStyleSheet("border: none; color: #64748B; font-size: 11px;");
    entete->addWidget(sousTitre, 1);

    QPushButton *actualiser = new QPushButton("Actualiser", page);
    actualiser->setCursor(Qt::PointingHandCursor);
    actualiser->setStyleSheet(
        "QPushButton { background: #E2E8F0; color: #334155; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 6px 12px; } QPushButton:hover { background: #CBD5E1; }");
    connect(actualiser, &QPushButton::clicked, this, &VueServices::rechargerDonnees);
    entete->addWidget(actualiser);

    QPushButton *tous = new QPushButton("Voir tous les employés", page);
    tous->setCursor(Qt::PointingHandCursor);
    tous->setStyleSheet(
        "QPushButton { background: #179BAE; color: #FFFFFF; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 6px 12px; } QPushButton:hover { background: #137A6E; }");
    connect(tous, &QPushButton::clicked, this, &VueServices::ouvrirEmployes);
    entete->addWidget(tous);

    principal->addLayout(entete);

    m_zone = new QScrollArea(page);
    m_zone->setWidgetResizable(true);
    m_zone->setFrameShape(QFrame::NoFrame);
    m_zone->setStyleSheet("QScrollArea { background: transparent; }");

    m_contenu = new QWidget(m_zone);
    m_contenu->setStyleSheet("background: transparent;");
    m_grille = new QGridLayout(m_contenu);
    m_grille->setContentsMargins(0, 0, 0, 0);
    m_grille->setHorizontalSpacing(14);
    m_grille->setVerticalSpacing(14);
    m_grille->setColumnStretch(0, 1);
    m_grille->setColumnStretch(1, 1);

    m_vide = new QLabel("Aucun employé enregistré : créez une fiche pour voir les services.",
                        m_contenu);
    m_vide->setObjectName("texteVide");
    m_vide->setAlignment(Qt::AlignCenter);
    m_vide->hide();

    m_zone->setWidget(m_contenu);
    principal->addWidget(m_zone, 1);
}

void VueServices::viderCartes()
{
    while (QLayoutItem *item = m_grille->takeAt(0)) {
        if (item->widget()) {
            // Suppression immédiate : les anciennes cartes ne doivent pas
            // rester visibles au-dessus des nouvelles pendant un rechargement.
            delete item->widget();
        }
        delete item;
    }
}

void VueServices::rechargerDonnees()
{
    viderCartes();

    const QVector<Employe> employes = Employes::chargerTous();
    QVector<GroupeService> groupes;

    for (const Employe &e : employes) {
        const QString nomService = e.service.trimmed().isEmpty()
            ? QString("Non renseigné") : e.service.trimmed();
        int index = -1;
        for (int i = 0; i < groupes.size(); ++i) {
            if (groupes.at(i).nom.compare(nomService, Qt::CaseInsensitive) == 0) {
                index = i;
                break;
            }
        }
        if (index < 0) {
            GroupeService groupe;
            groupe.nom = nomService;
            groupes.append(groupe);
            index = groupes.size() - 1;
        }

        groupes[index].employes.append(e);
        if (e.statut.compare("Actif", Qt::CaseInsensitive) == 0) {
            ++groupes[index].actifs;
        } else if (e.statut.compare("En congé", Qt::CaseInsensitive) == 0) {
            ++groupes[index].enConge;
        } else {
            ++groupes[index].inactifs;
        }
    }

    std::sort(groupes.begin(), groupes.end(),
              [](const GroupeService &a, const GroupeService &b) {
                  if (a.employes.size() != b.employes.size()) {
                      return a.employes.size() > b.employes.size();
                  }
                  return a.nom.localeAwareCompare(b.nom) < 0;
              });

    if (groupes.isEmpty()) {
        m_vide->show();
        m_grille->addWidget(m_vide, 0, 0, 1, 2);
        return;
    }
    m_vide->hide();

    int ligne = 0;
    int colonne = 0;
    for (const GroupeService &groupe : groupes) {
        QFrame *carte = new QFrame(m_contenu);
        carte->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                             "border-radius: 8px;");
        carte->setMinimumHeight(174);
        QVBoxLayout *layout = new QVBoxLayout(carte);
        layout->setContentsMargins(18, 16, 18, 14);
        layout->setSpacing(8);

        QHBoxLayout *entete = new QHBoxLayout();
        entete->setSpacing(10);
        QLabel *initiale = new QLabel(carte);
        initiale->setFixedSize(38, 38);
        initiale->setAlignment(Qt::AlignCenter);
        const QColor couleur = couleurAvatar(groupe.nom, QString());
        initiale->setStyleSheet(
            QString("background:%1; color:white; border-radius:19px; "
                    "font-size:16pt; font-weight:600;")
                .arg(couleur.name()));
        initiale->setText(groupe.nom.isEmpty() ? "?" : groupe.nom.left(1).toUpper());
        entete->addWidget(initiale);

        QLabel *nom = new QLabel(groupe.nom, carte);
        nom->setStyleSheet("border: none; color: #1B3346; font-size: 13px; "
                           "font-weight: bold;");
        entete->addWidget(nom);
        entete->addStretch();

        QLabel *compteur = new QLabel(
            QString("%1 employé%2").arg(groupe.employes.size())
                .arg(groupe.employes.size() > 1 ? "s" : ""), carte);
        compteur->setStyleSheet(
            "background:#E7F4F6; color:#137A6E; border-radius:10px; "
            "padding:3px 10px; font-size:8pt; font-weight:600;");
        compteur->setFixedHeight(22);
        entete->addWidget(compteur);
        layout->addLayout(entete);

        QLabel *statuts = new QLabel(
            QString("%1 actif(s)  ·  %2 en congé  ·  %3 inactif(s)")
                .arg(groupe.actifs).arg(groupe.enConge).arg(groupe.inactifs), carte);
        statuts->setStyleSheet("border: none; color: #64748B; font-size: 10px;");
        layout->addWidget(statuts);

        QStringList noms;
        const int maximum = 5;
        for (int i = 0; i < groupe.employes.size() && i < maximum; ++i) {
            const Employe &e = groupe.employes.at(i);
            noms << QString("%1 %2").arg(e.prenom, e.nom).trimmed();
        }
        if (groupe.employes.size() > maximum) {
            noms << QString("+%1 autre(s)").arg(groupe.employes.size() - maximum);
        }
        QLabel *equipe = new QLabel(noms.join(", "), carte);
        equipe->setStyleSheet("border: none; color: #24384B; font-size: 10px;");
        equipe->setWordWrap(true);
        layout->addWidget(equipe);
        layout->addStretch();

        QPushButton *voir = new QPushButton("Voir les employés", carte);
        voir->setCursor(Qt::PointingHandCursor);
        voir->setStyleSheet(
            "QPushButton { background: transparent; color: #179BAE; "
            "font-size: 11px; font-weight: bold; border: none; padding: 3px 0; "
            "text-align: left; } QPushButton:hover { color: #137A6E; "
            "text-decoration: underline; }");
        const QString service = groupe.nom == "Non renseigné" ? QString() : groupe.nom;
        connect(voir, &QPushButton::clicked, this, [this, service] {
            emit filtrerEmployesParService(service);
        });
        layout->addWidget(voir, 0, Qt::AlignLeft);

        m_grille->addWidget(carte, ligne, colonne);
        ++colonne;
        if (colonne == 2) {
            colonne = 0;
            ++ligne;
        }
    }

    m_grille->setRowStretch(ligne + 1, 1);
}
