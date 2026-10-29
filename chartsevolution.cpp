#include "chartsevolution.h"

#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

ChartsEvolution::ChartsEvolution(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mois << "Jan" << "Fév" << "Mar" << "Avr" << "Mai" << "Juin"
           << "Juil" << "Août" << "Sep" << "Oct" << "Nov" << "Déc";
}

void ChartsEvolution::setDonnees(const QVector<int> &valeurs)
{
    if (valeurs.isEmpty()) {
        m_valeurs.clear();
    } else {
        m_valeurs = valeurs;
        while (m_valeurs.size() < 12) {
            m_valeurs.append(0);
        }
        if (m_valeurs.size() > 12) {
            m_valeurs = m_valeurs.mid(0, 12);
        }
    }

    int maximum = 0;
    for (int valeur : m_valeurs) {
        maximum = qMax(maximum, valeur);
    }
    m_maxValeur = calculerMax(maximum);
    m_survolIndex = -1;
    update();
}

void ChartsEvolution::setTitre(const QString &titre)
{
    m_titre = titre;
    update();
}

void ChartsEvolution::setSousTitre(const QString &sousTitre)
{
    m_sousTitre = sousTitre;
    update();
}

void ChartsEvolution::setUnite(const QString &unite)
{
    m_unite = unite;
    update();
}

void ChartsEvolution::setCouleur(const QColor &couleur)
{
    m_couleur = couleur;
    update();
}

int ChartsEvolution::calculerMax(int maximum) const
{
    if (maximum <= 0) {
        return 4;
    }
    const int divisions = 4;
    int pas = qMax(1, int(std::ceil(maximum / double(divisions))));

    const double magnitude = std::pow(10.0, std::floor(std::log10(double(pas))));
    const int normalise = qMax(1, int(std::round(pas / magnitude)));
    if (normalise <= 1) {
        pas = int(magnitude);
    } else if (normalise <= 2) {
        pas = int(2 * magnitude);
    } else if (normalise <= 5) {
        pas = int(5 * magnitude);
    } else {
        pas = int(10 * magnitude);
    }
    return qMax(1, pas) * divisions;
}

void ChartsEvolution::paintEvent(QPaintEvent *event)
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
    const QString sousTitre = m_sousTitre.isEmpty()
        ? QString("12 mois — %1").arg(m_unite)
        : m_sousTitre;
    painter.drawText(QRect(20, 36, width() - 40, 18), Qt::AlignLeft | Qt::AlignVCenter, sousTitre);

    if (m_valeurs.isEmpty()) {
        painter.setPen(QColor("#8A9BA8"));
        painter.drawText(rect(), Qt::AlignCenter, "Aucune donnée disponible");
        return;
    }

    const int gauche = 48;
    const int droite = 20;
    const int haut = 64;
    const int bas = 32;
    const int largeurZone = width() - gauche - droite;
    const int hauteurZone = height() - haut - bas;
    if (largeurZone <= 20 || hauteurZone <= 20) {
        return;
    }

    const QRect zone(gauche, haut, largeurZone, hauteurZone);
    const int nbPoints = m_valeurs.size();

    QFont axePolice = font();
    axePolice.setPointSize(7);
    painter.setFont(axePolice);

    // Grille horizontale et valeurs de l'axe vertical.
    for (int i = 0; i <= 4; ++i) {
        const int y = zone.bottom() - (zone.height() * i / 4);
        painter.setPen(QPen(QColor(i == 0 ? "#D5E0E8" : "#EDF2F6"), 1));
        painter.drawLine(zone.left(), y, zone.right(), y);

        painter.setPen(QColor("#7A8B99"));
        const int valeur = m_maxValeur * i / 4;
        painter.drawText(QRect(0, y - 9, gauche - 8, 18),
                         Qt::AlignRight | Qt::AlignVCenter, QString::number(valeur));
    }

    // Étiquettes des mois.
    for (int i = 0; i < nbPoints; ++i) {
        const double x = nbPoints > 1
            ? zone.left() + (zone.width() * i / double(nbPoints - 1))
            : zone.center().x();
        const QRect etiquette(int(x) - 22, zone.bottom() + 7, 44, 16);
        painter.setPen(m_survolIndex == i ? QColor("#179BAE") : QColor("#7A8B99"));
        painter.drawText(etiquette, Qt::AlignCenter, m_mois.value(i));
    }

    QVector<QPointF> points;
    points.reserve(nbPoints);
    for (int i = 0; i < nbPoints; ++i) {
        const double x = nbPoints > 1
            ? zone.left() + (zone.width() * i / double(nbPoints - 1))
            : zone.center().x();
        const double y = zone.bottom()
            - (qBound(0, m_valeurs.at(i), m_maxValeur) / double(m_maxValeur)) * zone.height();
        points.append(QPointF(x, y));
    }

    QPainterPath ligne;
    ligne.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i) {
        const QPointF precedent = points.at(i - 1);
        const QPointF courant = points.at(i);
        const double milieu = (precedent.x() + courant.x()) / 2.0;
        ligne.cubicTo(QPointF(milieu, precedent.y()),
                      QPointF(milieu, courant.y()),
                      courant);
    }

    QPainterPath aire = ligne;
    aire.lineTo(points.last().x(), zone.bottom());
    aire.lineTo(points.first().x(), zone.bottom());
    aire.closeSubpath();

    QLinearGradient degrade(0, zone.top(), 0, zone.bottom());
    QColor couleurHaute = m_couleur;
    couleurHaute.setAlpha(70);
    QColor couleurBasse = m_couleur;
    couleurBasse.setAlpha(0);
    degrade.setColorAt(0.0, couleurHaute);
    degrade.setColorAt(1.0, couleurBasse);

    painter.save();
    painter.setClipRect(zone.adjusted(-1, -1, 1, 1));
    painter.setPen(Qt::NoPen);
    painter.setBrush(degrade);
    painter.drawPath(aire);
    painter.restore();

    painter.setPen(QPen(m_couleur, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(ligne);

    for (int i = 0; i < points.size(); ++i) {
        const bool survole = (i == m_survolIndex);
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(survole ? m_couleur : Qt::white);
        painter.drawEllipse(points.at(i), survole ? 5 : 3.5, survole ? 5 : 3.5);
        if (!survole) {
            painter.setBrush(m_couleur);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(points.at(i), 2, 2);
        }
    }

    if (m_survolIndex >= 0 && m_survolIndex < points.size()) {
        const QPointF point = points.at(m_survolIndex);
        painter.setPen(QPen(QColor("#A8BCC9"), 1, Qt::DashLine));
        painter.drawLine(QPointF(point.x(), zone.top()), QPointF(point.x(), zone.bottom()));

        const QString texte = QString("%1 : %2 %3")
            .arg(m_mois.value(m_survolIndex))
            .arg(m_valeurs.at(m_survolIndex))
            .arg(m_unite);
        QFont infoPolice = font();
        infoPolice.setPointSize(8);
        infoPolice.setBold(true);
        painter.setFont(infoPolice);
        const QFontMetrics metrics(infoPolice);
        const int largeurInfo = metrics.horizontalAdvance(texte) + 20;
        int xInfo = int(point.x()) - largeurInfo / 2;
        xInfo = qBound(zone.left(), xInfo, zone.right() - largeurInfo);
        int yInfo = int(point.y()) - 38;
        if (yInfo < zone.top()) {
            yInfo = int(point.y()) + 12;
        }
        const QRect info(xInfo, yInfo, largeurInfo, 26);
        painter.setPen(QPen(QColor("#D5E0E8"), 1));
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(info, 6, 6);
        painter.setPen(QColor("#1B3346"));
        painter.drawText(info, Qt::AlignCenter, texte);
    }
}

void ChartsEvolution::mouseMoveEvent(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint position = event->position().toPoint();
#else
    const QPoint position = event->pos();
#endif

    const int gauche = 48;
    const int droite = 20;
    const int haut = 64;
    const int bas = 32;
    const QRect zone(gauche, haut, width() - gauche - droite, height() - haut - bas);

    int index = -1;
    if (m_valeurs.size() > 1 && zone.contains(position)) {
        const double ratio = (position.x() - zone.left()) / double(zone.width());
        index = qBound(0, int(std::round(ratio * (m_valeurs.size() - 1))),
                       m_valeurs.size() - 1);
    }
    if (index != m_survolIndex) {
        m_survolIndex = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void ChartsEvolution::leaveEvent(QEvent *event)
{
    if (m_survolIndex != -1) {
        m_survolIndex = -1;
        update();
    }
    QWidget::leaveEvent(event);
}
