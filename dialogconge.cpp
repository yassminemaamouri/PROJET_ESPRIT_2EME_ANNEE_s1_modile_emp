#include "dialogconge.h"

#include "outilsemployes.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

DialogConge::DialogConge(int employeId, const QString &nomEmploye, QWidget *parent)
    : QDialog(parent)
    , m_employeId(employeId)
    , m_nomEmploye(nomEmploye)
{
    setModal(true);
    setWindowTitle("Nouvelle demande de congé");
    setMinimumWidth(540);
    appliquerStyle();

    m_droit = Conge::droitAnnuel(m_employeId);
    m_utilises = Conge::joursUtilises(m_employeId);

    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->setContentsMargins(26, 22, 26, 20);
    principal->setSpacing(14);

    QLabel *titre = new QLabel("Nouvelle demande de congé", this);
    titre->setObjectName("titreDialogue");
    principal->addWidget(titre);

    QLabel *sousTitre = new QLabel(
        QString("Employé : <b>%1</b>").arg(m_nomEmploye.toHtmlEscaped()), this);
    sousTitre->setObjectName("sousTitreDialogue");
    principal->addWidget(sousTitre);

    m_solde = new QLabel(this);
    m_solde->setObjectName("encartSolde");
    m_solde->setWordWrap(true);
    principal->addWidget(m_solde);
    mettreAJourSolde();

    QFrame *ligne = new QFrame(this);
    ligne->setFrameShape(QFrame::HLine);
    ligne->setObjectName("separateur");
    principal->addWidget(ligne);

    QLabel *labelType = new QLabel("Type de congé", this);
    principal->addWidget(labelType);
    m_type = new QComboBox(this);
    m_type->setEditable(true);
    m_type->addItems(Conge::types());
    m_type->setCurrentText("Congé annuel");
    principal->addWidget(m_type);

    QHBoxLayout *dates = new QHBoxLayout();
    dates->setSpacing(14);
    QVBoxLayout *colonneDebut = new QVBoxLayout();
    colonneDebut->setSpacing(5);
    colonneDebut->addWidget(new QLabel("Date de début", this));
    m_debut = new QDateEdit(QDate::currentDate(), this);
    m_debut->setDisplayFormat("dd/MM/yyyy");
    m_debut->setCalendarPopup(true);
    colonneDebut->addWidget(m_debut);
    dates->addLayout(colonneDebut);

    QVBoxLayout *colonneFin = new QVBoxLayout();
    colonneFin->setSpacing(5);
    colonneFin->addWidget(new QLabel("Date de fin", this));
    m_fin = new QDateEdit(QDate::currentDate(), this);
    m_fin->setDisplayFormat("dd/MM/yyyy");
    m_fin->setCalendarPopup(true);
    colonneFin->addWidget(m_fin);
    dates->addLayout(colonneFin);
    principal->addLayout(dates);

    m_jours = new QLabel(this);
    m_jours->setObjectName("joursCalcules");
    principal->addWidget(m_jours);

    QLabel *labelMotif = new QLabel("Motif (optionnel)", this);
    principal->addWidget(labelMotif);
    m_motif = new QTextEdit(this);
    m_motif->setPlaceholderText("Précisez éventuellement le motif de la demande…");
    m_motif->setFixedHeight(70);
    principal->addWidget(m_motif);

    m_erreur = new QLabel(this);
    m_erreur->setObjectName("messageErreur");
    m_erreur->setWordWrap(true);
    m_erreur->hide();
    principal->addWidget(m_erreur);

    QLabel *aide = new QLabel(
        "Les samedis et dimanches ne sont pas décomptés. "
        "La demande est enregistrée avec le statut « En attente ».",
        this);
    aide->setObjectName("aideDialogue");
    aide->setWordWrap(true);
    principal->addWidget(aide);

    QHBoxLayout *boutons = new QHBoxLayout();
    boutons->addStretch();
    QPushButton *annuler = new QPushButton("Annuler", this);
    annuler->setObjectName("btnAnnulerConge");
    QPushButton *envoyer = new QPushButton("Envoyer la demande", this);
    envoyer->setObjectName("btnEnvoyerConge");
    boutons->addWidget(annuler);
    boutons->addWidget(envoyer);
    principal->addLayout(boutons);

    connect(annuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(envoyer, &QPushButton::clicked, this, &DialogConge::envoyerDemande);
    connect(m_debut, &QDateEdit::dateChanged, this, &DialogConge::actualiserJours);
    connect(m_fin, &QDateEdit::dateChanged, this, &DialogConge::actualiserJours);

    actualiserJours();
}

void DialogConge::appliquerStyle()
{
    setStyleSheet(R"(
        QDialog { background-color: #F7FAFC; }
        QLabel { color: #1B3346; font-size: 9pt; }
        QLabel#titreDialogue { font-size: 16pt; font-weight: 600; color: #183B4E; }
        QLabel#sousTitreDialogue { color: #4A6678; }
        QLabel#encartSolde {
            background: #EAF6F7; color: #137A6E; border: 1px solid #C9E8EA;
            border-radius: 8px; padding: 10px 12px; font-weight: 600;
        }
        QLabel#joursCalcules { color: #179BAE; font-weight: 600; }
        QLabel#messageErreur {
            background: #FDECEA; color: #B03A2E; border: 1px solid #F5C6C0;
            border-radius: 6px; padding: 8px;
        }
        QLabel#aideDialogue { color: #7A8B99; font-size: 8pt; }
        QFrame#separateur { color: #E1E8EE; }
        QComboBox, QDateEdit, QTextEdit {
            background: white; border: 1px solid #D8E2EA; border-radius: 6px;
            padding: 7px 9px; min-height: 20px; color: #1B3346;
        }
        QComboBox:focus, QDateEdit:focus, QTextEdit:focus { border: 1px solid #179BAE; }
        QComboBox::drop-down { border: none; width: 24px; }
        QPushButton#btnEnvoyerConge {
            background: #179BAE; color: white; border: none; border-radius: 6px;
            padding: 9px 20px; font-weight: 600;
        }
        QPushButton#btnEnvoyerConge:hover { background: #12879A; }
        QPushButton#btnAnnulerConge {
            background: #E8EEF3; color: #1B3346; border: none; border-radius: 6px;
            padding: 9px 18px;
        }
        QPushButton#btnAnnulerConge:hover { background: #DCE5EC; }
    )");
}

void DialogConge::mettreAJourSolde()
{
    const int restants = qMax(0, m_droit - m_utilises);
    m_solde->setText(
        QString("Solde disponible : <b>%1 jour(s)</b> — droit annuel %2 jour(s), "
                "%3 jour(s) déjà utilisé(s) en %4.")
            .arg(restants)
            .arg(m_droit)
            .arg(m_utilises)
            .arg(QDate::currentDate().year()));
}

void DialogConge::actualiserJours()
{
    if (!m_debut || !m_fin || !m_jours) {
        return;
    }
    const int jours = joursOuvrables(m_debut->date(), m_fin->date());
    if (m_fin->date() < m_debut->date()) {
        m_jours->setText("La date de fin doit être postérieure à la date de début.");
        m_jours->setStyleSheet("color:#C0392B;");
        return;
    }
    if (jours <= 0) {
        m_jours->setText("Aucun jour ouvrable sur cette période.");
        m_jours->setStyleSheet("color:#C0392B;");
        return;
    }
    m_jours->setStyleSheet(QString());
    m_jours->setText(QString("%1 jour(s) ouvrable(s) décompté(s)").arg(jours));
}

void DialogConge::envoyerDemande()
{
    m_erreur->hide();

    if (m_type->currentText().trimmed().isEmpty()) {
        m_erreur->setText("Le type de congé est obligatoire.");
        m_erreur->show();
        return;
    }
    if (m_fin->date() < m_debut->date()) {
        m_erreur->setText("La date de fin doit être postérieure ou égale à la date de début.");
        m_erreur->show();
        return;
    }
    const int jours = joursOuvrables(m_debut->date(), m_fin->date());
    if (jours <= 0) {
        m_erreur->setText("La période ne contient aucun jour ouvrable (lundi à vendredi).");
        m_erreur->show();
        return;
    }

    Conge c;
    c.employeId = m_employeId;
    c.typeConge = m_type->currentText().trimmed();
    c.dateDebut = m_debut->date();
    c.dateFin = m_fin->date();
    c.motif = m_motif->toPlainText().trimmed();

    QString erreur;
    if (!c.ajouter(&erreur)) {
        m_erreur->setText(erreur.isEmpty() ? "La demande n'a pas pu être enregistrée." : erreur);
        m_erreur->show();
        return;
    }

    m_conge = c;
    accept();
}
