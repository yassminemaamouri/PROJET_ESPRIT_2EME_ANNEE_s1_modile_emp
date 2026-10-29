#include "chartsbarres.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

ChartsBarres::ChartsBarres(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ChartsBarres::setDonnees(const QVector<QPair<QString, int>> &donnees)
{
    m_donnees = donnees;
    update();
}

void ChartsBarres::setTitre(const QString &titre)
{
    m_titre = titre;
    update();
}

void ChartsBarres::setSousTitre(const QString &sousTitre)
{
    m_sousTitre = sousTitre;
    update();
}

void ChartsBarres::setCouleur(const QColor &couleur)
{
    m_couleur = couleur;
    update();
}

void ChartsBarres::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor("#E1E8EE"), 1));
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

    QFont titrePolice = font();
    titrePolice.setPointSize(10);
    titrePolice.setBold(true);
    painter.setFont(titrePolice);
    painter.setPen(QColor("#1B3346"));
    painter.drawText(QRect(20, 14, width() - 40, 22),
                     Qt::AlignLeft | Qt::AlignVCenter, m_titre);

    QFont sousTitrePolice = font();
    sousTitrePolice.setPointSize(8);
    painter.setFont(sousTitrePolice);
    painter.setPen(QColor("#7A8B99"));
    painter.drawText(QRect(20, 36, width() - 40, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     m_sousTitre.isEmpty() ? QString("Valeurs actuelles") : m_sousTitre);

    if (m_donnees.isEmpty()) {
        painter.setPen(QColor("#8A9BA8"));
        painter.drawText(rect().adjusted(20, 60, -20, -20),
                         Qt::AlignCenter, "Aucune donnée disponible");
        return;
    }

    int maximum = 1;
    for (const auto &donnee : m_donnees) {
        maximum = qMax(maximum, donnee.second);
    }

    const int gauche = 20;
    const int haut = 64;
    const int bas = 16;
    const int etiquetteLargeur = qMin(170, width() / 3);
    const int zoneGauche = gauche + etiquetteLargeur + 12;
    const int zoneLargeur = width() - zoneGauche - 54;
    const int hauteurDisponible = height() - haut - bas;
    if (zoneLargeur <= 20 || hauteurDisponible <= 20) {
        return;
    }

    const int nb = m_donnees.size();
    const int hauteurLigne = qMin(42, hauteurDisponible / nb);
    const int hauteurBarre = qMax(10, qMin(18, hauteurLigne - 12));
    int y = haut;

    QFont etiquettePolice = font();
    etiquettePolice.setPointSize(8);
    const QFontMetrics metrics(etiquettePolice);
    painter.setFont(etiquettePolice);

    const QVector<QColor> palette = {
        m_couleur,
        QColor("#137A6E"),
        QColor("#2E6F9E"),
        QColor("#7A5AA6"),
        QColor("#C47A24"),
        QColor("#1B3346")
    };

    for (int i = 0; i < m_donnees.size(); ++i) {
        const QString label = m_donnees.at(i).first;
        const int valeur = m_donnees.at(i).second;
        const QColor couleur = palette.at(i % palette.size());

        painter.setPen(QColor("#1B3346"));
        painter.drawText(QRect(gauche, y + (hauteurLigne - 18) / 2,
                               etiquetteLargeur, 18),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         metrics.elidedText(label, Qt::ElideRight, etiquetteLargeur));

        const QRect piste(zoneGauche, y + (hauteurLigne - hauteurBarre) / 2,
                          zoneLargeur, hauteurBarre);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#EDF2F6"));
        painter.drawRoundedRect(piste, hauteurBarre / 2.0, hauteurBarre / 2.0);

        const int largeurBarre = maximum > 0
            ? qMax(4, qRound(piste.width() * valeur / double(maximum)))
            : 0;
        if (largeurBarre > 0) {
            painter.setBrush(couleur);
            painter.drawRoundedRect(QRect(piste.left(), piste.top(),
                                          largeurBarre, piste.height()),
                                    hauteurBarre / 2.0, hauteurBarre / 2.0);
        }

        painter.setPen(QColor("#4A6678"));
        painter.drawText(QRect(piste.right() + 8, piste.top(), 46, piste.height()),
                         Qt::AlignLeft | Qt::AlignVCenter, QString::number(valeur));

        y += hauteurLigne;
    }
}
