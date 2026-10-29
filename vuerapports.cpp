#include "vuerapports.h"

#include "chartsbarres.h"
#include "conge.h"
#include "employe.h"
#include "icones.h"
#include "kpicard.h"
#include "outilsemployes.h"
#include "stylesemployes.h"

#include <QAbstractItemView>
#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace {

struct LigneService
{
    QString service;
    int effectif = 0;
    int actifs = 0;
    int enConge = 0;
    int inactifs = 0;
    double masse = 0.0;
    int joursUtilises = 0;

    double salaireMoyen() const
    {
        return effectif > 0 ? masse / effectif : 0.0;
    }
};

QString champCsv(QString valeur)
{
    valeur.replace(';', ',');
    valeur.replace('\n', ' ');
    valeur.replace('\r', ' ');
    return valeur.trimmed();
}

QString nombreFrancais(double valeur)
{
    return QString::number(valeur, 'f', 2).replace('.', ',');
}

QTableWidgetItem *itemRapport(const QString &texte,
                              Qt::Alignment alignement = Qt::AlignLeft | Qt::AlignVCenter)
{
    QTableWidgetItem *item = new QTableWidgetItem(texte);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(alignement);
    return item;
}

} // namespace

VueRapports::VueRapports(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageRapports");
    setStyleSheet(StylesEmployes::feuille());
    creerInterface();
    rechargerDonnees();
}

void VueRapports::creerInterface()
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
    iconeBanniere->setPixmap(Icones::dessiner(Icones::Rapports,
                                               QColor("#179BAE"), 26));
    QLabel *titreBanniere = new QLabel("GESTION DES EMPLOYÉS - RAPPORTS",
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
        "Synthèse des effectifs, de la masse salariale, des services et des congés.",
        page);
    sousTitre->setStyleSheet("border: none; color: #64748B; font-size: 11px;");
    entete->addWidget(sousTitre, 1);

    QPushButton *actualiser = new QPushButton("Actualiser", page);
    actualiser->setCursor(Qt::PointingHandCursor);
    actualiser->setStyleSheet(
        "QPushButton { background: #E2E8F0; color: #334155; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 6px 12px; } QPushButton:hover { background: #CBD5E1; }");
    connect(actualiser, &QPushButton::clicked, this, &VueRapports::rechargerDonnees);
    entete->addWidget(actualiser);

    QPushButton *exporter = new QPushButton("Exporter en CSV", page);
    exporter->setCursor(Qt::PointingHandCursor);
    exporter->setStyleSheet(
        "QPushButton { background: #179BAE; color: #FFFFFF; font-size: 11px; "
        "font-weight: bold; border: none; border-radius: 4px; "
        "padding: 6px 12px; } QPushButton:hover { background: #137A6E; }");
    connect(exporter, &QPushButton::clicked, this, &VueRapports::slotExporterCSV);
    entete->addWidget(exporter);
    principal->addLayout(entete);

    QHBoxLayout *kpis = new QHBoxLayout();
    kpis->setSpacing(14);
    m_kpiMasse = new KpiCard("Masse salariale", "0 DT", "Somme des salaires",
                             Icones::Mallette, KpiCard::NavyTeal, this);
    m_kpiMoyenne = new KpiCard("Salaire moyen", "0 DT", "Par employé",
                               Icones::Utilisateur, KpiCard::Emerald, this);
    m_kpiJours = new KpiCard("Jours de congé utilisés", "0", "Année en cours",
                             Icones::Calendrier, KpiCard::OceanBlue, this);
    m_kpiAttente = new KpiCard("Demandes en attente", "0", "À valider",
                               Icones::Horloge, KpiCard::SlateDark, this);
    kpis->addWidget(m_kpiMasse);
    kpis->addWidget(m_kpiMoyenne);
    kpis->addWidget(m_kpiJours);
    kpis->addWidget(m_kpiAttente);
    principal->addLayout(kpis);

    QHBoxLayout *charts = new QHBoxLayout();
    charts->setSpacing(14);
    m_chartServices = new ChartsBarres(this);
    m_chartServices->setTitre("Effectif par service");
    m_chartServices->setCouleur(QColor("#179BAE"));
    m_chartServices->setMinimumHeight(230);
    charts->addWidget(m_chartServices, 1);

    m_chartTypes = new ChartsBarres(this);
    m_chartTypes->setTitre("Jours de congé par type");
    m_chartTypes->setSousTitre("Congés acceptés");
    m_chartTypes->setCouleur(QColor("#7A5AA6"));
    m_chartTypes->setMinimumHeight(230);
    charts->addWidget(m_chartTypes, 1);
    principal->addLayout(charts, 1);

    QFrame *carte = new QFrame(this);
    carte->setStyleSheet("background: #FFFFFF; border: 1px solid #D8E2E9; "
                         "border-radius: 8px;");
    QVBoxLayout *layoutCarte = new QVBoxLayout(carte);
    layoutCarte->setContentsMargins(18, 14, 18, 12);
    layoutCarte->setSpacing(8);

    QLabel *titreTable = new QLabel("Synthèse par service", carte);
    titreTable->setStyleSheet("border: none; color: #1B3346; font-size: 12px; "
                              "font-weight: bold;");
    layoutCarte->addWidget(titreTable);

    QLabel *sousTitreTable = new QLabel(
        "Les jours utilisés correspondent aux congés acceptés de l'année en cours, "
        "calculés en jours ouvrables.", carte);
    sousTitreTable->setStyleSheet("border: none; color: #64748B; font-size: 10px;");
    sousTitreTable->setWordWrap(true);
    layoutCarte->addWidget(sousTitreTable);

    m_videServices = new QLabel("Aucun employé enregistré : la synthèse est vide.", carte);
    m_videServices->setObjectName("texteVide");
    m_videServices->setAlignment(Qt::AlignCenter);
    layoutCarte->addWidget(m_videServices);

    m_tableServices = new QTableWidget(carte);
    m_tableServices->setObjectName("tableRapports");
    m_tableServices->setStyleSheet(
        "QTableWidget { background: #FFFFFF; alternate-background-color: #F8FAFC; "
        "selection-background-color: #E0F2FE; selection-color: #0369A1; "
        "border: 1px solid #E2E8F0; border-radius: 4px; gridline-color: #EDF2F7; "
        "font-size: 11px; color: #24384B; } "
        "QHeaderView::section { background: #1E3A4F; color: #FFFFFF; "
        "font-weight: bold; font-size: 10px; padding: 7px 5px; border: none; }");
    m_tableServices->setColumnCount(8);
    m_tableServices->setHorizontalHeaderLabels(
        {"Service", "Effectif", "Actifs", "En congé", "Inactifs",
         "Masse salariale", "Salaire moyen", "Jours utilisés"});
    m_tableServices->verticalHeader()->hide();
    m_tableServices->setAlternatingRowColors(true);
    m_tableServices->setShowGrid(false);
    m_tableServices->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableServices->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableServices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableServices->setWordWrap(false);
    m_tableServices->setTextElideMode(Qt::ElideRight);
    m_tableServices->verticalHeader()->setDefaultSectionSize(38);
    m_tableServices->horizontalHeader()->setFixedHeight(38);
    m_tableServices->horizontalHeader()->setHighlightSections(false);
    m_tableServices->horizontalHeader()->setMinimumSectionSize(60);
    m_tableServices->setColumnWidth(1, 82);
    m_tableServices->setColumnWidth(2, 76);
    m_tableServices->setColumnWidth(3, 88);
    m_tableServices->setColumnWidth(4, 82);
    m_tableServices->setColumnWidth(5, 132);
    m_tableServices->setColumnWidth(6, 126);
    m_tableServices->setColumnWidth(7, 108);
    m_tableServices->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    layoutCarte->addWidget(m_tableServices, 1);

    principal->addWidget(carte, 1);
}

void VueRapports::rechargerDonnees()
{
    const int annee = QDate::currentDate().year();
    const QVector<Employe> employes = Employes::chargerTous();
    const QVector<Conge> conges = Conge::chargerTous();

    QHash<int, int> joursParEmploye;
    double masseTotale = 0.0;
    int totalJoursUtilises = 0;
    for (const Employe &e : employes) {
        masseTotale += e.salaire;
        const int jours = Conge::joursUtilises(e.id, annee);
        joursParEmploye.insert(e.id, jours);
        totalJoursUtilises += jours;
    }
    const double salaireMoyen = employes.isEmpty() ? 0.0 : masseTotale / employes.size();

    QVector<LigneService> lignes;
    for (const Employe &e : employes) {
        const QString service = e.service.trimmed().isEmpty()
            ? QString("Non renseigné") : e.service.trimmed();
        int index = -1;
        for (int i = 0; i < lignes.size(); ++i) {
            if (lignes.at(i).service.compare(service, Qt::CaseInsensitive) == 0) {
                index = i;
                break;
            }
        }
        if (index < 0) {
            LigneService ligne;
            ligne.service = service;
            lignes.append(ligne);
            index = lignes.size() - 1;
        }

        LigneService &ligne = lignes[index];
        ++ligne.effectif;
        ligne.masse += e.salaire;
        ligne.joursUtilises += joursParEmploye.value(e.id, 0);
        if (e.statut.compare("Actif", Qt::CaseInsensitive) == 0) {
            ++ligne.actifs;
        } else if (e.statut.compare("En congé", Qt::CaseInsensitive) == 0) {
            ++ligne.enConge;
        } else {
            ++ligne.inactifs;
        }
    }

    std::sort(lignes.begin(), lignes.end(),
              [](const LigneService &a, const LigneService &b) {
                  if (a.effectif != b.effectif) return a.effectif > b.effectif;
                  return a.service.localeAwareCompare(b.service) < 0;
              });

    int demandesEnAttente = 0;
    int joursEnAttente = 0;
    QVector<QPair<QString, int>> joursParType;
    for (const Conge &c : conges) {
        const QString statut = statutCongeNormalise(c.statut);
        if (statut == "En attente") {
            ++demandesEnAttente;
            joursEnAttente += c.nbJours;
        } else if (statut == "Accepté" && c.dateDebut.isValid()
                   && c.dateDebut.year() == annee) {
            bool trouve = false;
            for (auto &paire : joursParType) {
                if (paire.first.compare(c.typeConge, Qt::CaseInsensitive) == 0) {
                    paire.second += c.nbJours;
                    trouve = true;
                    break;
                }
            }
            if (!trouve) {
                joursParType.append({c.typeConge.trimmed().isEmpty()
                                         ? QString("Non renseigné") : c.typeConge,
                                     c.nbJours});
            }
        }
    }
    std::sort(joursParType.begin(), joursParType.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first.localeAwareCompare(b.first) < 0;
              });

    m_kpiMasse->setValeur(salaireTexte(masseTotale));
    m_kpiMasse->setEvolution(QString("%1 employé(s)").arg(employes.size()));
    m_kpiMoyenne->setValeur(salaireTexte(salaireMoyen));
    m_kpiMoyenne->setEvolution("Moyenne de l'effectif");
    m_kpiJours->setValeur(QString::number(totalJoursUtilises));
    m_kpiJours->setEvolution(QString("Année %1").arg(annee));
    m_kpiAttente->setValeur(QString::number(demandesEnAttente));
    m_kpiAttente->setEvolution(QString("%1 jour(s) à valider").arg(joursEnAttente));

    QVector<QPair<QString, int>> parService;
    for (const LigneService &l : lignes) {
        parService.append({l.service, l.effectif});
    }
    m_chartServices->setDonnees(parService);
    m_chartServices->setSousTitre(QString("%1 employé(s) au total").arg(employes.size()));
    m_chartTypes->setDonnees(joursParType);
    m_chartTypes->setSousTitre(QString("Année %1 — jours ouvrables").arg(annee));

    m_tableServices->setRowCount(0);
    m_tableServices->setRowCount(lignes.size());
    for (int i = 0; i < lignes.size(); ++i) {
        const LigneService &l = lignes.at(i);
        m_tableServices->setItem(i, 0, itemRapport(l.service));
        m_tableServices->setItem(i, 1, itemRapport(QString::number(l.effectif), Qt::AlignCenter));
        m_tableServices->setItem(i, 2, itemRapport(QString::number(l.actifs), Qt::AlignCenter));
        m_tableServices->setItem(i, 3, itemRapport(QString::number(l.enConge), Qt::AlignCenter));
        m_tableServices->setItem(i, 4, itemRapport(QString::number(l.inactifs), Qt::AlignCenter));
        m_tableServices->setItem(i, 5, itemRapport(salaireTexte(l.masse),
                                                    Qt::AlignRight | Qt::AlignVCenter));
        m_tableServices->setItem(i, 6, itemRapport(salaireTexte(l.salaireMoyen()),
                                                    Qt::AlignRight | Qt::AlignVCenter));
        m_tableServices->setItem(i, 7, itemRapport(QString::number(l.joursUtilises),
                                                    Qt::AlignCenter));
        m_tableServices->setRowHeight(i, 38);
    }
    const bool vide = lignes.isEmpty();
    m_tableServices->setVisible(!vide);
    m_videServices->setVisible(vide);

    m_lignesCsv.clear();
    m_lignesCsv << QString("Rapport des employés;généré le;%1")
                       .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    m_lignesCsv << QString();
    m_lignesCsv << "Indicateur;Valeur";
    m_lignesCsv << QString("Effectif total;%1").arg(employes.size());
    m_lignesCsv << QString("Masse salariale (DT);%1").arg(nombreFrancais(masseTotale));
    m_lignesCsv << QString("Salaire moyen (DT);%1").arg(nombreFrancais(salaireMoyen));
    m_lignesCsv << QString("Jours de congé utilisés (%1);%2").arg(annee).arg(totalJoursUtilises);
    m_lignesCsv << QString("Demandes en attente;%1").arg(demandesEnAttente);
    m_lignesCsv << QString("Jours en attente;%1").arg(joursEnAttente);
    m_lignesCsv << QString();
    m_lignesCsv << "Service;Effectif;Actifs;En congé;Inactifs;"
                   "Masse salariale (DT);Salaire moyen (DT);Jours utilisés";
    for (const LigneService &l : lignes) {
        m_lignesCsv << QString("%1;%2;%3;%4;%5;%6;%7;%8")
                           .arg(champCsv(l.service))
                           .arg(l.effectif)
                           .arg(l.actifs)
                           .arg(l.enConge)
                           .arg(l.inactifs)
                           .arg(nombreFrancais(l.masse))
                           .arg(nombreFrancais(l.salaireMoyen()))
                           .arg(l.joursUtilises);
    }
    m_lignesCsv << QString();
    m_lignesCsv << "Type de congé;Jours acceptés";
    for (const auto &paire : joursParType) {
        m_lignesCsv << QString("%1;%2").arg(champCsv(paire.first)).arg(paire.second);
    }
}

void VueRapports::slotExporterCSV()
{
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString nomParDefaut = dossier + "/Rapport_employes_"
        + QDate::currentDate().toString("yyyyMMdd") + ".csv";

    QString chemin = QFileDialog::getSaveFileName(
        this, "Exporter la synthèse", nomParDefaut, "Fichiers CSV (*.csv)");
    if (chemin.isEmpty()) {
        return;
    }
    if (!chemin.endsWith(".csv", Qt::CaseInsensitive)) {
        chemin += ".csv";
    }

    if (!exporterCSV(chemin)) {
        QMessageBox::warning(this, "Export CSV",
                             "Le fichier n'a pas pu être écrit.");
        return;
    }

    QMessageBox::information(
        this, "Export CSV",
        QString("La synthèse a été exportée :\n\n%1").arg(chemin));
}

bool VueRapports::exporterCSV(const QString &cheminFichier)
{
    if (cheminFichier.trimmed().isEmpty()) {
        return false;
    }

    QFile fichier(cheminFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    fichier.write("\xEF\xBB\xBF");
    fichier.write(m_lignesCsv.join("\r\n").toUtf8());
    const bool ok = fichier.error() == QFileDevice::NoError;
    fichier.close();
    return ok;
}
