#include "kpicard.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

KpiCard::KpiCard(const QString &titre, const QString &valeur, const QString &evolution,
                 Icones::Type typeIcone, StyleVariant style, QWidget *parent)
    : QWidget(parent),
      m_titre(titre),
      m_valeur(valeur),
      m_evolution(evolution),
      m_typeIcone(typeIcone),
      m_style(style)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(88);
    setMaximumHeight(100);

    switch (m_style) {
    case NavyTeal:
        m_couleurFond = QColor("#183B4E");
        m_couleurCercleIcone = QColor(255, 255, 255, 30);
        m_couleurIcone = QColor("#8EE0E8");
        break;
    case Emerald:
        m_couleurFond = QColor("#124E50");
        m_couleurCercleIcone = QColor(255, 255, 255, 30);
        m_couleurIcone = QColor("#79E2A5");
        break;
    case OceanBlue:
        m_couleurFond = QColor("#163F61");
        m_couleurCercleIcone = QColor(255, 255, 255, 30);
        m_couleurIcone = QColor("#7DBFF5");
        break;
    case SlateDark:
        m_couleurFond = QColor("#283C4F");
        m_couleurCercleIcone = QColor(255, 255, 255, 30);
        m_couleurIcone = QColor("#CBD5E1");
        break;
    }
}

void KpiCard::setValeur(const QString &valeur)
{
    m_valeur = valeur;
    update();
}

void KpiCard::setEvolution(const QString &evolution)
{
    m_evolution = evolution;
    update();
}

void KpiCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = rect().adjusted(0, 0, -1, -1);

    // Fond avec coins arrondis
    p.setPen(Qt::NoPen);
    p.setBrush(m_couleurFond);
    p.drawRoundedRect(r, 8, 8);

    // Cercle d'icône à gauche
    const int diametre = 44;
    const int margeGauche = 16;
    const int yCentre = r.center().y();
    const QRectF cercleRect(margeGauche, yCentre - diametre / 2, diametre, diametre);

    p.setBrush(m_couleurCercleIcone);
    p.drawEllipse(cercleRect);

    // Dessin de l'icône
    QPixmap iconePix = Icones::dessiner(m_typeIcone, m_couleurIcone, 26);
    p.drawPixmap(cercleRect.center().x() - 13, cercleRect.center().y() - 13, iconePix);

    // Textes à droite du cercle
    const int xTexte = margeGauche + diametre + 14;

    // Titre
    QFont fontTitre = font();
    fontTitre.setPointSize(9);
    fontTitre.setBold(false);
    p.setFont(fontTitre);
    p.setPen(QColor("#A9C1D6"));
    p.drawText(QRect(xTexte, yCentre - 34, r.width() - xTexte - 10, 18),
               Qt::AlignLeft | Qt::AlignVCenter, m_titre);

    // Grande valeur
    QFont fontValeur = font();
    fontValeur.setPointSize(16);
    fontValeur.setBold(true);
    p.setFont(fontValeur);
    p.setPen(Qt::white);
    p.drawText(QRect(xTexte, yCentre - 14, r.width() - xTexte - 10, 26),
               Qt::AlignLeft | Qt::AlignVCenter, m_valeur);

    // Évolution / sous-titre
    QFont fontEvol = font();
    fontEvol.setPointSize(8);
    fontEvol.setBold(false);
    p.setFont(fontEvol);
    p.setPen(QColor("#7FA3C0"));
    p.drawText(QRect(xTexte, yCentre + 12, r.width() - xTexte - 10, 16),
               Qt::AlignLeft | Qt::AlignVCenter, m_evolution);
}
