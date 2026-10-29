#include "mainwindow.h"

#include "fichedetailleemploye.h"
#include "stylesemployes.h"
#include "vueaccueil.h"
#include "vueemployes.h"
#include "vuerapports.h"
#include "vueservices.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

/*
 * Le fichier logo.png possède un fond blanc. On le détoure et on le
 * transforme en tracé clair pour l'intégrer directement au fond bleu nuit,
 * sans rectangle blanc ni texte.
 */
QPixmap logoBarreLaterale(int taille)
{
    QImage source(":/images/logo.png");
    if (source.isNull()) {
        return Icones::dessiner(Icones::Logo, QColor("#EAF5F8"), 96)
            .scaled(taille, taille, Qt::KeepAspectRatio,
                    Qt::SmoothTransformation);
    }

    source = source.convertToFormat(QImage::Format_ARGB32);

    int minX = source.width();
    int minY = source.height();
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < source.height(); ++y) {
        for (int x = 0; x < source.width(); ++x) {
            const QColor c = source.pixelColor(x, y);
            const int minC = qMin(c.red(), qMin(c.green(), c.blue()));
            const int maxC = qMax(c.red(), qMax(c.green(), c.blue()));
            if (c.alpha() > 0 && (minC < 235 || maxC - minC > 20)) {
                minX = qMin(minX, x);
                minY = qMin(minY, y);
                maxX = qMax(maxX, x);
                maxY = qMax(maxY, y);
            }
        }
    }
    if (maxX >= minX && maxY >= minY) {
        source = source.copy(QRect(minX, minY, maxX - minX + 1,
                                   maxY - minY + 1));
    }

    source = source.scaled(taille, taille, Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    QImage resultat(source.size(), QImage::Format_ARGB32);
    resultat.fill(Qt::transparent);
    for (int y = 0; y < source.height(); ++y) {
        for (int x = 0; x < source.width(); ++x) {
            const QColor c = source.pixelColor(x, y);
            const int minC = qMin(c.red(), qMin(c.green(), c.blue()));
            const int alpha = qBound(0, (255 - minC) * 2, 255);
            if (alpha < 16) {
                continue;
            }
            QColor clair("#EAF5F8");
            clair.setAlpha(alpha);
            resultat.setPixelColor(x, y, clair);
        }
    }
    return QPixmap::fromImage(resultat);
}

const char *STYLE_MENU_ACTIF =
    "QPushButton {"
    "  background-color: #179BAE;"
    "  color: #FFFFFF;"
    "  font-size: 11px;"
    "  font-weight: bold;"
    "  border-radius: 6px;"
    "  border: none;"
    "  padding: 9px 12px;"
    "  text-align: left;"
    "}"
    "QPushButton:hover { background-color: #137A6E; }";

const char *STYLE_MENU_INACTIF =
    "QPushButton {"
    "  background-color: transparent;"
    "  color: #8EA8BE;"
    "  font-size: 11px;"
    "  font-weight: 500;"
    "  border-radius: 6px;"
    "  border: none;"
    "  padding: 9px 12px;"
    "  text-align: left;"
    "}"
    "QPushButton:hover { background-color: #203546; color: #FFFFFF; }";

const char *STYLE_MENU_INDISPONIBLE =
    "QPushButton {"
    "  background-color: transparent;"
    "  color: #5D7688;"
    "  font-size: 11px;"
    "  font-weight: 500;"
    "  border-radius: 6px;"
    "  border: none;"
    "  padding: 9px 12px;"
    "  text-align: left;"
    "}"
    "QPushButton:disabled { color: #5D7688; }";

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart Weather Management — Gestion des Employés");
    resize(1394, 882);
    setMinimumSize(1180, 720);
    setStyleSheet(StylesEmployes::feuille());

    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #F0F4F8;");
    setCentralWidget(central);

    QHBoxLayout *principal = new QHBoxLayout(central);
    principal->setContentsMargins(0, 0, 0, 0);
    principal->setSpacing(0);
    principal->addWidget(creerBarreLaterale());

    m_pages = new QStackedWidget(this);
    m_pageAccueil = new VueAccueil(this);
    m_pageEmployes = new VueEmployes(this);
    m_pageFiche = new FicheDetailleeEmploye(this);
    m_pageServices = new VueServices(this);
    m_pageRapports = new VueRapports(this);

    m_pages->addWidget(m_pageAccueil);    // PageAccueil
    m_pages->addWidget(m_pageEmployes);   // PageEmployes
    m_pages->addWidget(m_pageFiche);      // PageFiche
    m_pages->addWidget(m_pageServices);   // PageServices
    m_pages->addWidget(m_pageRapports);   // PageRapports
    principal->addWidget(m_pages, 1);

    connect(m_pageAccueil, &VueAccueil::ouvrirEmploye,
            this, &MainWindow::slotOuvrirFiche);
    connect(m_pageAccueil, &VueAccueil::ouvrirEmployes, this, [this] {
        afficherPage(PageEmployes);
        m_pageEmployes->afficherTousLesEmployes();
    });

    connect(m_pageEmployes, &VueEmployes::ouvrirFicheDetaillee,
            this, &MainWindow::slotOuvrirFiche);
    connect(m_pageEmployes, &VueEmployes::ouvrirHistoriqueEmploye,
            this, [this](int id) {
                slotOuvrirFiche(id);
                m_pageFiche->afficherOngletHistorique();
            });
    connect(m_pageEmployes, &VueEmployes::exporterFicheEmploye,
            this, [this](int id) {
                if (m_pageFiche->chargerEmploye(id)) {
                    m_pageFiche->exporterFichePDF();
                }
            });
    connect(m_pageEmployes, &VueEmployes::donneesModifiees,
            this, &MainWindow::slotDonneesModifiees);

    connect(m_pageFiche, &FicheDetailleeEmploye::retourListe,
            this, &MainWindow::slotRetourEmployes);
    connect(m_pageFiche, &FicheDetailleeEmploye::employeModifie, this, [this] {
        m_pageEmployes->rechargerDonnees();
        rafraichirPagesSecondaires();
    });
    connect(m_pageFiche, &FicheDetailleeEmploye::employeAffiche, this, [this](int id) {
        m_idFicheCourante = id;
    });

    connect(m_pageServices, &VueServices::ouvrirEmployes, this, [this] {
        afficherPage(PageEmployes);
        m_pageEmployes->afficherTousLesEmployes();
    });
    connect(m_pageServices, &VueServices::filtrerEmployesParService,
            this, &MainWindow::slotFiltrerEmployes);

    afficherPage(PageAccueil);
}

MainWindow::~MainWindow()
{
}

QWidget *MainWindow::creerBarreLaterale()
{
    QFrame *barre = new QFrame(this);
    barre->setObjectName("barreLaterale");
    barre->setFixedWidth(220);
    barre->setStyleSheet("QFrame#barreLaterale { background-color: #162836; border: none; }");

    QVBoxLayout *layout = new QVBoxLayout(barre);
    layout->setContentsMargins(16, 24, 16, 18);
    layout->setSpacing(6);

    // Seul le logo reste en haut de la barre latérale.
    QLabel *logo = new QLabel(barre);
    logo->setFixedSize(72, 72);
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("background: transparent; border: none;");
    logo->setPixmap(logoBarreLaterale(58));
    layout->addWidget(logo, 0, Qt::AlignHCenter);
    layout->addSpacing(10);

    QFrame *separateur = new QFrame(barre);
    separateur->setFixedHeight(1);
    separateur->setStyleSheet("background: #203546; border: none;");
    layout->addWidget(separateur);
    layout->addSpacing(12);

    m_btnAccueil = creerBoutonMenu("Accueil", Icones::Maison);
    m_btnEmployes = creerBoutonMenu("Employé", Icones::GroupeUtilisateurs);
    m_btnStation = creerBoutonMenu("Station", Icones::Antenne);
    m_btnEquipement = creerBoutonMenu("Équipement", Icones::Cle);
    m_btnIntervention = creerBoutonMenu("Intervention", Icones::Mallette);

    // Les trois modules sans page dédiée restent visibles, mais sont
    // clairement signalés comme indisponibles plutôt que de simuler un écran.
    const struct {
        QPushButton *bouton;
        const char *module;
        Icones::Type icone;
    } modulesIndisponibles[] = {
        { m_btnStation, "Station", Icones::Antenne },
        { m_btnEquipement, "Équipement", Icones::Cle },
        { m_btnIntervention, "Intervention", Icones::Mallette }
    };
    for (const auto &module : modulesIndisponibles) {
        module.bouton->setEnabled(false);
        module.bouton->setCursor(Qt::ArrowCursor);
        module.bouton->setStyleSheet(STYLE_MENU_INDISPONIBLE);
        module.bouton->setIcon(Icones::icone(module.icone, QColor("#5D7688"), 32));
        module.bouton->setToolTip(
            QString("Aucune page %1 n'existe encore dans ce projet.")
                .arg(QString::fromUtf8(module.module)));
    }

    layout->addWidget(m_btnAccueil);
    layout->addWidget(m_btnEmployes);
    layout->addWidget(m_btnStation);
    layout->addWidget(m_btnEquipement);
    layout->addWidget(m_btnIntervention);
    layout->addStretch();

    QFrame *carteUtilisateur = new QFrame(barre);
    carteUtilisateur->setStyleSheet("background: #203546; border-radius: 6px; border: none;");
    QHBoxLayout *layoutUtilisateur = new QHBoxLayout(carteUtilisateur);
    layoutUtilisateur->setContentsMargins(10, 8, 10, 8);
    layoutUtilisateur->setSpacing(8);

    QLabel *iconeUtilisateur = new QLabel(carteUtilisateur);
    iconeUtilisateur->setPixmap(Icones::dessiner(Icones::Avatar, QColor("#179BAE"), 48)
                                    .scaled(24, 24, Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
    layoutUtilisateur->addWidget(iconeUtilisateur);

    QVBoxLayout *texteUtilisateur = new QVBoxLayout();
    texteUtilisateur->setSpacing(0);
    QLabel *nomUtilisateur = new QLabel("Administrateur", carteUtilisateur);
    nomUtilisateur->setStyleSheet("color: #FFFFFF; font-size: 10px; font-weight: bold; border: none;");
    QLabel *roleUtilisateur = new QLabel("Module RH", carteUtilisateur);
    roleUtilisateur->setStyleSheet("color: #8EA8BE; font-size: 9px; border: none;");
    texteUtilisateur->addWidget(nomUtilisateur);
    texteUtilisateur->addWidget(roleUtilisateur);
    layoutUtilisateur->addLayout(texteUtilisateur, 1);
    layout->addWidget(carteUtilisateur);

    connect(m_btnAccueil, &QPushButton::clicked, this, [this] {
        afficherPage(PageAccueil);
    });
    connect(m_btnEmployes, &QPushButton::clicked, this, [this] {
        afficherPage(PageEmployes);
    });

    return barre;
}

QPushButton *MainWindow::creerBoutonMenu(const QString &texte, Icones::Type iconeType)
{
    QPushButton *bouton = new QPushButton(this);
    bouton->setObjectName("boutonMenu");
    bouton->setText(texte);
    bouton->setIcon(Icones::icone(iconeType, QColor("#8EA8BE"), 32));
    bouton->setIconSize(QSize(20, 20));
    bouton->setCursor(Qt::PointingHandCursor);
    bouton->setFixedHeight(48);
    bouton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bouton->setStyleSheet(STYLE_MENU_INACTIF);
    return bouton;
}

void MainWindow::afficherPage(int page)
{
    if (page < 0 || page >= m_pages->count()) {
        return;
    }

    m_pages->setCurrentIndex(page);

    switch (page) {
    case PageAccueil:
        m_pageAccueil->rechargerDonnees();
        break;
    case PageEmployes:
        m_pageEmployes->rechargerDonnees();
        break;
    case PageServices:
        m_pageServices->rechargerDonnees();
        break;
    case PageRapports:
        m_pageRapports->rechargerDonnees();
        break;
    case PageFiche:
    default:
        break;
    }

    mettreAJourNavigation(page);
}

void MainWindow::mettreAJourNavigation(int page)
{
    if (page == PageFiche) {
        page = PageEmployes;
    }

    auto mettreAJour = [](QPushButton *bouton, Icones::Type iconeType, bool actif) {
        bouton->setStyleSheet(actif ? STYLE_MENU_ACTIF : STYLE_MENU_INACTIF);
        bouton->setIcon(Icones::icone(iconeType,
                                      actif ? QColor(Qt::white) : QColor("#8EA8BE"),
                                      32));
    };

    mettreAJour(m_btnAccueil, Icones::Maison, page == PageAccueil);
    mettreAJour(m_btnEmployes, Icones::GroupeUtilisateurs, page == PageEmployes);
}

void MainWindow::rafraichirPagesSecondaires()
{
    m_pageAccueil->rechargerDonnees();
    m_pageServices->rechargerDonnees();
    m_pageRapports->rechargerDonnees();
}

void MainWindow::slotOuvrirFiche(int id)
{
    if (id <= 0) {
        return;
    }
    if (!m_pageFiche->chargerEmploye(id)) {
        QMessageBox::warning(this, "Fiche employé",
                             "Cet employé n'existe plus dans la base.");
        m_pageEmployes->rechargerDonnees();
        afficherPage(PageEmployes);
        return;
    }

    m_idFicheCourante = id;
    m_pages->setCurrentIndex(PageFiche);
    mettreAJourNavigation(PageEmployes);
}

void MainWindow::slotRetourEmployes()
{
    afficherPage(PageEmployes);
    if (m_idFicheCourante > 0) {
        m_pageEmployes->selectionnerEmploye(m_idFicheCourante);
    }
}

void MainWindow::slotDonneesModifiees()
{
    rafraichirPagesSecondaires();
}

void MainWindow::slotFiltrerEmployes(const QString &service)
{
    afficherPage(PageEmployes);
    if (service.trimmed().isEmpty()) {
        m_pageEmployes->afficherTousLesEmployes();
    } else {
        m_pageEmployes->filtrerParService(service);
    }
}
