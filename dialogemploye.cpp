#include "dialogemploye.h"
#include "ui_dialogemploye.h"

#include "outilsemployes.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>

DialogEmploye::DialogEmploye(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogEmploye)
{
    ui->setupUi(this);
    setModal(true);
    appliquerStyle();
    remplirListes();

    ui->champDate->setMaximumDate(QDate::currentDate());
    ui->champDate->setDate(QDate::currentDate());
    ui->champSalaire->setMinimum(0.01);
    ui->champSalaire->setMaximum(1000000.0);
    ui->champSalaire->setDecimals(2);
    ui->champSolde->setRange(0, 365);
    ui->champPoste->setCurrentIndex(-1);
    ui->champService->setCurrentIndex(-1);
    ui->champStatut->setCurrentText("Actif");

    connect(ui->btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnEnregistrer, &QPushButton::clicked, this, &DialogEmploye::slotEnregistrer);
    connect(ui->btnChoisirPhoto, &QPushButton::clicked, this, &DialogEmploye::slotChoisirPhoto);
    connect(ui->btnRetirerPhoto, &QPushButton::clicked, this, &DialogEmploye::slotRetirerPhoto);

    modeAjout();
}

DialogEmploye::~DialogEmploye()
{
    delete ui;
}

void DialogEmploye::appliquerStyle()
{
    setStyleSheet(R"(
        QDialog { background-color: #F7FAFC; }
        QLabel { color: #1B3346; font-size: 9pt; }
        QLabel#titreLabel { font-size: 18pt; font-weight: 600; color: #183B4E; }
        QLabel#sousTitreLabel { color: #6B7C8C; font-size: 9pt; }
        QLabel#aidePhoto, QLabel#labelObligatoire { color: #7A8B99; font-size: 8pt; }
        QLabel#apercuPhoto {
            background: #FFFFFF;
            border: 1px solid #D8E2EA;
            border-radius: 12px;
            color: #8A9BA8;
            font-size: 8pt;
        }
        QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QSpinBox {
            background: #FFFFFF;
            border: 1px solid #D8E2EA;
            border-radius: 6px;
            padding: 7px 9px;
            min-height: 20px;
            color: #1B3346;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus,
        QDoubleSpinBox:focus, QSpinBox:focus {
            border: 1px solid #179BAE;
        }
        QLineEdit:read-only { background: #EEF3F7; color: #6B7C8C; }
        QComboBox::drop-down { border: none; width: 24px; }
        QPushButton#btnEnregistrer {
            background: #179BAE; color: white; border: none; border-radius: 6px;
            padding: 9px 24px; font-weight: 600;
        }
        QPushButton#btnEnregistrer:hover { background: #12879A; }
        QPushButton#btnAnnuler {
            background: #E8EEF3; color: #1B3346; border: none; border-radius: 6px;
            padding: 9px 20px;
        }
        QPushButton#btnAnnuler:hover { background: #DCE5EC; }
        QPushButton#btnChoisirPhoto {
            background: #E7F4F6; color: #137A6E; border: 1px solid #BFE3E5;
            border-radius: 6px; padding: 7px 12px;
        }
        QPushButton#btnChoisirPhoto:hover { background: #D8EFF1; }
        QPushButton#btnRetirerPhoto {
            background: transparent; color: #C0392B; border: none; padding: 4px;
        }
    )");
}

void DialogEmploye::remplirListes()
{
    const QStringList postes = fusionnerListes(Employe::postesParDefaut(),
                                               Employes::postesEnBase());
    const QStringList services = fusionnerListes(Employe::servicesParDefaut(),
                                                 Employes::servicesEnBase());

    ui->champPoste->clear();
    ui->champPoste->addItems(postes);
    ui->champService->clear();
    ui->champService->addItems(services);
    ui->champStatut->clear();
    ui->champStatut->addItems(Employe::statuts());
}

void DialogEmploye::modeAjout()
{
    m_modification = false;
    m_photoModifiee = false;
    m_photoSource.clear();
    m_employe = Employe();

    setWindowTitle("Nouvel employé");
    ui->titreLabel->setText("Nouvel employé");
    ui->sousTitreLabel->setText(
        "Renseignez les informations de la fiche employé. "
        "L'identifiant est généré automatiquement.");
    ui->labelMotDePasse->setText("Mot de passe *");
    ui->champMotDePasse->setPlaceholderText("Minimum 4 caractères");

    ui->champId->setText("Automatique");
    ui->champNom->clear();
    ui->champPrenom->clear();
    ui->champEmail->clear();
    ui->champTelephone->clear();
    ui->champPoste->setCurrentIndex(-1);
    ui->champService->setCurrentIndex(-1);
    ui->champPoste->setCurrentText(QString());
    ui->champService->setCurrentText(QString());
    ui->champDate->setDate(QDate::currentDate());
    ui->champSalaire->setValue(0.0);
    ui->champSolde->setValue(30);
    ui->champStatut->setCurrentText("Actif");
    ui->champMotDePasse->clear();
    mettreAJourApercu();
    ui->champNom->setFocus();
}

void DialogEmploye::chargerEmploye(const Employe &employe)
{
    m_modification = true;
    m_photoModifiee = false;
    m_photoSource.clear();
    m_employe = employe;

    setWindowTitle("Modifier l'employé");
    ui->titreLabel->setText("Modifier l'employé");
    ui->sousTitreLabel->setText(
        "L'identifiant reste inchangé. Laissez le mot de passe vide pour le conserver.");
    ui->labelMotDePasse->setText("Mot de passe");
    ui->champMotDePasse->setPlaceholderText("Laisser vide pour conserver");

    ui->champId->setText(QString("#%1").arg(employe.id));
    ui->champNom->setText(employe.nom);
    ui->champPrenom->setText(employe.prenom);
    ui->champEmail->setText(employe.email);
    ui->champTelephone->setText(employe.telephone);

    if (ui->champPoste->findText(employe.poste, Qt::MatchFixedString) < 0 &&
        !employe.poste.trimmed().isEmpty()) {
        ui->champPoste->addItem(employe.poste);
    }
    if (ui->champService->findText(employe.service, Qt::MatchFixedString) < 0 &&
        !employe.service.trimmed().isEmpty()) {
        ui->champService->addItem(employe.service);
    }
    ui->champPoste->setCurrentText(employe.poste);
    ui->champService->setCurrentText(employe.service);

    QDate date = dateDepuisTexte(employe.dateRecrutement);
    if (!date.isValid() || date > QDate::currentDate()) {
        date = QDate::currentDate();
    }
    ui->champDate->setDate(date);
    ui->champSalaire->setValue(employe.salaire);
    ui->champSolde->setValue(employe.soldeConges);
    if (ui->champStatut->findText(employe.statut, Qt::MatchFixedString) < 0) {
        ui->champStatut->addItem(employe.statut);
    }
    ui->champStatut->setCurrentText(employe.statut);
    ui->champMotDePasse->clear();

    mettreAJourApercu();
    ui->champNom->setFocus();
}

void DialogEmploye::mettreAJourApercu()
{
    ui->apercuPhoto->clear();
    QString chemin;
    if (m_photoModifiee) {
        chemin = m_photoSource;
    } else {
        chemin = m_employe.photo;
    }

    if (!chemin.trimmed().isEmpty() && QFileInfo::exists(chemin)) {
        QPixmap pixmap(chemin);
        if (!pixmap.isNull()) {
            ui->apercuPhoto->setPixmap(
                pixmap.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            return;
        }
    }

    ui->apercuPhoto->setText(m_photoModifiee ? "Aucune photo" : "Aucune photo");
}

void DialogEmploye::slotChoisirPhoto()
{
    const QString chemin = QFileDialog::getOpenFileName(
        this, "Choisir une photo", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (chemin.isEmpty()) {
        return;
    }

    QPixmap pixmap(chemin);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Photo",
                             "Le fichier sélectionné n'est pas une image valide.");
        return;
    }

    m_photoSource = chemin;
    m_photoModifiee = true;
    mettreAJourApercu();
}

void DialogEmploye::slotRetirerPhoto()
{
    m_photoSource.clear();
    m_photoModifiee = true;
    mettreAJourApercu();
}

bool DialogEmploye::verifierSaisie()
{
    if (ui->champNom->text().trimmed().isEmpty() ||
        ui->champPrenom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires",
                             "Le nom et le prénom sont obligatoires.");
        ui->champNom->setFocus();
        return false;
    }
    if (ui->champEmail->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires",
                             "L'adresse e-mail est obligatoire.");
        ui->champEmail->setFocus();
        return false;
    }
    if (ui->champTelephone->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires",
                             "Le téléphone est obligatoire.");
        ui->champTelephone->setFocus();
        return false;
    }
    if (ui->champPoste->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Le poste est obligatoire.");
        ui->champPoste->setFocus();
        return false;
    }
    if (ui->champService->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Le service est obligatoire.");
        ui->champService->setFocus();
        return false;
    }
    if (ui->champSalaire->value() <= 0.0) {
        QMessageBox::warning(this, "Salaire",
                             "Le salaire doit être supérieur à 0.");
        ui->champSalaire->setFocus();
        return false;
    }

    const QString motDePasse = ui->champMotDePasse->text();
    if (!m_modification && motDePasse.trimmed().size() < 4) {
        QMessageBox::warning(this, "Mot de passe",
                             "Le mot de passe doit contenir au moins 4 caractères.");
        ui->champMotDePasse->setFocus();
        return false;
    }
    if (m_modification && !motDePasse.isEmpty() && motDePasse.trimmed().size() < 4) {
        QMessageBox::warning(this, "Mot de passe",
                             "Le nouveau mot de passe doit contenir au moins 4 caractères.");
        ui->champMotDePasse->setFocus();
        return false;
    }
    return true;
}

void DialogEmploye::slotEnregistrer()
{
    if (!verifierSaisie()) {
        return;
    }

    Employe e;
    e.id = m_modification ? m_employe.id : 0;
    e.nom = ui->champNom->text().trimmed();
    e.prenom = ui->champPrenom->text().trimmed();
    e.email = ui->champEmail->text().trimmed();
    e.telephone = ui->champTelephone->text().trimmed();
    e.poste = ui->champPoste->currentText().trimmed();
    e.service = ui->champService->currentText().trimmed();
    e.dateRecrutement = dateIso(ui->champDate->date());
    e.salaire = ui->champSalaire->value();
    e.soldeConges = ui->champSolde->value();
    e.statut = ui->champStatut->currentText().trimmed();
    e.motDePasse = ui->champMotDePasse->text();

    const QString anciennePhoto = m_employe.photo;
    QString photoCopiee;
    if (m_photoModifiee) {
        if (!m_photoSource.trimmed().isEmpty()) {
            const QFileInfo source(m_photoSource);
            QString extension = source.suffix().toLower();
            if (extension.isEmpty()) {
                extension = "png";
            }
            const QString nomFichier = QString("employe_%1_%2.%3")
                .arg(e.id > 0 ? e.id : 0)
                .arg(QDateTime::currentMSecsSinceEpoch())
                .arg(extension);
            const QString destination = Employes::dossierPhotos() + "/" + nomFichier;
            if (!QFile::copy(m_photoSource, destination)) {
                QMessageBox::warning(this, "Photo",
                                     "Impossible de copier la photo dans le dossier de l'application.");
                return;
            }
            photoCopiee = destination;
            e.photo = destination;
        } else {
            e.photo.clear();
        }
    } else {
        e.photo = m_employe.photo;
    }

    QString erreur;
    const bool ok = m_modification ? Employes::modifier(e, &erreur)
                                   : Employes::ajouter(e, &erreur);
    if (!ok) {
        if (!photoCopiee.isEmpty()) {
            QFile::remove(photoCopiee);
        }
        QMessageBox::warning(this, "Enregistrement impossible",
                             erreur.isEmpty() ? "Une erreur est survenue." : erreur);
        return;
    }

    if (!photoCopiee.isEmpty() || m_photoModifiee) {
        if (!anciennePhoto.isEmpty() && anciennePhoto != e.photo &&
            anciennePhoto.startsWith(Employes::dossierPhotos()) &&
            QFileInfo::exists(anciennePhoto)) {
            QFile::remove(anciennePhoto);
        }
    }

    const Employe recharge = Employes::charger(e.id);
    m_employe = (recharge.id > 0) ? recharge : e;
    accept();
}
