#ifndef VUERAPPORTS_H
#define VUERAPPORTS_H

#include <QStringList>
#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class KpiCard;
class ChartsBarres;

/*
 * Page « Rapports » : synthèse réelle des effectifs, de la masse salariale,
 * des services et des congés. Les chiffres proviennent uniquement de la base.
 */
class VueRapports : public QWidget
{
    Q_OBJECT

public:
    explicit VueRapports(QWidget *parent = nullptr);

    void rechargerDonnees();

    /* Écrit la synthèse courante dans un fichier CSV (UTF-8 avec BOM). */
    bool exporterCSV(const QString &cheminFichier);

private slots:
    void slotExporterCSV();

private:
    void creerInterface();

    KpiCard *m_kpiMasse = nullptr;
    KpiCard *m_kpiMoyenne = nullptr;
    KpiCard *m_kpiJours = nullptr;
    KpiCard *m_kpiAttente = nullptr;

    ChartsBarres *m_chartServices = nullptr;
    ChartsBarres *m_chartTypes = nullptr;

    QTableWidget *m_tableServices = nullptr;
    QLabel *m_videServices = nullptr;

    QStringList m_lignesCsv;
};

#endif // VUERAPPORTS_H
