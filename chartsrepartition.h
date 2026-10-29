#ifndef CHARTSREPARTITION_H
#define CHARTSREPARTITION_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QWidget>

struct SegmentDonut
{
    QString label;
    int valeur = 0;
    int pourcentage = 0;
    QColor couleur;
};

/*
 * Donut de répartition (statuts, types de congés, etc.).
 * Les pourcentages sont calculés à partir des valeurs réelles.
 */
class ChartsRepartition : public QWidget
{
    Q_OBJECT

public:
    explicit ChartsRepartition(QWidget *parent = nullptr);

    void setDonnees(const QVector<SegmentDonut> &segments);
    void setTitre(const QString &titre);
    void setSousTitre(const QString &sousTitre);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<SegmentDonut> m_segments;
    int m_total = 0;
    QString m_titre = "Répartition";
    QString m_sousTitre;
};

#endif // CHARTSREPARTITION_H
