#ifndef MODELEEMPLOYES_H
#define MODELEEMPLOYES_H

#include "employe.h"

#include <QAbstractTableModel>
#include <QVector>

/*
 * Modèle du tableau des employés : recherche, filtres service/statut et tri.
 * Les données restent dans un QVector ; la base n'est interrogée qu'au
 * rechargement, ce qui rend le tri instantané.
 */
class ModeleEmployes : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Colonne {
        ColId = 0,
        ColPhoto,
        ColNom,
        ColPrenom,
        // Statut et actions restent visibles sans défilement horizontal.
        ColStatut,
        ColActions,
        ColEmail,
        ColTelephone,
        ColPoste,
        ColService,
        ColDateRecrutement,
        ColSalaire,
        ColSolde,
        NbColonnes
    };

    explicit ModeleEmployes(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setEmployes(const QVector<Employe> &employes);
    void setRecherche(const QString &recherche);
    void setService(const QString &service);
    void setStatut(const QString &statut);

    int nombreTotal() const { return m_tous.size(); }
    int nombreFiltre() const { return m_filtres.size(); }
    const QVector<Employe> &employes() const { return m_tous; }
    const QVector<Employe> &employesFiltres() const { return m_filtres; }
    const Employe &employe(int ligne) const;
    int idLigne(int ligne) const;
    int lignePourId(int id) const;

private:
    void recalculer();

    QVector<Employe> m_tous;
    QVector<Employe> m_filtres;
    QString m_recherche;
    QString m_service;
    QString m_statut;
    int m_colonneTri = ColId;
    Qt::SortOrder m_ordreTri = Qt::AscendingOrder;
};

#endif // MODELEEMPLOYES_H
