#ifndef CHARTSBARRES_H
#define CHARTSBARRES_H

#include <QColor>
#include <QPair>
#include <QString>
#include <QVector>
#include <QWidget>

/*
 * Graphique à barres horizontales, utilisé pour la répartition par service
 * et par poste. Aucune donnée par défaut.
 */
class ChartsBarres : public QWidget
{
    Q_OBJECT

public:
    explicit ChartsBarres(QWidget *parent = nullptr);

    void setDonnees(const QVector<QPair<QString, int>> &donnees);
    void setTitre(const QString &titre);
    void setSousTitre(const QString &sousTitre);
    void setCouleur(const QColor &couleur);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString, int>> m_donnees;
    QString m_titre = "Répartition";
    QString m_sousTitre;
    QColor m_couleur = QColor("#179BAE");
};

#endif // CHARTSBARRES_H
