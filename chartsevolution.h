#ifndef CHARTSEVOLUTION_H
#define CHARTSEVOLUTION_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>

/*
 * Courbe d'évolution sur 12 mois. Le graphique est entièrement dessiné par
 * code, sans données fictives : si aucune valeur n'est fournie, il affiche
 * un message. L'échelle verticale s'adapte aux données.
 */
class ChartsEvolution : public QWidget
{
    Q_OBJECT

public:
    explicit ChartsEvolution(QWidget *parent = nullptr);

    void setDonnees(const QVector<int> &valeurs);
    void setTitre(const QString &titre);
    void setSousTitre(const QString &sousTitre);
    void setUnite(const QString &unite);
    void setCouleur(const QColor &couleur);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int calculerMax(int maximum) const;

    QStringList m_mois;
    QVector<int> m_valeurs;
    int m_maxValeur = 4;
    int m_survolIndex = -1;
    QString m_titre = "Évolution sur 12 mois";
    QString m_sousTitre;
    QString m_unite = "employés";
    QColor m_couleur = QColor("#179BAE");
};

#endif // CHARTSEVOLUTION_H
