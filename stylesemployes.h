#ifndef STYLESEMPLOYES_H
#define STYLESEMPLOYES_H

#include <QString>

/*
 * Feuille de style commune aux pages du module Employés.
 * Les graphiques et certains badges sont dessinés par code.
 */
namespace StylesEmployes {

inline QString feuille()
{
    return QStringLiteral(R"(
        QMainWindow, QWidget#pageAccueil, QWidget#pageEmployes,
        QWidget#pageServices, QWidget#pageRapports, QWidget#pageFiche {
            background-color: #F0F4F8;
        }

        QFrame#barreLaterale {
            background-color: #183B4E;
            border: none;
        }
        QFrame#logoCadre {
            background: #FFFFFF;
            border-radius: 12px;
        }
        QLabel#logoNom {
            color: #FFFFFF;
            font-size: 8pt;
            font-weight: 600;
            letter-spacing: 1px;
        }
        QLabel#logoSousNom {
            color: #8FB0C2;
            font-size: 6.5pt;
            letter-spacing: 1px;
        }
        QLabel#versionBase {
            color: #6F8EA1;
            font-size: 7pt;
        }
        QToolButton#boutonMenu {
            background: transparent;
            color: #B9CBD8;
            border: none;
            border-radius: 10px;
            font-size: 8pt;
            padding: 6px 2px;
        }
        QToolButton#boutonMenu:hover {
            background: #224A60;
            color: #FFFFFF;
        }
        QToolButton#boutonMenu:checked {
            background: #179BAE;
            color: #FFFFFF;
            font-weight: 600;
        }

        QLabel#titrePage {
            color: #183B4E;
            font-size: 21pt;
            font-weight: 600;
        }
        QLabel#sousTitrePage {
            color: #6B7C8C;
            font-size: 9pt;
        }
        QLabel#titreSection {
            color: #1B3346;
            font-size: 11pt;
            font-weight: 600;
        }
        QLabel#texteDiscret {
            color: #7A8B99;
            font-size: 8pt;
        }
        QLabel#texteValeur {
            color: #1B3346;
            font-size: 9pt;
        }
        QLabel#texteVide {
            color: #8A9BA8;
            font-size: 9pt;
        }

        QFrame#carte {
            background: #FFFFFF;
            border: 1px solid #E1E8EE;
            border-radius: 12px;
        }
        QFrame#carteDouce {
            background: #F8FBFD;
            border: 1px solid #E1E8EE;
            border-radius: 10px;
        }
        QFrame#encart {
            background: #F8FBFD;
            border: 1px solid #E1E8EE;
            border-radius: 10px;
        }

        QPushButton#btnPrimaire {
            background: #179BAE;
            color: #FFFFFF;
            border: none;
            border-radius: 7px;
            padding: 9px 18px;
            font-weight: 600;
        }
        QPushButton#btnPrimaire:hover { background: #12879A; }
        QPushButton#btnPrimaire:disabled { background: #A9CDD2; }

        QPushButton#btnSecondaire {
            background: #FFFFFF;
            color: #1B3346;
            border: 1px solid #D8E2EA;
            border-radius: 7px;
            padding: 8px 16px;
        }
        QPushButton#btnSecondaire:hover { background: #F4F8FA; border-color: #B9CBD8; }

        QPushButton#btnLien {
            background: transparent;
            color: #179BAE;
            border: none;
            padding: 6px 4px;
            font-weight: 600;
        }
        QPushButton#btnLien:hover { color: #0F7180; text-decoration: underline; }

        QPushButton#btnAction {
            background: #E7F4F6;
            color: #137A6E;
            border: 1px solid #C9E8EA;
            border-radius: 6px;
            padding: 7px 14px;
            font-weight: 600;
        }
        QPushButton#btnAction:hover { background: #D8EFF1; }
        QPushButton#btnAction:disabled {
            background: #F1F5F7; color: #A8B6C0; border-color: #E1E8EE;
        }

        QPushButton#btnDanger {
            background: #FDECEA;
            color: #B03A2E;
            border: 1px solid #F5C6C0;
            border-radius: 6px;
            padding: 7px 14px;
            font-weight: 600;
        }
        QPushButton#btnDanger:hover { background: #FADBD8; }
        QPushButton#btnDanger:disabled {
            background: #F7F2F1; color: #C9B6B2; border-color: #EFE1DF;
        }

        QLineEdit#recherche, QComboBox#filtre, QLineEdit#champ {
            background: #FFFFFF;
            border: 1px solid #D8E2EA;
            border-radius: 7px;
            padding: 8px 10px;
            min-height: 20px;
            color: #1B3346;
        }
        QLineEdit#recherche:focus, QComboBox#filtre:focus, QLineEdit#champ:focus {
            border: 1px solid #179BAE;
        }
        QComboBox#filtre::drop-down { border: none; width: 24px; }
        QComboBox#filtre QAbstractItemView {
            background: #FFFFFF;
            border: 1px solid #D8E2EA;
            selection-background-color: #E7F4F6;
            selection-color: #1B3346;
        }

        QTableView#tableEmployes, QTableWidget#tableConges, QTableWidget#tableActivite {
            background: #FFFFFF;
            border: none;
            gridline-color: transparent;
            alternate-background-color: #F8FBFD;
            selection-background-color: #E7F4F6;
            selection-color: #1B3346;
            font-size: 8.5pt;
        }
        QTableView#tableEmployes::item, QTableWidget#tableConges::item,
        QTableWidget#tableActivite::item {
            border-bottom: 1px solid #F0F4F8;
            padding: 4px;
        }
        QHeaderView::section {
            background: #F8FBFD;
            color: #5B6B79;
            font-size: 8pt;
            font-weight: 600;
            border: none;
            border-bottom: 1px solid #E1E8EE;
            padding: 8px 6px;
        }
        QTableCornerButton::section { background: #F8FBFD; border: none; }

        QTabWidget#ongletsFiche::pane {
            border: none;
            background: transparent;
            top: -1px;
        }
        QTabWidget#ongletsFiche QTabBar::tab {
            background: transparent;
            color: #6B7C8C;
            padding: 10px 16px;
            border: none;
            border-bottom: 2px solid transparent;
            font-size: 9pt;
        }
        QTabWidget#ongletsFiche QTabBar::tab:hover { color: #179BAE; }
        QTabWidget#ongletsFiche QTabBar::tab:selected {
            color: #179BAE;
            border-bottom: 2px solid #179BAE;
            font-weight: 600;
        }

        QScrollBar:vertical {
            background: transparent; width: 10px; margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #C9D6DF; border-radius: 4px; min-height: 28px;
        }
        QScrollBar::handle:vertical:hover { background: #A8BCC9; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QScrollBar:horizontal {
            background: transparent; height: 10px; margin: 2px;
        }
        QScrollBar::handle:horizontal {
            background: #C9D6DF; border-radius: 4px; min-width: 28px;
        }
        QScrollBar::handle:horizontal:hover { background: #A8BCC9; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
    )");
}

} // namespace StylesEmployes

#endif // STYLESEMPLOYES_H
