#include "modelemployes.h"

#include "outilsemployes.h"

#include <QBrush>
#include <QDate>
#include <QFont>

#include <algorithm>

ModeleEmployes::ModeleEmployes(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ModeleEmployes::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filtres.size();
}

int ModeleEmployes::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NbColonnes;
}

QVariant ModeleEmployes::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtres.size()) {
        return QVariant();
    }

    const Employe &e = m_filtres.at(index.row());
    const int colonne = index.column();

    if (role == Qt::UserRole + 10) {
        return colonne == ColPhoto ? e.photo : QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        switch (colonne) {
        case ColId:
        case ColPhoto:
        case ColDateRecrutement:
        case ColSalaire:
        case ColSolde:
        case ColStatut:
        case ColActions:
            return int(Qt::AlignCenter);
        default:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::ToolTipRole && colonne == ColPhoto) {
        return e.photo.trimmed().isEmpty()
            ? QString()
            : QString("Cliquer pour agrandir la photo");
    }

    if (role == Qt::ToolTipRole
        && colonne != ColPhoto && colonne != ColActions) {
        return data(index, Qt::DisplayRole);
    }

    if (role == Qt::FontRole && colonne == ColNom) {
        QFont police;
        police.setBold(true);
        return police;
    }

    if (role == Qt::ForegroundRole && colonne == ColEmail) {
        return QBrush(QColor("#4A6678"));
    }

    if (role == Qt::DisplayRole) {
        switch (colonne) {
        case ColId:
            return e.id;
        case ColPhoto:
            return QString();
        case ColNom:
            return e.nom;
        case ColPrenom:
            return e.prenom;
        case ColEmail:
            return e.email;
        case ColTelephone:
            return e.telephone;
        case ColPoste:
            return e.poste;
        case ColService:
            return e.service;
        case ColDateRecrutement:
            return dateAffichage(e.dateRecrutement);
        case ColSalaire:
            return salaireTexte(e.salaire);
        case ColSolde:
            return QString("%1 j").arg(e.soldeConges);
        case ColStatut:
            return e.statut;
        case ColActions:
            return QString();
        default:
            break;
        }
    }

    return QVariant();
}

QVariant ModeleEmployes::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Vertical) {
        return section + 1;
    }

    switch (section) {
    case ColId: return "ID";
    case ColPhoto: return "Photo";
    case ColNom: return "Nom";
    case ColPrenom: return "Prénom";
    case ColEmail: return "E-mail";
    case ColTelephone: return "Téléphone";
    case ColPoste: return "Poste";
    case ColService: return "Service";
    case ColDateRecrutement: return "Recrutement";
    case ColSalaire: return "Salaire";
    case ColSolde: return "Solde";
    case ColStatut: return "Statut";
    case ColActions: return "Actions";
    default: return QVariant();
    }
}

void ModeleEmployes::setEmployes(const QVector<Employe> &employes)
{
    m_tous = employes;
    recalculer();
}

void ModeleEmployes::setRecherche(const QString &recherche)
{
    if (m_recherche == recherche) {
        return;
    }
    m_recherche = recherche;
    recalculer();
}

void ModeleEmployes::setService(const QString &service)
{
    if (m_service == service) {
        return;
    }
    m_service = service;
    recalculer();
}

void ModeleEmployes::setStatut(const QString &statut)
{
    if (m_statut == statut) {
        return;
    }
    m_statut = statut;
    recalculer();
}

void ModeleEmployes::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= NbColonnes || column == ColActions) {
        return;
    }
    m_colonneTri = column;
    m_ordreTri = order;
    recalculer();
}

const Employe &ModeleEmployes::employe(int ligne) const
{
    static const Employe vide;
    if (ligne < 0 || ligne >= m_filtres.size()) {
        return vide;
    }
    return m_filtres.at(ligne);
}

int ModeleEmployes::idLigne(int ligne) const
{
    return employe(ligne).id;
}

int ModeleEmployes::lignePourId(int id) const
{
    for (int i = 0; i < m_filtres.size(); ++i) {
        if (m_filtres.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

void ModeleEmployes::recalculer()
{
    beginResetModel();

    const QString recherche = m_recherche.trimmed();
    m_filtres.clear();
    m_filtres.reserve(m_tous.size());

    for (const Employe &e : m_tous) {
        if (!m_service.trimmed().isEmpty() &&
            e.service.compare(m_service, Qt::CaseInsensitive) != 0) {
            continue;
        }
        if (!m_statut.trimmed().isEmpty() &&
            e.statut.compare(m_statut, Qt::CaseInsensitive) != 0) {
            continue;
        }
        if (!recherche.isEmpty()) {
            const QStringList champs = {
                QString::number(e.id), e.nom, e.prenom, e.email, e.telephone,
                e.poste, e.service
            };
            bool trouve = false;
            for (const QString &champ : champs) {
                if (champ.contains(recherche, Qt::CaseInsensitive)) {
                    trouve = true;
                    break;
                }
            }
            if (!trouve) {
                continue;
            }
        }
        m_filtres.append(e);
    }

    const int colonne = m_colonneTri;
    const bool ascendant = m_ordreTri == Qt::AscendingOrder;

    std::stable_sort(m_filtres.begin(), m_filtres.end(),
        [colonne, ascendant](const Employe &a, const Employe &b) {
            int comparaison = 0;
            switch (colonne) {
            case ColId:
                comparaison = a.id - b.id;
                break;
            case ColPhoto:
                comparaison = a.id - b.id;
                break;
            case ColNom:
                comparaison = a.nom.localeAwareCompare(b.nom);
                break;
            case ColPrenom:
                comparaison = a.prenom.localeAwareCompare(b.prenom);
                break;
            case ColEmail:
                comparaison = a.email.localeAwareCompare(b.email);
                break;
            case ColTelephone:
                comparaison = a.telephone.localeAwareCompare(b.telephone);
                break;
            case ColPoste:
                comparaison = a.poste.localeAwareCompare(b.poste);
                break;
            case ColService:
                comparaison = a.service.localeAwareCompare(b.service);
                break;
            case ColDateRecrutement: {
                const QDate da = dateDepuisTexte(a.dateRecrutement);
                const QDate db = dateDepuisTexte(b.dateRecrutement);
                comparaison = da == db ? 0 : (da < db ? -1 : 1);
                break;
            }
            case ColSalaire:
                comparaison = a.salaire < b.salaire ? -1 : (a.salaire > b.salaire ? 1 : 0);
                break;
            case ColSolde:
                comparaison = a.soldeConges - b.soldeConges;
                break;
            case ColStatut:
                comparaison = a.statut.localeAwareCompare(b.statut);
                break;
            default:
                comparaison = a.id - b.id;
                break;
            }

            if (!ascendant) {
                comparaison = -comparaison;
            }
            if (comparaison == 0) {
                comparaison = a.id - b.id;
            }
            return comparaison < 0;
        });

    endResetModel();
}
