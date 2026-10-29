#ifndef DIALOGCONGE_H
#define DIALOGCONGE_H

#include "conge.h"

#include <QDialog>
#include <QString>

class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QTextEdit;

/*
 * Boîte de dialogue de création d'une demande de congé.
 * Le contrôle du solde et des chevauchements est fait par Conge::ajouter().
 */
class DialogConge : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConge(int employeId, const QString &nomEmploye,
                         QWidget *parent = nullptr);

    Conge congeCree() const { return m_conge; }

private slots:
    void actualiserJours();
    void envoyerDemande();

private:
    void appliquerStyle();
    void mettreAJourSolde();

    int m_employeId = 0;
    QString m_nomEmploye;
    int m_droit = 30;
    int m_utilises = 0;
    Conge m_conge;

    QComboBox *m_type = nullptr;
    QDateEdit *m_debut = nullptr;
    QDateEdit *m_fin = nullptr;
    QLabel *m_jours = nullptr;
    QLabel *m_solde = nullptr;
    QTextEdit *m_motif = nullptr;
    QLabel *m_erreur = nullptr;
};

#endif // DIALOGCONGE_H
