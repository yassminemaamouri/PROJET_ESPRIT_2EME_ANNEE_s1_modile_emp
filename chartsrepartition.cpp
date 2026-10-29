#include "chartsrepartition.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

ChartsRepartition::ChartsRepartition(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ChartsRepartition::setDonnees(const QVector<SegmentDonut> &segments)
{
    m_segments = segments;
    m_total = 0;
    for (SegmentDonut &segment : m_segments) {
        m_total += qMax(0, segment.valeur);
        segment.pourcentage = 0;
    }

    if (m_total > 0) {
        int cumul = 0;
        for (int i = 0; i < m_segments.size(); ++i) {
            if (i == m_segments.size() - 1) {
                m_segments[i].pourcentage = qMax(0, 100 - cumul);
            } else {
                m_segments[i].pourcentage = qRound(
                    m_segments.at(i).valeur * 100.0 / m_total);
                cumul += m_segments.at(i).pourcentage;
            }
        }
    }
    update();
}

void ChartsRepartition::setTitre(const QString &titre)
{
    m_titre = titre;
    update();
}

void ChartsRepartition::setSousTitre(const QString &sousTitre)
{
    m_sousTitre = sousTitre;
    update();
}

void ChartsRepartition::paintEvent(QPaintEvent *event)
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
    painter.drawText(QRect(20, 14, width() - 40, 22), Qt::AlignLeft | Qt::AlignVCenter, m_titre);

    QFont sousTitrePolice = font();
    sousTitrePolice.setPointSize(8);
    painter.setFont(sousTitrePolice);
    painter.setPen(QColor("#7A8B99"));
    painter.drawText(QRect(20, 36, width() - 40, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     m_sousTitre.isEmpty() ? QString("Répartition actuelle") : m_sousTitre);

    const int taille = qMin(156, qMin(width() / 2 - 24, height() - 88));
    if (taille <= 20) {
        return;
    }
    const QRectF cercle(28, 58, taille, taille);

    QPen fond(QColor("#EDF2F6"), 20);
    fond.setCapStyle(Qt::FlatCap);
    painter.setPen(fond);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(cercle, 0, 360 * 16);

    if (m_total > 0) {
        int angle = 90 * 16;
        for (const SegmentDonut &segment : m_segments) {
            if (segment.valeur <= 0) {
                continue;
            }
            const int span = -qRound(segment.valeur * 360.0 * 16 / m_total);
            QPen pen(segment.couleur, 20);
            pen.setCapStyle(Qt::FlatCap);
            painter.setPen(pen);
            painter.drawArc(cercle, angle, span);
            angle += span;
        }
    }

    QFont totalPolice = font();
    totalPolice.setPointSize(20);
    totalPolice.setBold(true);
    painter.setFont(totalPolice);
    painter.setPen(QColor("#183B4E"));
    painter.drawText(cercle, Qt::AlignCenter, QString::number(m_total));

    QFont legendePolice = font();
    legendePolice.setPointSize(8);
    painter.setFont(legendePolice);
    painter.setPen(QColor("#7A8B99"));
    painter.drawText(QRectF(cercle.left(), cercle.bottom() + 1, cercle.width(), 16),
                     Qt::AlignCenter, m_total == 1 ? "employé" : "employés");

    const int xLegende = int(cercle.right()) + 26;
    const int largeurLegende = width() - xLegende - 20;
    if (largeurLegende < 80) {
        return;
    }

    int y = 66;
    const int hauteurLigne = 34;
    for (const SegmentDonut &segment : m_segments) {
        if (y + hauteurLigne > height() - 10) {
            break;
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(segment.couleur);
        painter.drawEllipse(QRectF(xLegende, y + 6, 10, 10));

        QFont labelPolice = font();
        labelPolice.setPointSize(8);
        labelPolice.setBold(true);
        painter.setFont(labelPolice);
        painter.setPen(QColor("#1B3346"));
        painter.drawText(QRect(xLegende + 18, y, largeurLegende - 18, 18),
                         Qt::AlignLeft | Qt::AlignVCenter, segment.label);

        QFont valeurPolice = font();
        valeurPolice.setPointSize(8);
        painter.setFont(valeurPolice);
        painter.setPen(QColor("#7A8B99"));
        painter.drawText(QRect(xLegende + 18, y + 17, largeurLegende - 18, 16),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString("%1 — %2 %")
                             .arg(segment.valeur)
                             .arg(segment.pourcentage));
        y += hauteurLigne;
    }

    if (m_total == 0) {
        painter.setPen(QColor("#8A9BA8"));
        painter.drawText(QRect(xLegende, 66, largeurLegende, 40),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         "Aucune donnée disponible");
    }
}
