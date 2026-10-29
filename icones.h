#ifndef ICONES_H
#define ICONES_H

#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QRectF>

/*
 * Icônes vectorielles dessinées par code (aucun fichier image externe requis).
 * Repère 100 x 100 avec mise à l'échelle automatique et antialiasing haute définition.
 */
namespace Icones {

enum Type {
    Utilisateur,
    GroupeUtilisateurs,
    AjoutUtilisateur,
    CocheCercle,
    CroixCercle,
    Antenne,
    Thermometre,
    Cle,
    Maison,
    Deconnexion,
    Oeil,
    Crayon,
    Poubelle,
    Loupe,
    Grille,
    FlecheBas,
    FlecheGauche,
    FlecheDroite,
    Avatar,
    Logo,
    GraphiqueLigne,
    GraphiqueCamembert,
    GraphiqueBarres,
    Calendrier,
    Filtre,
    DocumentPdf,
    Email,
    Telephone,
    Localisation,
    Mallette,
    Plus,
    Rafraichir,
    Parametres,
    Rapports,
    Services,
    Horloge
};

inline QPixmap dessiner(Type type, const QColor &couleur, int taille = 96)
{
    QPixmap pixmap(taille, taille);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.scale(taille / 100.0, taille / 100.0);

    const QPen stylo(couleur, 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(stylo);
    p.setBrush(Qt::NoBrush);

    switch (type) {
    case Utilisateur: {
        p.drawEllipse(QPointF(50, 32), 15, 15);
        QPainterPath corps;
        corps.moveTo(22, 88);
        corps.cubicTo(22, 56, 78, 56, 78, 88);
        p.drawPath(corps);
        break;
    }
    case GroupeUtilisateurs: {
        // Main user in center
        p.drawEllipse(QPointF(50, 30), 13, 13);
        QPainterPath c1;
        c1.moveTo(26, 86);
        c1.cubicTo(26, 58, 74, 58, 74, 86);
        p.drawPath(c1);
        // Side user left
        p.drawEllipse(QPointF(24, 34), 10, 10);
        QPainterPath c2;
        c2.moveTo(8, 86);
        c2.cubicTo(8, 64, 30, 64, 30, 76);
        p.drawPath(c2);
        // Side user right
        p.drawEllipse(QPointF(76, 34), 10, 10);
        QPainterPath c3;
        c3.moveTo(70, 76);
        c3.cubicTo(70, 64, 92, 64, 92, 86);
        p.drawPath(c3);
        break;
    }
    case AjoutUtilisateur: {
        p.drawEllipse(QPointF(40, 32), 14, 14);
        QPainterPath corps;
        corps.moveTo(14, 86);
        corps.cubicTo(14, 58, 66, 58, 66, 86);
        p.drawPath(corps);
        // Plus sign on top right
        p.drawLine(QPointF(80, 28), QPointF(80, 48));
        p.drawLine(QPointF(70, 38), QPointF(90, 38));
        break;
    }
    case CocheCercle: {
        p.drawEllipse(QPointF(50, 50), 40, 40);
        QPainterPath check;
        check.moveTo(30, 50);
        check.lineTo(44, 66);
        check.lineTo(72, 34);
        p.drawPath(check);
        break;
    }
    case CroixCercle: {
        p.drawEllipse(QPointF(50, 50), 40, 40);
        p.drawLine(QPointF(34, 34), QPointF(66, 66));
        p.drawLine(QPointF(66, 34), QPointF(34, 66));
        break;
    }
    case Antenne: {
        QPainterPath mat;
        mat.moveTo(34, 92);
        mat.lineTo(50, 42);
        mat.lineTo(66, 92);
        p.drawPath(mat);
        p.drawLine(QPointF(40, 74), QPointF(60, 74));
        p.setBrush(couleur);
        p.drawEllipse(QPointF(50, 32), 4, 4);
        p.setBrush(Qt::NoBrush);
        for (int rayon = 14; rayon <= 26; rayon += 12) {
            const QRectF cadre(50 - rayon, 32 - rayon, 2.0 * rayon, 2.0 * rayon);
            p.drawArc(cadre, 130 * 16, 100 * 16);
            p.drawArc(cadre, -50 * 16, 100 * 16);
        }
        break;
    }
    case Thermometre: {
        p.drawRoundedRect(QRectF(40, 8, 20, 60), 10, 10);
        p.setBrush(couleur);
        p.drawEllipse(QPointF(50, 78), 14, 14);
        p.drawLine(QPointF(50, 78), QPointF(50, 36));
        break;
    }
    case Cle: {
        p.drawLine(QPointF(26, 76), QPointF(64, 38));
        p.drawEllipse(QPointF(72, 28), 15, 15);
        p.setBrush(couleur);
        p.drawEllipse(QPointF(22, 80), 6, 6);
        break;
    }
    case Maison: {
        QPainterPath toit;
        toit.moveTo(10, 50);
        toit.lineTo(50, 14);
        toit.lineTo(90, 50);
        p.drawPath(toit);
        QPainterPath mur;
        mur.moveTo(24, 42);
        mur.lineTo(24, 88);
        mur.lineTo(76, 88);
        mur.lineTo(76, 42);
        p.drawPath(mur);
        p.drawRect(QRectF(42, 60, 16, 28));
        break;
    }
    case Deconnexion: {
        QPainterPath cadre;
        cadre.moveTo(44, 16);
        cadre.lineTo(16, 16);
        cadre.lineTo(16, 84);
        cadre.lineTo(44, 84);
        p.drawPath(cadre);
        p.drawLine(QPointF(36, 50), QPointF(86, 50));
        QPainterPath fleche;
        fleche.moveTo(68, 32);
        fleche.lineTo(86, 50);
        fleche.lineTo(68, 68);
        p.drawPath(fleche);
        break;
    }
    case Oeil: {
        QPainterPath oeil;
        oeil.moveTo(6, 50);
        oeil.quadTo(50, 12, 94, 50);
        oeil.quadTo(50, 88, 6, 50);
        p.drawPath(oeil);
        p.drawEllipse(QPointF(50, 50), 13, 13);
        break;
    }
    case Crayon: {
        QPainterPath crayon;
        crayon.moveTo(20, 82);
        crayon.lineTo(26, 60);
        crayon.lineTo(66, 20);
        crayon.lineTo(80, 34);
        crayon.lineTo(40, 74);
        crayon.closeSubpath();
        p.drawPath(crayon);
        p.drawLine(QPointF(58, 28), QPointF(72, 42));
        break;
    }
    case Poubelle: {
        p.drawLine(QPointF(16, 26), QPointF(84, 26));
        QPainterPath poignee;
        poignee.moveTo(38, 26);
        poignee.lineTo(38, 12);
        poignee.lineTo(62, 12);
        poignee.lineTo(62, 26);
        p.drawPath(poignee);
        QPainterPath corps;
        corps.moveTo(24, 26);
        corps.lineTo(30, 88);
        corps.lineTo(70, 88);
        corps.lineTo(76, 26);
        p.drawPath(corps);
        p.drawLine(QPointF(42, 40), QPointF(42, 74));
        p.drawLine(QPointF(58, 40), QPointF(58, 74));
        break;
    }
    case Loupe: {
        p.drawEllipse(QPointF(42, 42), 24, 24);
        p.drawLine(QPointF(60, 60), QPointF(86, 86));
        break;
    }
    case Grille: {
        p.drawRoundedRect(QRectF(14, 14, 32, 32), 6, 6);
        p.drawRoundedRect(QRectF(54, 14, 32, 32), 6, 6);
        p.drawRoundedRect(QRectF(14, 54, 32, 32), 6, 6);
        p.drawRoundedRect(QRectF(54, 54, 32, 32), 6, 6);
        break;
    }
    case FlecheBas: {
        p.setPen(Qt::NoPen);
        p.setBrush(couleur);
        QPolygonF triangle;
        triangle << QPointF(20, 36) << QPointF(80, 36) << QPointF(50, 72);
        p.drawPolygon(triangle);
        break;
    }
    case FlecheGauche: {
        p.drawLine(QPointF(76, 50), QPointF(24, 50));
        QPainterPath f;
        f.moveTo(46, 28);
        f.lineTo(24, 50);
        f.lineTo(46, 72);
        p.drawPath(f);
        break;
    }
    case FlecheDroite: {
        p.drawLine(QPointF(24, 50), QPointF(76, 50));
        QPainterPath f;
        f.moveTo(54, 28);
        f.lineTo(76, 50);
        f.lineTo(54, 72);
        p.drawPath(f);
        break;
    }
    case Avatar: {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#D3E2EE"));
        p.drawRoundedRect(QRectF(0, 0, 100, 100), 16, 16);
        p.setBrush(QColor("#5B82A6"));
        p.drawEllipse(QPointF(50, 38), 18, 18);
        QPainterPath buste;
        buste.moveTo(16, 100);
        buste.cubicTo(16, 64, 84, 64, 84, 100);
        buste.closeSubpath();
        p.drawPath(buste);
        break;
    }
    case Logo: {
        QPainterPath ecu;
        ecu.moveTo(50, 6);
        ecu.lineTo(88, 20);
        ecu.lineTo(88, 52);
        ecu.cubicTo(88, 76, 66, 90, 50, 96);
        ecu.cubicTo(34, 90, 12, 76, 12, 52);
        ecu.lineTo(12, 20);
        ecu.closeSubpath();
        p.setPen(QPen(QColor("#E9B240"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(QColor("#1B3346"));
        p.drawPath(ecu);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#208A9A"));
        p.drawRect(QRectF(28, 56, 10, 22));
        p.setBrush(QColor("#E9B240"));
        p.drawRect(QRectF(45, 42, 10, 36));
        p.setBrush(QColor("#5FB3C0"));
        p.drawRect(QRectF(62, 50, 10, 28));
        break;
    }
    case GraphiqueLigne: {
        p.drawLine(QPointF(16, 84), QPointF(84, 84));
        p.drawLine(QPointF(16, 84), QPointF(16, 16));
        QPainterPath line;
        line.moveTo(24, 68);
        line.lineTo(44, 46);
        line.lineTo(62, 56);
        line.lineTo(80, 26);
        p.drawPath(line);
        break;
    }
    case GraphiqueCamembert: {
        p.drawEllipse(QPointF(50, 50), 36, 36);
        p.drawLine(QPointF(50, 50), QPointF(50, 14));
        p.drawLine(QPointF(50, 50), QPointF(80, 68));
        p.drawLine(QPointF(50, 50), QPointF(20, 64));
        break;
    }
    case GraphiqueBarres: {
        p.drawLine(QPointF(14, 86), QPointF(86, 86));
        p.drawRect(QRectF(22, 54, 12, 32));
        p.drawRect(QRectF(42, 34, 12, 52));
        p.drawRect(QRectF(62, 20, 12, 66));
        break;
    }
    case Calendrier: {
        p.drawRoundedRect(QRectF(14, 20, 72, 68), 8, 8);
        p.drawLine(QPointF(14, 40), QPointF(86, 40));
        p.drawLine(QPointF(32, 12), QPointF(32, 24));
        p.drawLine(QPointF(68, 12), QPointF(68, 24));
        p.setBrush(couleur);
        p.drawEllipse(QPointF(32, 56), 3, 3);
        p.drawEllipse(QPointF(50, 56), 3, 3);
        p.drawEllipse(QPointF(68, 56), 3, 3);
        p.drawEllipse(QPointF(32, 72), 3, 3);
        p.drawEllipse(QPointF(50, 72), 3, 3);
        break;
    }
    case Filtre: {
        QPainterPath f;
        f.moveTo(16, 20);
        f.lineTo(84, 20);
        f.lineTo(58, 52);
        f.lineTo(58, 80);
        f.lineTo(42, 88);
        f.lineTo(42, 52);
        f.closeSubpath();
        p.drawPath(f);
        break;
    }
    case DocumentPdf: {
        QPainterPath doc;
        doc.moveTo(22, 14);
        doc.lineTo(60, 14);
        doc.lineTo(78, 32);
        doc.lineTo(78, 86);
        doc.lineTo(22, 86);
        doc.closeSubpath();
        p.drawPath(doc);
        p.drawLine(QPointF(60, 14), QPointF(60, 32));
        p.drawLine(QPointF(60, 32), QPointF(78, 32));
        break;
    }
    case Email: {
        p.drawRoundedRect(QRectF(14, 24, 72, 52), 6, 6);
        QPainterPath env;
        env.moveTo(16, 26);
        env.lineTo(50, 54);
        env.lineTo(84, 26);
        p.drawPath(env);
        break;
    }
    case Telephone: {
        QPainterPath tel;
        tel.moveTo(34, 16);
        tel.lineTo(46, 16);
        tel.lineTo(50, 34);
        tel.lineTo(40, 42);
        tel.cubicTo(46, 54, 54, 62, 66, 68);
        tel.lineTo(74, 58);
        tel.lineTo(90, 64);
        tel.lineTo(90, 76);
        tel.cubicTo(90, 88, 22, 20, 34, 16);
        p.drawPath(tel);
        break;
    }
    case Localisation: {
        QPainterPath pin;
        pin.moveTo(50, 88);
        pin.cubicTo(26, 60, 24, 38, 24, 34);
        pin.arcTo(QRectF(24, 12, 52, 52), 180, -180);
        pin.cubicTo(76, 38, 74, 60, 50, 88);
        p.drawPath(pin);
        p.drawEllipse(QPointF(50, 38), 9, 9);
        break;
    }
    case Mallette: {
        p.drawRoundedRect(QRectF(14, 34, 72, 52), 8, 8);
        QPainterPath poignee;
        poignee.moveTo(36, 34);
        poignee.lineTo(36, 20);
        poignee.lineTo(64, 20);
        poignee.lineTo(64, 34);
        p.drawPath(poignee);
        p.drawLine(QPointF(14, 56), QPointF(86, 56));
        break;
    }
    case Plus: {
        p.drawLine(QPointF(50, 20), QPointF(50, 80));
        p.drawLine(QPointF(20, 50), QPointF(80, 50));
        break;
    }
    case Rafraichir: {
        const QRectF cadre(20, 20, 60, 60);
        p.drawArc(cadre, 30 * 16, 280 * 16);
        QPainterPath fleche;
        fleche.moveTo(76, 26);
        fleche.lineTo(84, 46);
        fleche.lineTo(64, 44);
        p.drawPath(fleche);
        break;
    }
    case Parametres: {
        p.drawEllipse(QPointF(50, 50), 16, 16);
        for (int i = 0; i < 8; ++i) {
            p.save();
            p.translate(50, 50);
            p.rotate(i * 45);
            p.drawLine(QPointF(0, -32), QPointF(0, -44));
            p.restore();
        }
        break;
    }
    case Rapports: {
        p.drawRoundedRect(QRectF(18, 16, 64, 68), 6, 6);
        p.drawLine(QPointF(30, 34), QPointF(70, 34));
        p.drawLine(QPointF(30, 50), QPointF(70, 50));
        p.drawLine(QPointF(30, 66), QPointF(54, 66));
        break;
    }
    case Services: {
        p.drawRoundedRect(QRectF(16, 16, 30, 30), 6, 6);
        p.drawRoundedRect(QRectF(54, 16, 30, 30), 6, 6);
        p.drawRoundedRect(QRectF(16, 54, 30, 30), 6, 6);
        p.drawRoundedRect(QRectF(54, 54, 30, 30), 6, 6);
        break;
    }
    case Horloge: {
        p.drawEllipse(QPointF(50, 50), 38, 38);
        p.drawLine(QPointF(50, 50), QPointF(50, 26));
        p.drawLine(QPointF(50, 50), QPointF(68, 50));
        break;
    }
    }

    p.end();
    return pixmap;
}

inline QIcon icone(Type type, const QColor &couleur, int taille = 96)
{
    return QIcon(dessiner(type, couleur, taille));
}

} // namespace Icones

#endif // ICONES_H