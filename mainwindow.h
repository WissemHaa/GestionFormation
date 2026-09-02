#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cours.h"

#include <QMainWindow>
#include <QPair>

class QTimer;

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>

#include <QPrinter>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_btnAjouter_clicked();
    void on_btnEnregistrer_clicked();
    void on_btnModifier_clicked();
    void on_btnSupprimer_clicked();
    void on_btnActualiser_clicked();
    void on_btnFiltrer_clicked();
    void on_btnReinitialiser_clicked();
    void on_btnAnnuler_clicked();
    void on_tableFormateurs_itemSelectionChanged();
    void on_btnActualiserStats_clicked();
    void on_btnGenererPDF_clicked();

    void on_sidebarList_currentRowChanged(int currentRow);

    void on_btnAjouterCours_clicked();
    void on_btnEnregistrerCours_clicked();
    void on_btnModifierCours_clicked();
    void on_btnSupprimerCours_clicked();
    void on_btnActualiserCours_clicked();
    void on_btnFiltrerCours_clicked();
    void on_btnReinitialiserCours_clicked();
    void on_btnAnnulerCours_clicked();
    void on_btnActualiserStatsCours_clicked();
    void on_btnGenererPDFCours_clicked();
    void on_tableCours_itemSelectionChanged();

    void on_btnEnvoyerRappel_clicked();

    //chabot
    void on_btnEnvoyerQuestionCours_clicked();

private:
    Ui::MainWindow *ui;

    int idSelectionne;    // -1 = aucun formateur sélectionné (mode ajout)

    QChartView *chartViewSpecialite = nullptr;
    QChartView *chartViewStatut = nullptr;

    void chargerTable(const QList<class Formateur> &liste);
    void chargerTableComplete();
    void chargerCombos();
    void viderFormulaire();
    void remplirFormulaireDepuisSelection();
    bool validerFormulaire();
    void rafraichirStatistiques();
    void genererRapportPDF(const QString &cheminFichier);

    int idCoursSelectionne;

    void chargerTableCours(const QList<Cours> &liste);
    void chargerTableCoursComplete();
    void chargerCombosCours();
    void viderFormulaireCours();
    void remplirFormulaireCoursDepuisSelection();
    bool validerFormulaireCours();
    void rafraichirStatistiquesCours();
    void rafraichirDashboard();
    void genererRapportPDFCours(const QString &cheminFichier);

    QChartView *chartViewNiveauCours = nullptr;
    QChartView *chartViewStatutCours = nullptr;

    //chatbot
    QString construireContexteCours();
    void afficherMessageChat(const QString &auteur, const QString &texte);
    void rafraichirChatCours();
    void definirIndicateurReflexionCours(bool actif);

    QList<QPair<QString, QString>> historiqueChatCours; // (auteur, texte brut - échappé au rendu)
    bool reflexionEnCoursActive = false;
    QTimer *timerReflexionCours = nullptr;
    int pointsReflexionCours = 1;


};

#endif // MAINWINDOW_H