#ifndef DIALOGEMPLOYE_H
#define DIALOGEMPLOYE_H

#include "employe.h"

#include <QDialog>
#include <QString>

namespace Ui {
class DialogEmploye;
}

/*
 * Formulaire d'ajout / modification d'un employé.
 * L'identifiant n'est jamais saisi : il est calculé par la base.
 */
class DialogEmploye : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEmploye(QWidget *parent = nullptr);
    ~DialogEmploye() override;

    void modeAjout();
    void chargerEmploye(const Employe &employe);

    Employe employe() const { return m_employe; }
    bool estModification() const { return m_modification; }

private slots:
    void slotChoisirPhoto();
    void slotRetirerPhoto();
    void slotEnregistrer();

private:
    void remplirListes();
    void mettreAJourApercu();
    void appliquerStyle();
    bool verifierSaisie();

    Ui::DialogEmploye *ui;
    Employe m_employe;
    bool m_modification = false;
    bool m_photoModifiee = false;
    QString m_photoSource;
};

#endif // DIALOGEMPLOYE_H
