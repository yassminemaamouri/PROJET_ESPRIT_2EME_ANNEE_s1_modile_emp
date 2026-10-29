#ifndef DELEGUESEMPLOYES_H
#define DELEGUESEMPLOYES_H

#include "icones.h"
#include "outilsemployes.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QToolTip>

/*
 * Délégués du tableau des employés :
 *  - vignette de photo (ou initiales si aucune photo) ;
 *  - badge de statut ;
 *  - trois actions consulter / modifier / désactiver.
 */

class DeleguePhotoEmploye : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DeleguePhotoEmploye(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text.clear();

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        const QString nom = index.sibling(index.row(), 2).data().toString();
        const QString prenom = index.sibling(index.row(), 3).data().toString();
        const QString photo = index.data(Qt::UserRole + 10).toString();

        const QRect zone = opt.rect.adjusted(5, 5, -5, -5);
        const int taille = qMax(0, qMin(zone.width(), zone.height()));
        if (taille <= 0) {
            return;
        }
        const QRect avatar(zone.center().x() - taille / 2,
                           zone.center().y() - taille / 2,
                           taille, taille);
        const qreal rayon = 5.0;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QPainterPath chemin;
        chemin.addRoundedRect(avatar, rayon, rayon);
        painter->setClipPath(chemin);

        QPixmap pixmap(photo);
        if (!photo.trimmed().isEmpty() && !pixmap.isNull()) {
            painter->fillRect(avatar, QColor("#F8FAFC"));
            // Conserver toute la photo, sans la rogner : les proportions
            // restent identiques à celles du fichier d'origine.
            const QPixmap redimensionne = pixmap.scaled(
                avatar.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            const QPoint origine(avatar.center().x() - redimensionne.width() / 2,
                                 avatar.center().y() - redimensionne.height() / 2);
            painter->drawPixmap(origine, redimensionne);
        } else {
            painter->fillRect(avatar, couleurAvatar(nom, prenom));
            QFont police = opt.font;
            police.setBold(true);
            police.setPointSize(qMax(8, taille / 4));
            painter->setFont(police);
            painter->setPen(Qt::white);
            painter->drawText(avatar, Qt::AlignCenter,
                              initiales(nom, prenom));
        }
        painter->restore();

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(QColor("#DCE6EE"), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(avatar.adjusted(0, 0, -1, -1), rayon, rayon);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return QSize(52, 48);
    }
};

class DelegueStatutEmploye : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegueStatutEmploye(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QString texte = statutCongeNormalise(opt.text);
        opt.text.clear();

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        QString fond = "#EEF3F7";
        QString couleur = "#5B6B79";
        if (texte.compare("Actif", Qt::CaseInsensitive) == 0 ||
            texte.compare("Accepté", Qt::CaseInsensitive) == 0) {
            fond = "#E5F5EC";
            couleur = "#137A4B";
        } else if (texte.compare("En congé", Qt::CaseInsensitive) == 0 ||
                   texte.compare("En attente", Qt::CaseInsensitive) == 0) {
            fond = "#FFF4DC";
            couleur = "#A66A00";
        } else if (texte.compare("Inactif", Qt::CaseInsensitive) == 0) {
            fond = "#F1F4F7";
            couleur = "#6B7C8C";
        } else if (texte.compare("Refusé", Qt::CaseInsensitive) == 0) {
            fond = "#FDECEA";
            couleur = "#C0392B";
        } else if (texte.compare("Annulé", Qt::CaseInsensitive) == 0) {
            fond = "#EEF1F4";
            couleur = "#7A8B99";
        }

        QFont police = opt.font;
        police.setBold(true);
        police.setPointSize(8);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setFont(police);
        const QFontMetrics metrics(police);
        const int largeur = qMin(opt.rect.width() - 12,
                                 metrics.horizontalAdvance(texte) + 22);
        const int hauteur = 24;
        const QRect badge(opt.rect.center().x() - largeur / 2,
                          opt.rect.center().y() - hauteur / 2,
                          largeur, hauteur);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(fond));
        painter->drawRoundedRect(badge, hauteur / 2, hauteur / 2);
        painter->setPen(QColor(couleur));
        painter->drawText(badge, Qt::AlignCenter, texte);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return QSize(112, 44);
    }
};

class DelegueActionsEmploye : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegueActionsEmploye(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text.clear();
        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        const QRect zone = opt.rect.adjusted(6, 0, -6, 0);
        const int largeur = zone.width() / 3;
        const QVector<QPair<Icones::Type, QColor>> actions = {
            {Icones::Oeil, QColor("#1B3346")},
            {Icones::Crayon, QColor("#179BAE")},
            {Icones::CroixCercle, QColor("#D64545")}
        };

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        for (int i = 0; i < actions.size(); ++i) {
            const QRect cellule(zone.left() + i * largeur, zone.top(), largeur, zone.height());
            const QPixmap pixmap = Icones::dessiner(actions.at(i).first,
                                                    actions.at(i).second, 40);
            const QRect cible(cellule.center().x() - 9, cellule.center().y() - 9, 18, 18);
            painter->drawPixmap(cible, pixmap);
        }
        painter->restore();
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override
    {
        Q_UNUSED(model);
        if (event->type() != QEvent::MouseButtonRelease) {
            return false;
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPoint position = static_cast<QMouseEvent *>(event)->position().toPoint();
#else
        const QPoint position = static_cast<QMouseEvent *>(event)->pos();
#endif
        const QRect zone = option.rect.adjusted(6, 0, -6, 0);
        const int largeur = zone.width() / 3;
        if (largeur <= 0 || !zone.contains(position)) {
            return false;
        }
        const int action = qBound(0, (position.x() - zone.left()) / largeur, 2);
        const int id = index.sibling(index.row(), 0).data().toInt();
        if (action == 0) {
            emit consulter(id);
        } else if (action == 1) {
            emit modifier(id);
        } else {
            emit desactiver(id);
        }
        return true;
    }

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override
    {
        if (event->type() != QEvent::ToolTip) {
            return QStyledItemDelegate::helpEvent(event, view, option, index);
        }
        const QRect zone = option.rect.adjusted(6, 0, -6, 0);
        const int largeur = zone.width() / 3;
        if (largeur <= 0 || !zone.contains(event->pos())) {
            return false;
        }
        const int action = qBound(0, (event->pos().x() - zone.left()) / largeur, 2);
        const QStringList libelles = {
            QStringLiteral("Consulter la fiche"),
            QStringLiteral("Modifier la fiche"),
            QStringLiteral("Désactiver l'employé")
        };
        QToolTip::showText(event->globalPos(), libelles.at(action),
                           static_cast<QWidget *>(view));
        return true;
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return QSize(132, 44);
    }

signals:
    void consulter(int id);
    void modifier(int id);
    void desactiver(int id);
};

#endif // DELEGUESEMPLOYES_H
