/********************************************************************************
** Form generated from reading UI file 'dialogemploye.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGEMPLOYE_H
#define UI_DIALOGEMPLOYE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogEmploye
{
public:
    QVBoxLayout *layoutPrincipal;
    QLabel *titreLabel;
    QLabel *sousTitreLabel;
    QHBoxLayout *ligneContenu;
    QGridLayout *formLayout;
    QLabel *labelId;
    QLineEdit *champId;
    QLabel *labelNom;
    QLineEdit *champNom;
    QLabel *labelPrenom;
    QLineEdit *champPrenom;
    QLabel *labelEmail;
    QLineEdit *champEmail;
    QLabel *labelTelephone;
    QLineEdit *champTelephone;
    QLabel *labelPoste;
    QComboBox *champPoste;
    QLabel *labelService;
    QComboBox *champService;
    QLabel *labelDate;
    QDateEdit *champDate;
    QLabel *labelSalaire;
    QDoubleSpinBox *champSalaire;
    QLabel *labelSolde;
    QSpinBox *champSolde;
    QLabel *labelStatut;
    QComboBox *champStatut;
    QLabel *labelMotDePasse;
    QLineEdit *champMotDePasse;
    QVBoxLayout *colonnePhoto;
    QLabel *apercuPhoto;
    QPushButton *btnChoisirPhoto;
    QPushButton *btnRetirerPhoto;
    QLabel *aidePhoto;
    QSpacerItem *spacerPhoto;
    QLabel *labelObligatoire;
    QHBoxLayout *ligneBoutons;
    QSpacerItem *spacerBoutons;
    QPushButton *btnAnnuler;
    QPushButton *btnEnregistrer;

    void setupUi(QDialog *DialogEmploye)
    {
        if (DialogEmploye->objectName().isEmpty())
            DialogEmploye->setObjectName("DialogEmploye");
        DialogEmploye->resize(880, 650);
        layoutPrincipal = new QVBoxLayout(DialogEmploye);
        layoutPrincipal->setSpacing(12);
        layoutPrincipal->setObjectName("layoutPrincipal");
        layoutPrincipal->setContentsMargins(24, 22, 24, 20);
        titreLabel = new QLabel(DialogEmploye);
        titreLabel->setObjectName("titreLabel");

        layoutPrincipal->addWidget(titreLabel);

        sousTitreLabel = new QLabel(DialogEmploye);
        sousTitreLabel->setObjectName("sousTitreLabel");

        layoutPrincipal->addWidget(sousTitreLabel);

        ligneContenu = new QHBoxLayout();
        ligneContenu->setSpacing(24);
        ligneContenu->setObjectName("ligneContenu");
        formLayout = new QGridLayout();
        formLayout->setObjectName("formLayout");
        formLayout->setHorizontalSpacing(14);
        formLayout->setVerticalSpacing(8);
        labelId = new QLabel(DialogEmploye);
        labelId->setObjectName("labelId");

        formLayout->addWidget(labelId, 0, 0, 1, 1);

        champId = new QLineEdit(DialogEmploye);
        champId->setObjectName("champId");
        champId->setReadOnly(true);

        formLayout->addWidget(champId, 0, 1, 1, 1);

        labelNom = new QLabel(DialogEmploye);
        labelNom->setObjectName("labelNom");

        formLayout->addWidget(labelNom, 1, 0, 1, 1);

        champNom = new QLineEdit(DialogEmploye);
        champNom->setObjectName("champNom");

        formLayout->addWidget(champNom, 1, 1, 1, 1);

        labelPrenom = new QLabel(DialogEmploye);
        labelPrenom->setObjectName("labelPrenom");

        formLayout->addWidget(labelPrenom, 2, 0, 1, 1);

        champPrenom = new QLineEdit(DialogEmploye);
        champPrenom->setObjectName("champPrenom");

        formLayout->addWidget(champPrenom, 2, 1, 1, 1);

        labelEmail = new QLabel(DialogEmploye);
        labelEmail->setObjectName("labelEmail");

        formLayout->addWidget(labelEmail, 3, 0, 1, 1);

        champEmail = new QLineEdit(DialogEmploye);
        champEmail->setObjectName("champEmail");

        formLayout->addWidget(champEmail, 3, 1, 1, 1);

        labelTelephone = new QLabel(DialogEmploye);
        labelTelephone->setObjectName("labelTelephone");

        formLayout->addWidget(labelTelephone, 4, 0, 1, 1);

        champTelephone = new QLineEdit(DialogEmploye);
        champTelephone->setObjectName("champTelephone");

        formLayout->addWidget(champTelephone, 4, 1, 1, 1);

        labelPoste = new QLabel(DialogEmploye);
        labelPoste->setObjectName("labelPoste");

        formLayout->addWidget(labelPoste, 5, 0, 1, 1);

        champPoste = new QComboBox(DialogEmploye);
        champPoste->setObjectName("champPoste");
        champPoste->setEditable(true);

        formLayout->addWidget(champPoste, 5, 1, 1, 1);

        labelService = new QLabel(DialogEmploye);
        labelService->setObjectName("labelService");

        formLayout->addWidget(labelService, 6, 0, 1, 1);

        champService = new QComboBox(DialogEmploye);
        champService->setObjectName("champService");
        champService->setEditable(true);

        formLayout->addWidget(champService, 6, 1, 1, 1);

        labelDate = new QLabel(DialogEmploye);
        labelDate->setObjectName("labelDate");

        formLayout->addWidget(labelDate, 7, 0, 1, 1);

        champDate = new QDateEdit(DialogEmploye);
        champDate->setObjectName("champDate");
        champDate->setCalendarPopup(true);

        formLayout->addWidget(champDate, 7, 1, 1, 1);

        labelSalaire = new QLabel(DialogEmploye);
        labelSalaire->setObjectName("labelSalaire");

        formLayout->addWidget(labelSalaire, 8, 0, 1, 1);

        champSalaire = new QDoubleSpinBox(DialogEmploye);
        champSalaire->setObjectName("champSalaire");
        champSalaire->setDecimals(2);
        champSalaire->setMaximum(1000000.000000000000000);

        formLayout->addWidget(champSalaire, 8, 1, 1, 1);

        labelSolde = new QLabel(DialogEmploye);
        labelSolde->setObjectName("labelSolde");

        formLayout->addWidget(labelSolde, 9, 0, 1, 1);

        champSolde = new QSpinBox(DialogEmploye);
        champSolde->setObjectName("champSolde");
        champSolde->setMaximum(365);
        champSolde->setValue(30);

        formLayout->addWidget(champSolde, 9, 1, 1, 1);

        labelStatut = new QLabel(DialogEmploye);
        labelStatut->setObjectName("labelStatut");

        formLayout->addWidget(labelStatut, 10, 0, 1, 1);

        champStatut = new QComboBox(DialogEmploye);
        champStatut->setObjectName("champStatut");

        formLayout->addWidget(champStatut, 10, 1, 1, 1);

        labelMotDePasse = new QLabel(DialogEmploye);
        labelMotDePasse->setObjectName("labelMotDePasse");

        formLayout->addWidget(labelMotDePasse, 11, 0, 1, 1);

        champMotDePasse = new QLineEdit(DialogEmploye);
        champMotDePasse->setObjectName("champMotDePasse");
        champMotDePasse->setEchoMode(QLineEdit::Password);

        formLayout->addWidget(champMotDePasse, 11, 1, 1, 1);


        ligneContenu->addLayout(formLayout);

        colonnePhoto = new QVBoxLayout();
        colonnePhoto->setSpacing(8);
        colonnePhoto->setObjectName("colonnePhoto");
        apercuPhoto = new QLabel(DialogEmploye);
        apercuPhoto->setObjectName("apercuPhoto");
        apercuPhoto->setMinimumSize(QSize(150, 150));
        apercuPhoto->setMaximumSize(QSize(150, 150));
        apercuPhoto->setAlignment(Qt::AlignCenter);

        colonnePhoto->addWidget(apercuPhoto);

        btnChoisirPhoto = new QPushButton(DialogEmploye);
        btnChoisirPhoto->setObjectName("btnChoisirPhoto");

        colonnePhoto->addWidget(btnChoisirPhoto);

        btnRetirerPhoto = new QPushButton(DialogEmploye);
        btnRetirerPhoto->setObjectName("btnRetirerPhoto");

        colonnePhoto->addWidget(btnRetirerPhoto);

        aidePhoto = new QLabel(DialogEmploye);
        aidePhoto->setObjectName("aidePhoto");
        aidePhoto->setWordWrap(true);
        aidePhoto->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

        colonnePhoto->addWidget(aidePhoto);

        spacerPhoto = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        colonnePhoto->addItem(spacerPhoto);


        ligneContenu->addLayout(colonnePhoto);


        layoutPrincipal->addLayout(ligneContenu);

        labelObligatoire = new QLabel(DialogEmploye);
        labelObligatoire->setObjectName("labelObligatoire");

        layoutPrincipal->addWidget(labelObligatoire);

        ligneBoutons = new QHBoxLayout();
        ligneBoutons->setObjectName("ligneBoutons");
        spacerBoutons = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ligneBoutons->addItem(spacerBoutons);

        btnAnnuler = new QPushButton(DialogEmploye);
        btnAnnuler->setObjectName("btnAnnuler");

        ligneBoutons->addWidget(btnAnnuler);

        btnEnregistrer = new QPushButton(DialogEmploye);
        btnEnregistrer->setObjectName("btnEnregistrer");

        ligneBoutons->addWidget(btnEnregistrer);


        layoutPrincipal->addLayout(ligneBoutons);


        retranslateUi(DialogEmploye);

        QMetaObject::connectSlotsByName(DialogEmploye);
    } // setupUi

    void retranslateUi(QDialog *DialogEmploye)
    {
        DialogEmploye->setWindowTitle(QCoreApplication::translate("DialogEmploye", "Employ\303\251", nullptr));
        titreLabel->setText(QCoreApplication::translate("DialogEmploye", "Nouvel employ\303\251", nullptr));
        sousTitreLabel->setText(QCoreApplication::translate("DialogEmploye", "Renseignez les informations de la fiche employ\303\251. L'identifiant est g\303\251n\303\251r\303\251 automatiquement.", nullptr));
        labelId->setText(QCoreApplication::translate("DialogEmploye", "Identifiant", nullptr));
        labelNom->setText(QCoreApplication::translate("DialogEmploye", "Nom *", nullptr));
        champNom->setPlaceholderText(QCoreApplication::translate("DialogEmploye", "Nom de famille", nullptr));
        labelPrenom->setText(QCoreApplication::translate("DialogEmploye", "Pr\303\251nom *", nullptr));
        champPrenom->setPlaceholderText(QCoreApplication::translate("DialogEmploye", "Pr\303\251nom", nullptr));
        labelEmail->setText(QCoreApplication::translate("DialogEmploye", "E-mail *", nullptr));
        champEmail->setPlaceholderText(QCoreApplication::translate("DialogEmploye", "prenom.nom@exemple.tn", nullptr));
        labelTelephone->setText(QCoreApplication::translate("DialogEmploye", "T\303\251l\303\251phone *", nullptr));
        champTelephone->setPlaceholderText(QCoreApplication::translate("DialogEmploye", "+216 20 123 456", nullptr));
        labelPoste->setText(QCoreApplication::translate("DialogEmploye", "Poste *", nullptr));
        labelService->setText(QCoreApplication::translate("DialogEmploye", "Service *", nullptr));
        labelDate->setText(QCoreApplication::translate("DialogEmploye", "Date de recrutement *", nullptr));
        champDate->setDisplayFormat(QCoreApplication::translate("DialogEmploye", "dd/MM/yyyy", nullptr));
        labelSalaire->setText(QCoreApplication::translate("DialogEmploye", "Salaire *", nullptr));
        champSalaire->setSuffix(QCoreApplication::translate("DialogEmploye", " DT", nullptr));
        labelSolde->setText(QCoreApplication::translate("DialogEmploye", "Solde de cong\303\251s *", nullptr));
        champSolde->setSuffix(QCoreApplication::translate("DialogEmploye", " j", nullptr));
        labelStatut->setText(QCoreApplication::translate("DialogEmploye", "Statut *", nullptr));
        labelMotDePasse->setText(QCoreApplication::translate("DialogEmploye", "Mot de passe *", nullptr));
        champMotDePasse->setPlaceholderText(QCoreApplication::translate("DialogEmploye", "Minimum 4 caract\303\250res", nullptr));
        btnChoisirPhoto->setText(QCoreApplication::translate("DialogEmploye", "Choisir une photo\342\200\246", nullptr));
        btnRetirerPhoto->setText(QCoreApplication::translate("DialogEmploye", "Retirer la photo", nullptr));
        aidePhoto->setText(QCoreApplication::translate("DialogEmploye", "JPG ou PNG \342\200\224 150 \303\227 150 recommand\303\251", nullptr));
        labelObligatoire->setText(QCoreApplication::translate("DialogEmploye", "* Champs obligatoires", nullptr));
        btnAnnuler->setText(QCoreApplication::translate("DialogEmploye", "Annuler", nullptr));
        btnEnregistrer->setText(QCoreApplication::translate("DialogEmploye", "Enregistrer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogEmploye: public Ui_DialogEmploye {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGEMPLOYE_H
