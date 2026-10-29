#ifndef KPICARD_H
#define KPICARD_H

#include "icones.h"

#include <QColor>
#include <QString>
#include <QWidget>

class KpiCard : public QWidget
{
    Q_OBJECT

public:
    enum StyleVariant {
        NavyTeal,    // Effectif total (#183B4E)
        Emerald,     // Nouveaux (#124E50)
        OceanBlue,   // Actifs (#163F61)
        SlateDark    // Inactifs (#253B4E)
    };

    explicit KpiCard(const QString &titre, const QString &valeur, const QString &evolution,
                     Icones::Type typeIcone, StyleVariant style = NavyTeal,
                     QWidget *parent = nullptr);

    void setValeur(const QString &valeur);
    void setEvolution(const QString &evolution);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_titre;
    QString m_valeur;
    QString m_evolution;
    Icones::Type m_typeIcone;
    StyleVariant m_style;
    QColor m_couleurFond;
    QColor m_couleurCercleIcone;
    QColor m_couleurIcone;
};

#endif // KPICARD_H
