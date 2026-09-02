#include "mainwindow.h"
#include "connection.h"
#include "formateur.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

// ---------------------------------------------------------
// Feuille de style globale (QSS) - thème neutre gris/blanc
// ---------------------------------------------------------
static const char *STYLE_APPLICATION = R"(
/* Couleur de texte par défaut forcée explicitement (corrige le texte
   invisible en mode sombre Windows : sans ça, Qt utilise la couleur de
   texte de la palette système - blanche en dark mode - dès qu'un
   QApplication::setStyleSheet() est actif). */
QWidget {
    color: #2c2f36;
}
QLabel, QGroupBox, QCheckBox, QRadioButton, QMenuBar, QMenu, QStatusBar {
    color: #2c2f36;
}
QTableWidget, QTableWidget::item, QHeaderView::section {
    color: #2c2f36;
}
QMessageBox QLabel {
    color: #2c2f36;
}
QMessageBox {
    background-color: #ffffff;
}
QMessageBox QPushButton {
    min-width: 72px;
}

QMainWindow, #centralwidget {
    background-color: #f3f4f6;
}

/* ---- Sidebar ---- */
QListWidget#sidebarList {
    background-color: #ffffff;
    border: none;
    border-right: 1px solid #e3e4e8;
    padding-top: 10px;
    outline: 0;
    font-size: 13px;
}
QListWidget#sidebarList::item {
    padding: 12px 14px;
    margin: 2px 8px;
    border-radius: 6px;
    color: #4b4f58;
    cursor: pointing-hand;
}
QListWidget#sidebarList::item:selected {
    background-color: #2d3138;
    color: #ffffff;
    font-weight: 600;
}
QListWidget#sidebarList::item:hover:!selected {
    background-color: #f0f1f3;
}

/* ---- Pages / conteneurs ---- */
QStackedWidget {
    background-color: #f3f4f6;
}

QLabel#labelDashboardTitre {
    font-size: 20px;
    font-weight: 700;
    color: #1f2229;
    padding-top: 6px;
}
QLabel#labelDashboardSousTitre {
    font-size: 12px;
    color: #8a8f98;
    padding-bottom: 6px;
}

/* ---- Cartes / groupes ---- */
QGroupBox {
    background-color: #ffffff;
    border: 1px solid #e3e4e8;
    border-radius: 10px;
    margin-top: 12px;
    padding: 14px 12px 12px 12px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 6px;
    color: #6b6f76;
    font-size: 11px;
    font-weight: 600;
}

/* ---- Boutons ---- */
QPushButton {
    background-color: #eef0f2;
    border: 1px solid #d8dadd;
    border-radius: 7px;
    padding: 7px 18px;
    color: #333740;
    font-size: 12px;
    font-weight: 500;
    cursor: pointing-hand;
}
QPushButton:hover {
    background-color: #e2e4e8;
    border-color: #c7cad0;
    color: #1f2229;
}
QPushButton:pressed {
    background-color: #d5d8dc;
    border-color: #b9bcc2;
}
QPushButton:disabled {
    color: #b3b6bc;
    background-color: #f7f8f9;
    border-color: #e8e9ec;
}

/* Boutons primaires (enregistrer) */
QPushButton#btnEnregistrer, QPushButton#btnEnregistrerCours {
    background-color: #2d3138;
    border: 1px solid #2d3138;
    color: #ffffff;
    font-weight: 600;
}
QPushButton#btnEnregistrer:hover, QPushButton#btnEnregistrerCours:hover {
    background-color: #40454e;
    border-color: #40454e;
}
QPushButton#btnEnregistrer:pressed, QPushButton#btnEnregistrerCours:pressed {
    background-color: #22252b;
    border-color: #22252b;
}

/* Boutons de suppression (danger) */
QPushButton#btnSupprimer, QPushButton#btnSupprimerCours {
    background-color: #fdf1f0;
    color: #c0392b;
    border: 1px solid #f0c4bd;
    font-weight: 500;
}
QPushButton#btnSupprimer:hover, QPushButton#btnSupprimerCours:hover {
    background-color: #f7d9d5;
    border-color: #e2a099;
    color: #a5301f;
}
QPushButton#btnSupprimer:pressed, QPushButton#btnSupprimerCours:pressed {
    background-color: #f0c3bc;
    border-color: #d98d84;
}

/* ---- Zones de texte multi-lignes (ex. historique de chat) ---- */
QTextEdit, QTextBrowser, QPlainTextEdit {
    background-color: #ffffff;
    color: #2c2f36;
    border: 1px solid #e3e4e8;
    border-radius: 8px;
    padding: 8px;
    selection-background-color: #2d3138;
    selection-color: #ffffff;
}

/* ---- Champs de saisie ---- */
QLineEdit, QComboBox, QDateEdit, QSpinBox {
    background-color: #ffffff;
    border: 1px solid #d6d8dc;
    border-radius: 5px;
    padding: 5px 8px;
    color: #2c2f36;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus {
    border: 1px solid #2d3138;
}
QComboBox::drop-down {
    border: none;
    width: 20px;
}

/* Liste déroulante des combobox (popup séparé - nécessite un ciblage
   explicite, sinon même bug de texte invisible qu'ailleurs en dark mode) */
QComboBox QAbstractItemView {
    background-color: #ffffff;
    color: #2c2f36;
    border: 1px solid #d6d8dc;
    outline: 0;
    selection-background-color: #2d3138;
    selection-color: #ffffff;
    padding: 2px;
}
QComboBox QAbstractItemView::item {
    padding: 6px 8px;
    min-height: 22px;
}
QComboBox QAbstractItemView::item:hover {
    background-color: #f0f1f3;
    color: #1f2229;
}

/* Popup calendrier des QDateEdit (encore un widget séparé, même famille
   de bug que la popup des combobox : fond sombre + texte peu lisible
   par défaut en dark mode Windows). */
QCalendarWidget QWidget {
    background-color: #ffffff;
    color: #2c2f36;
    alternate-background-color: #f7f8f9;
}
QCalendarWidget QAbstractItemView:enabled {
    background-color: #ffffff;
    color: #2c2f36;
    selection-background-color: #2d3138;
    selection-color: #ffffff;
    outline: 0;
}
QCalendarWidget QAbstractItemView:disabled {
    color: #c3c6cb;
}
QCalendarWidget QToolButton {
    background-color: transparent;
    color: #ffffff;
    font-weight: 600;
    icon-size: 16px;
    padding: 4px 8px;
}
QCalendarWidget QToolButton:hover {
    background-color: rgba(255, 255, 255, 40);
    border-radius: 4px;
}
QCalendarWidget QWidget#qt_calendar_navigationbar {
    background-color: #2d3138;
}
QCalendarWidget QMenu {
    background-color: #ffffff;
    color: #2c2f36;
}
QCalendarWidget QSpinBox {
    background-color: #ffffff;
    color: #2c2f36;
    selection-background-color: #2d3138;
    selection-color: #ffffff;
}

/* ---- Tableaux ---- */
QTableWidget {
    background-color: #ffffff;
    border: 1px solid #e3e4e8;
    border-radius: 8px;
    gridline-color: #eceef0;
    selection-background-color: #e4e6ea;
    selection-color: #1f2229;
    alternate-background-color: #fafbfc;
}
QTableWidget::item {
    padding: 4px;
}
QHeaderView::section {
    background-color: #f7f8f9;
    color: #4b4f58;
    padding: 6px;
    border: none;
    border-bottom: 1px solid #e3e4e8;
    font-weight: 600;
}

/* ---- Onglets ---- */
QTabWidget::pane {
    border: 1px solid #e3e4e8;
    border-radius: 8px;
    top: -1px;
    background-color: #ffffff;
}
QTabBar::tab {
    background: transparent;
    padding: 8px 18px;
    color: #6b6f76;
    border-bottom: 2px solid transparent;
    cursor: pointing-hand;
}
QTabBar::tab:selected {
    color: #1f2229;
    font-weight: 600;
    border-bottom: 2px solid #2d3138;
}
QTabBar::tab:hover:!selected {
    color: #2c2f36;
}
)";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(QString::fromUtf8(STYLE_APPLICATION));

    // On récupère l'instance unique du singleton
    Connection& c = Connection::createInstance();

    // On tente d'établir la connexion
    bool test = c.createConnection();

    MainWindow w;

    if (test) {
        w.show();

        // --- Test rapide de la classe Formateur ---
        QList<Formateur> liste = Formateur::afficherTous();
        qDebug() << "Nombre de formateurs trouvés :" << liste.size();
        for (const Formateur &f : liste) {
            qDebug() << f.getIdFormateur() << f.getNom() << f.getPrenom() << f.getSpecialite();
        }
        // --- Fin du test ---

    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion.")
            );
        return -1;
    }

    return a.exec();
}