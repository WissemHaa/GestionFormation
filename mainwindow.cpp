#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formateur.h"
#include "cours.h"

#include "emailservice.h"

#include "groqservice.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDate>
#include <QColor>
#include <QHeaderView>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QTimer>
#include <QPair>
#include <QScrollBar>
#include <algorithm>

#include <QSpinBox>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , idSelectionne(-1)
    , idCoursSelectionne(-1)
{
    ui->setupUi(this);

    chargerCombos();
    chargerTableComplete();
    rafraichirStatistiques();
    viderFormulaire();

    chargerCombosCours();
    chargerTableCoursComplete();
    rafraichirStatistiquesCours();
    viderFormulaireCours();

    rafraichirDashboard();

    ui->sidebarList->setCurrentRow(0); // affiche le tableau de bord au démarrage
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sidebarList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);

    if (currentRow == 0)
        rafraichirDashboard();
}

// ---------------------------------------------------------
// Chargement des données
// ---------------------------------------------------------

void MainWindow::chargerTable(const QList<Formateur> &liste)
{
    ui->tableFormateurs->setRowCount(0);
    ui->tableFormateurs->setRowCount(liste.size());

    for (int row = 0; row < liste.size(); ++row) {
        const Formateur &f = liste.at(row);

        auto *itemId = new QTableWidgetItem(QString::number(f.getIdFormateur()));
        itemId->setData(Qt::UserRole, f.getIdFormateur());

        ui->tableFormateurs->setItem(row, 0, itemId);
        ui->tableFormateurs->setItem(row, 1, new QTableWidgetItem(f.getNom()));
        ui->tableFormateurs->setItem(row, 2, new QTableWidgetItem(f.getPrenom()));
        ui->tableFormateurs->setItem(row, 3, new QTableWidgetItem(f.getEmail()));
        ui->tableFormateurs->setItem(row, 4, new QTableWidgetItem(f.getSpecialite()));
        ui->tableFormateurs->setItem(row, 5, new QTableWidgetItem(f.getTelephone()));

        auto *itemStatut = new QTableWidgetItem(f.getStatut());
        if (f.getStatut() == "Inactif")
            itemStatut->setForeground(Qt::red);
        else
            itemStatut->setForeground(QColor(0, 128, 0));
        ui->tableFormateurs->setItem(row, 6, itemStatut);
    }

    ui->tableFormateurs->resizeColumnsToContents();
    ui->labelTotal->setText(QString("Total : %1 formateurs").arg(liste.size()));
}

void MainWindow::chargerTableComplete()
{
    chargerTable(Formateur::afficherTous());
}

void MainWindow::chargerCombos()
{
    // Combo de filtre : Spécialité
    ui->comboSpecialite->clear();
    ui->comboSpecialite->addItem("-- Toutes --", "");
    const QStringList specialites = Formateur::listeSpecialites();
    for (const QString &s : specialites)
        ui->comboSpecialite->addItem(s, s);

    // Combo de filtre : Statut
    ui->comboStatut->clear();
    ui->comboStatut->addItem("-- Tous --", "");
    ui->comboStatut->addItem("Actif", "Actif");
    ui->comboStatut->addItem("Inactif", "Inactif");

    // Combo de tri
    ui->comboTri->clear();
    ui->comboTri->addItem("Nom", "nom");
    ui->comboTri->addItem("Prénom", "prenom");
    ui->comboTri->addItem("Email", "email");
    ui->comboTri->addItem("Spécialité", "specialite");
    ui->comboTri->addItem("Date d'embauche", "date_embauche");
    ui->comboTri->addItem("Statut", "statut");

    // Combo du formulaire : Spécialité (éditable, valeurs existantes + saisie libre)
    QString specialiteActuelle = ui->comboSpecialiteForm->currentText();
    ui->comboSpecialiteForm->clear();
    ui->comboSpecialiteForm->addItems(specialites);
    ui->comboSpecialiteForm->setCurrentText(specialiteActuelle);
}

void MainWindow::rafraichirStatistiques()
{
    // --- Bar chart : nombre de formateurs par spécialité ---
    QMap<QString, int> statsSpec = Formateur::statsParSpecialite();

    QBarSet *barSet = new QBarSet("Formateurs");
    QStringList categories;
    for (auto it = statsSpec.constBegin(); it != statsSpec.constEnd(); ++it) {
        *barSet << it.value();
        categories << it.key();
    }

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(barSet);

    QChart *chartSpec = new QChart();
    chartSpec->addSeries(barSeries);
    chartSpec->setTitle("Formateurs par spécialité");
    chartSpec->legend()->hide();

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartSpec->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chartSpec->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    if (chartViewSpecialite) {
        ui->widgetChartSpecialite->layout()->removeWidget(chartViewSpecialite);
        delete chartViewSpecialite;
    }
    chartViewSpecialite = new QChartView(chartSpec);
    chartViewSpecialite->setRenderHint(QPainter::Antialiasing);
    ui->widgetChartSpecialite->layout()->addWidget(chartViewSpecialite);

    // --- Pie chart : Actif vs Inactif ---
    QMap<QString, int> statsStatut = Formateur::statsParStatut();

    QPieSeries *pieSeries = new QPieSeries();
    for (auto it = statsStatut.constBegin(); it != statsStatut.constEnd(); ++it) {
        QPieSlice *slice = pieSeries->append(QString("%1 (%2)").arg(it.key()).arg(it.value()), it.value());
        if (it.key() == "Actif")
            slice->setColor(QColor(0, 150, 0));
        else
            slice->setColor(QColor(200, 0, 0));
    }
    pieSeries->setLabelsVisible(true);

    QChart *chartStatut = new QChart();
    chartStatut->addSeries(pieSeries);
    chartStatut->setTitle("Répartition par statut");

    if (chartViewStatut) {
        ui->widgetChartStatut->layout()->removeWidget(chartViewStatut);
        delete chartViewStatut;
    }
    chartViewStatut = new QChartView(chartStatut);
    chartViewStatut->setRenderHint(QPainter::Antialiasing);
    ui->widgetChartStatut->layout()->addWidget(chartViewStatut);
}


void MainWindow::chargerTableCours(const QList<Cours> &liste)
{
    ui->tableCours->setRowCount(0);
    ui->tableCours->setRowCount(liste.size());

    for (int row = 0; row < liste.size(); ++row) {
        const Cours &c = liste.at(row);

        auto *itemId = new QTableWidgetItem(QString::number(c.getIdCours()));
        itemId->setData(Qt::UserRole, c.getIdCours());

        ui->tableCours->setItem(row, 0, itemId);
        ui->tableCours->setItem(row, 1, new QTableWidgetItem(c.getTitre()));
        ui->tableCours->setItem(row, 2, new QTableWidgetItem(QString::number(c.getDureeHeures())));
        ui->tableCours->setItem(row, 3, new QTableWidgetItem(c.getNiveau()));
        ui->tableCours->setItem(row, 4, new QTableWidgetItem(
                                            c.getDateDebut().isValid() ? c.getDateDebut().toString("dd/MM/yyyy") : ""));
        ui->tableCours->setItem(row, 5, new QTableWidgetItem(
                                            c.getDateFin().isValid() ? c.getDateFin().toString("dd/MM/yyyy") : ""));
        ui->tableCours->setItem(row, 6, new QTableWidgetItem(
                                            c.getIdFormateur() > 0 ? c.getNomFormateur() : "-- Non assigné --"));

        auto *itemStatut = new QTableWidgetItem(c.getStatut());
        if (c.getStatut() == "Annulé")
            itemStatut->setForeground(Qt::red);
        else if (c.getStatut() == "Terminé")
            itemStatut->setForeground(Qt::darkGray);
        else if (c.getStatut() == "En cours")
            itemStatut->setForeground(QColor(0, 120, 200));
        else
            itemStatut->setForeground(QColor(0, 128, 0));
        ui->tableCours->setItem(row, 7, itemStatut);
    }

    ui->tableCours->resizeColumnsToContents();
    ui->labelTotalCours->setText(QString("Total : %1 cours").arg(liste.size()));
}

void MainWindow::chargerTableCoursComplete()
{
    chargerTableCours(Cours::afficherTous());
}

void MainWindow::chargerCombosCours()
{
    // Filtre : Niveau
    ui->comboNiveauCours->clear();
    ui->comboNiveauCours->addItem("-- Tous --", "");
    ui->comboNiveauCours->addItem("Débutant", "Débutant");
    ui->comboNiveauCours->addItem("Intermédiaire", "Intermédiaire");
    ui->comboNiveauCours->addItem("Avancé", "Avancé");

    // Filtre : Statut
    ui->comboStatutCours->clear();
    ui->comboStatutCours->addItem("-- Tous --", "");
    ui->comboStatutCours->addItem("Planifié", "Planifié");
    ui->comboStatutCours->addItem("En cours", "En cours");
    ui->comboStatutCours->addItem("Terminé", "Terminé");
    ui->comboStatutCours->addItem("Annulé", "Annulé");

    // Tri
    ui->comboTriCours->clear();
    ui->comboTriCours->addItem("Titre", "titre");
    ui->comboTriCours->addItem("Niveau", "niveau");
    ui->comboTriCours->addItem("Durée", "duree_heures");
    ui->comboTriCours->addItem("Date début", "date_debut");
    ui->comboTriCours->addItem("Date fin", "date_fin");
    ui->comboTriCours->addItem("Statut", "statut");

    // Combo formateur assigné (formulaire)
    int idActuel = ui->comboFormateurCours->currentData().toInt();
    ui->comboFormateurCours->clear();
    ui->comboFormateurCours->addItem("-- Non assigné --", -1);
    const auto formateurs = Formateur::listeIdNom();
    for (const auto &paire : formateurs)
        ui->comboFormateurCours->addItem(paire.second, paire.first);

    int index = ui->comboFormateurCours->findData(idActuel);
    if (index >= 0)
        ui->comboFormateurCours->setCurrentIndex(index);
}


// ---------------------------------------------------------
// Formulaire Cours
// ---------------------------------------------------------

void MainWindow::viderFormulaireCours()
{
    ui->lineEditTitreCours->clear();
    ui->lineEditDescriptionCours->clear();
    ui->spinDureeCours->setValue(10);
    ui->comboNiveauFormCours->setCurrentIndex(0);
    ui->dateEditDebutCours->setDate(QDate::currentDate());
    ui->dateEditFinCours->setDate(QDate::currentDate().addMonths(1));
    ui->comboFormateurCours->setCurrentIndex(0); // -- Non assigné --
    ui->comboStatutFormCours->setCurrentIndex(0); // Planifié
    idCoursSelectionne = -1;
    ui->tableCours->clearSelection();
}

void MainWindow::remplirFormulaireCoursDepuisSelection()
{
    int row = ui->tableCours->currentRow();
    if (row < 0) return;

    idCoursSelectionne = ui->tableCours->item(row, 0)->data(Qt::UserRole).toInt();
    ui->lineEditTitreCours->setText(ui->tableCours->item(row, 1)->text());
    ui->spinDureeCours->setValue(ui->tableCours->item(row, 2)->text().toInt());
    ui->comboNiveauFormCours->setCurrentText(ui->tableCours->item(row, 3)->text());

    QString dateDebutTexte = ui->tableCours->item(row, 4)->text();
    if (!dateDebutTexte.isEmpty())
        ui->dateEditDebutCours->setDate(QDate::fromString(dateDebutTexte, "dd/MM/yyyy"));

    QString dateFinTexte = ui->tableCours->item(row, 5)->text();
    if (!dateFinTexte.isEmpty())
        ui->dateEditFinCours->setDate(QDate::fromString(dateFinTexte, "dd/MM/yyyy"));

    QString formateurTexte = ui->tableCours->item(row, 6)->text();
    if (formateurTexte == "-- Non assigné --")
        ui->comboFormateurCours->setCurrentIndex(0);
    else
        ui->comboFormateurCours->setCurrentText(formateurTexte);

    ui->comboStatutFormCours->setCurrentText(ui->tableCours->item(row, 7)->text());

    // La description n'est pas affichée dans la table, on la recharge depuis la BD
    const auto liste = Cours::afficherTous();
    for (const auto &c : liste) {
        if (c.getIdCours() == idCoursSelectionne) {
            ui->lineEditDescriptionCours->setText(c.getDescription());
            break;
        }
    }
}

bool MainWindow::validerFormulaireCours()
{
    if (ui->lineEditTitreCours->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ obligatoire", "Veuillez saisir un titre pour le cours.");
        return false;
    }

    if (ui->spinDureeCours->value() <= 0) {
        QMessageBox::warning(this, "Durée invalide", "La durée doit être supérieure à 0.");
        return false;
    }

    if (ui->dateEditFinCours->date() < ui->dateEditDebutCours->date()) {
        QMessageBox::warning(this, "Dates invalides",
                             "La date de fin ne peut pas être antérieure à la date de début.");
        return false;
    }

    return true;
}

// ---------------------------------------------------------
// Slots - boutons de la liste Cours
// ---------------------------------------------------------

void MainWindow::on_btnAjouterCours_clicked()
{
    viderFormulaireCours();
    ui->lineEditTitreCours->setFocus();
}

void MainWindow::on_btnModifierCours_clicked()
{
    if (ui->tableCours->currentRow() < 0) {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un cours dans la liste.");
        return;
    }
    remplirFormulaireCoursDepuisSelection();
    ui->lineEditTitreCours->setFocus();
}

void MainWindow::on_btnSupprimerCours_clicked()
{
    int row = ui->tableCours->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un cours à supprimer.");
        return;
    }

    int id = ui->tableCours->item(row, 0)->data(Qt::UserRole).toInt();
    QString titre = ui->tableCours->item(row, 1)->text();

    auto reponse = QMessageBox::question(this, "Confirmer la suppression",
                                         QString("Supprimer le cours \"%1\" ?").arg(titre),
                                         QMessageBox::Yes | QMessageBox::No);

    if (reponse == QMessageBox::Yes) {
        Cours c;
        c.setIdCours(id);
        if (c.supprimer()) {
            QMessageBox::information(this, "Succès", "Cours supprimé.");
            chargerCombosCours();
            chargerTableCoursComplete();
            rafraichirStatistiquesCours();
            rafraichirDashboard();
            viderFormulaireCours();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
        }
    }
}

void MainWindow::on_btnActualiserCours_clicked()
{
    chargerCombosCours();
    chargerTableCoursComplete();
    viderFormulaireCours();
}

// ---------------------------------------------------------
// Slots - formulaire Cours
// ---------------------------------------------------------

void MainWindow::on_btnEnregistrerCours_clicked()
{
    if (!validerFormulaireCours())
        return;

    Cours c;
    c.setTitre(ui->lineEditTitreCours->text().trimmed());
    c.setDescription(ui->lineEditDescriptionCours->text().trimmed());
    c.setDureeHeures(ui->spinDureeCours->value());
    c.setNiveau(ui->comboNiveauFormCours->currentText());
    c.setDateDebut(ui->dateEditDebutCours->date());
    c.setDateFin(ui->dateEditFinCours->date());
    c.setIdFormateur(ui->comboFormateurCours->currentData().toInt());
    c.setStatut(ui->comboStatutFormCours->currentText());

    bool succes;
    if (idCoursSelectionne == -1) {
        succes = c.ajouter();
    } else {
        c.setIdCours(idCoursSelectionne);
        succes = c.modifier();
    }

    if (succes) {
        QMessageBox::information(this, "Succès", "Cours enregistré avec succès.");
        chargerCombosCours();
        chargerTableCoursComplete();
        rafraichirStatistiquesCours();
        rafraichirDashboard();

        // Notification automatique si un formateur est assigné
        int idFormateurAssigne = c.getIdFormateur();
        if (idFormateurAssigne > 0) {
            Formateur f = Formateur::getParId(idFormateurAssigne);
            if (!f.getEmail().isEmpty()) {
                EmailService::envoyerNotificationAssignation(
                    f.getEmail(),
                    f.getNom() + " " + f.getPrenom(),
                    c.getTitre(),
                    c.getDateDebut().isValid() ? c.getDateDebut().toString("dd/MM/yyyy") : "Non définie",
                    c.getDateFin().isValid() ? c.getDateFin().toString("dd/MM/yyyy") : "Non définie"
                    // pas de parent ici -> pas de popup, envoi silencieux en arrière-plan
                    );
            }
        }

        viderFormulaireCours();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'enregistrement du cours.");
    }
}

void MainWindow::on_btnReinitialiserCours_clicked()
{
    viderFormulaireCours();
}

void MainWindow::on_btnAnnulerCours_clicked()
{
    viderFormulaireCours();
}

// ---------------------------------------------------------
// Slots - filtre / recherche / tri Cours
// ---------------------------------------------------------

void MainWindow::on_btnFiltrerCours_clicked()
{
    QString motCle = ui->lineEditRechercheCours->text().trimmed();
    QString niveau = ui->comboNiveauCours->currentData().toString();
    QString statut = ui->comboStatutCours->currentData().toString();
    QString colonneTri = ui->comboTriCours->currentData().toString();
    bool ordreAscendant = (ui->comboOrdreCours->currentIndex() == 0);

    QList<Cours> resultats = Cours::rechercher(motCle, niveau, statut, colonneTri, ordreAscendant);
    chargerTableCours(resultats);
}

// ---------------------------------------------------------
// Slots - sélection dans le tableau Cours
// ---------------------------------------------------------

void MainWindow::on_tableCours_itemSelectionChanged()
{
    auto lignesSelectionnees = ui->tableCours->selectionModel()->selectedRows();
    if (lignesSelectionnees.size() == 1)
        remplirFormulaireCoursDepuisSelection();
}


// slot notification formatuers
void MainWindow::on_btnEnvoyerRappel_clicked()
{
    int row = ui->tableFormateurs->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un formateur dans la liste.");
        return;
    }

    int id = ui->tableFormateurs->item(row, 0)->data(Qt::UserRole).toInt();
    Formateur f = Formateur::getParId(id);

    if (f.getEmail().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Impossible de récupérer les informations du formateur.");
        return;
    }

    QList<Cours> coursAssignes = Cours::listerParFormateur(id);
    if (coursAssignes.isEmpty()) {
        QMessageBox::information(this, "Aucun cours",
                                 "Ce formateur n'a aucun cours assigné actuellement.");
        return;
    }

    // On envoie un rappel concernant le prochain cours (ou le premier de la liste)
    const Cours &c = coursAssignes.first();

    auto reponse = QMessageBox::question(this, "Confirmer l'envoi",
                                         QString("Envoyer un rappel à %1 %2 concernant le cours \"%3\" ?")
                                             .arg(f.getNom(), f.getPrenom(), c.getTitre()),
                                         QMessageBox::Yes | QMessageBox::No);

    if (reponse == QMessageBox::Yes) {
        EmailService::envoyerNotificationAssignation(
            f.getEmail(),
            f.getNom() + " " + f.getPrenom(),
            c.getTitre(),
            c.getDateDebut().isValid() ? c.getDateDebut().toString("dd/MM/yyyy") : "Non définie",
            c.getDateFin().isValid() ? c.getDateFin().toString("dd/MM/yyyy") : "Non définie",
            this
            );
    }
}

// ---------------------------------------------------------
// Formulaire
// ---------------------------------------------------------

void MainWindow::viderFormulaire()
{
    ui->lineEditNom->clear();
    ui->lineEditPrenom->clear();
    ui->lineEditEmail->clear();
    ui->lineEditTelephone->clear();
    ui->comboSpecialiteForm->setCurrentIndex(-1);
    ui->comboSpecialiteForm->setCurrentText("");
    ui->dateEditEmbauche->setDate(QDate::currentDate());
    ui->comboStatutForm->setCurrentIndex(0); // Actif
    idSelectionne = -1;
    ui->tableFormateurs->clearSelection();
}

void MainWindow::remplirFormulaireDepuisSelection()
{
    int row = ui->tableFormateurs->currentRow();
    if (row < 0) return;

    idSelectionne = ui->tableFormateurs->item(row, 0)->data(Qt::UserRole).toInt();
    ui->lineEditNom->setText(ui->tableFormateurs->item(row, 1)->text());
    ui->lineEditPrenom->setText(ui->tableFormateurs->item(row, 2)->text());
    ui->lineEditEmail->setText(ui->tableFormateurs->item(row, 3)->text());
    ui->comboSpecialiteForm->setCurrentText(ui->tableFormateurs->item(row, 4)->text());
    ui->lineEditTelephone->setText(ui->tableFormateurs->item(row, 5)->text());
    ui->comboStatutForm->setCurrentText(ui->tableFormateurs->item(row, 6)->text());
    ui->dateEditEmbauche->setDate(QDate::currentDate()); // simple, la date d'origine reste inchangée en BD
}

bool MainWindow::validerFormulaire()
{
    if (ui->lineEditNom->text().trimmed().isEmpty() ||
        ui->lineEditPrenom->text().trimmed().isEmpty() ||
        ui->lineEditEmail->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires",
                             "Veuillez remplir les champs Nom, Prénom et Email.");
        return false;
    }

    QString email = ui->lineEditEmail->text().trimmed();
    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Email invalide",
                             "Veuillez saisir une adresse email valide.");
        return false;
    }

    return true;
}

// ---------------------------------------------------------
// Slots - boutons de la liste
// ---------------------------------------------------------

void MainWindow::on_btnAjouter_clicked()
{
    viderFormulaire();
    ui->lineEditNom->setFocus();
}

void MainWindow::on_btnModifier_clicked()
{
    if (ui->tableFormateurs->currentRow() < 0) {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un formateur dans la liste.");
        return;
    }
    remplirFormulaireDepuisSelection();
    ui->lineEditNom->setFocus();
}

void MainWindow::on_btnSupprimer_clicked()
{
    int row = ui->tableFormateurs->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un formateur à supprimer.");
        return;
    }

    int id = ui->tableFormateurs->item(row, 0)->data(Qt::UserRole).toInt();
    QString nom = ui->tableFormateurs->item(row, 1)->text();

    auto reponse = QMessageBox::question(this, "Confirmer la suppression",
                                         QString("Supprimer le formateur \"%1\" ?").arg(nom),
                                         QMessageBox::Yes | QMessageBox::No);

    if (reponse == QMessageBox::Yes) {
        Formateur f;
        f.setIdFormateur(id);
        if (f.supprimer()) {
            QMessageBox::information(this, "Succès", "Formateur supprimé.");
            chargerCombos();
            chargerTableComplete();
            rafraichirDashboard();
            rafraichirStatistiques();
            viderFormulaire();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
        }
    }
}

void MainWindow::on_btnGenererPDF_clicked()
{
    if (ui->tableFormateurs->rowCount() == 0) {
        QMessageBox::information(this, "Aucune donnée",
                                 "La liste est vide, rien à exporter.");
        return;
    }

    auto lignesSelectionnees = ui->tableFormateurs->selectionModel()->selectedRows();

    if (lignesSelectionnees.isEmpty()) {
        auto reponse = QMessageBox::question(this, "Aucune sélection",
                                             "Aucun formateur sélectionné.\nVoulez-vous exporter TOUS les formateurs affichés ?",
                                             QMessageBox::Yes | QMessageBox::No);
        if (reponse != QMessageBox::Yes)
            return;
    } else {
        QString message = QString("Exporter les %1 formateur(s) sélectionné(s) ?").arg(lignesSelectionnees.size());
        auto reponse = QMessageBox::question(this, "Confirmer l'export", message,
                                             QMessageBox::Yes | QMessageBox::No);
        if (reponse != QMessageBox::Yes)
            return;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          + "/rapport_formateurs.pdf";

    QString chemin = QFileDialog::getSaveFileName(this, "Enregistrer le rapport PDF",
                                                  defaultPath, "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty())
        return;

    genererRapportPDF(chemin);

    QMessageBox::information(this, "Succès",
                             "Le rapport PDF a été généré avec succès.");
}

void MainWindow::genererRapportPDF(const QString &cheminFichier)
{
    // Détermine quelles lignes exporter : sélection actuelle, ou tout si rien n'est sélectionné
    QList<int> lignesAExporter;
    auto lignesSelectionnees = ui->tableFormateurs->selectionModel()->selectedRows();
    if (!lignesSelectionnees.isEmpty()) {
        for (const QModelIndex &index : lignesSelectionnees)
            lignesAExporter.append(index.row());
        std::sort(lignesAExporter.begin(), lignesAExporter.end());
    } else {
        for (int r = 0; r < ui->tableFormateurs->rowCount(); ++r)
            lignesAExporter.append(r);
    }
    int totalAExporter = lignesAExporter.size();

    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(150); // résolution fixe et prévisible
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(cheminFichier);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;
    if (!painter.begin(&printer))
        return;

    // Facteur d'échelle : nos constantes ci-dessous sont conçues pour 72 DPI
    double echelle = printer.resolution() / 72.0;
    auto S = [echelle](double valeur) { return static_cast<int>(valeur * echelle); };

    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int marge = S(20);
    int y = pageRect.top() + marge;
    int pageNumero = 1;

    int largeurPage = pageRect.width() - 2 * marge;
    QVector<double> proportions = {0.06, 0.16, 0.16, 0.24, 0.16, 0.12, 0.10};
    QVector<int> largeurs;
    for (double p : proportions)
        largeurs.append(static_cast<int>(largeurPage * p));

    QStringList entetes = {"ID", "Nom", "Prénom", "Email", "Spécialité", "Téléphone", "Statut"};

    auto dessinerEntetePage = [&]() {
        QFont titreFont("Arial", 16, QFont::Bold);
        painter.setFont(titreFont);
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(30),
                         Qt::AlignLeft, "Rapport des Formateurs - Centre de Formation");
        y += S(35);

        QFont sousTitreFont("Arial", 9);
        painter.setFont(sousTitreFont);
        painter.setPen(Qt::darkGray);
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(20), Qt::AlignLeft,
                         "Généré le : " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(20), Qt::AlignRight,
                         QString("Total : %1 formateur(s)").arg(totalAExporter));
        painter.setPen(Qt::black);
        y += S(30);

        int x = pageRect.left() + marge;
        QFont enteteFont("Arial", 10, QFont::Bold);
        painter.setFont(enteteFont);
        painter.setBrush(QColor(220, 230, 245));
        painter.drawRect(x, y, largeurPage, S(30));
        for (int col = 0; col < entetes.size(); ++col) {
            painter.drawRect(x, y, largeurs[col], S(30));
            painter.drawText(x + S(5), y, largeurs[col] - S(10), S(30),
                             Qt::AlignVCenter | Qt::AlignLeft, entetes[col]);
            x += largeurs[col];
        }
        y += S(30);
        painter.setBrush(Qt::NoBrush);
    };

    dessinerEntetePage();

    QFont ligneFont("Arial", 9);
    painter.setFont(ligneFont);
    int hauteurLigne = S(26);

    int compteur = 0;
    for (int row : lignesAExporter) {
        if (y + hauteurLigne > pageRect.bottom() - marge - S(30)) {
            painter.setFont(QFont("Arial", 8));
            painter.drawText(pageRect.left(), pageRect.bottom() - S(20), pageRect.width(), S(20),
                             Qt::AlignCenter, QString("Page %1").arg(pageNumero));

            printer.newPage();
            pageNumero++;
            y = pageRect.top() + marge;
            dessinerEntetePage();
            painter.setFont(ligneFont);
        }

        int x = pageRect.left() + marge;
        if (compteur % 2 == 0) {
            painter.setBrush(QColor(245, 245, 245));
            painter.drawRect(x, y, largeurPage, hauteurLigne);
            painter.setBrush(Qt::NoBrush);
        }

        for (int col = 0; col < entetes.size(); ++col) {
            QString texte = ui->tableFormateurs->item(row, col) ?
                                ui->tableFormateurs->item(row, col)->text() : "";
            painter.drawRect(x, y, largeurs[col], hauteurLigne);
            painter.drawText(x + S(5), y, largeurs[col] - S(10), hauteurLigne,
                             Qt::AlignVCenter | Qt::AlignLeft, texte);
            x += largeurs[col];
        }
        y += hauteurLigne;
        compteur++;
    }

    painter.setFont(QFont("Arial", 8));
    painter.drawText(pageRect.left(), pageRect.bottom() - S(20), pageRect.width(), S(20),
                     Qt::AlignCenter, QString("Page %1").arg(pageNumero));

    painter.end();
}


void MainWindow::on_btnActualiser_clicked()
{
    chargerCombos();
    chargerTableComplete();
    viderFormulaire();
}



QString MainWindow::construireContexteCours()
{
    QString contexte;

    QList<Cours> tousCours = Cours::afficherTous();
    contexte += QString("Nombre total de cours : %1\n").arg(tousCours.size());

    QMap<QString, int> parNiveau = Cours::statsParNiveau();
    contexte += "Répartition par niveau : ";
    for (auto it = parNiveau.constBegin(); it != parNiveau.constEnd(); ++it)
        contexte += QString("%1=%2 ").arg(it.key()).arg(it.value());
    contexte += "\n";

    QMap<QString, int> parStatut = Cours::statsParStatut();
    contexte += "Répartition par statut : ";
    for (auto it = parStatut.constBegin(); it != parStatut.constEnd(); ++it)
        contexte += QString("%1=%2 ").arg(it.key()).arg(it.value());
    contexte += "\n\n";

    contexte += "Liste détaillée des cours :\n";
    for (const auto &c : tousCours) {
        contexte += QString("- [ID %1] \"%2\" | Niveau: %3 | Durée: %4h | Statut: %5 | "
                            "Début: %6 | Fin: %7 | Formateur: %8\n")
                        .arg(c.getIdCours())
                        .arg(c.getTitre())
                        .arg(c.getNiveau())
                        .arg(c.getDureeHeures())
                        .arg(c.getStatut())
                        .arg(c.getDateDebut().isValid() ? c.getDateDebut().toString("dd/MM/yyyy") : "N/A")
                        .arg(c.getDateFin().isValid() ? c.getDateFin().toString("dd/MM/yyyy") : "N/A")
                        .arg(c.getIdFormateur() > 0 ? c.getNomFormateur() : "Non assigné");
    }

    contexte += "\nListe des formateurs :\n";
    QList<Formateur> tousFormateurs = Formateur::afficherTous();
    for (const auto &f : tousFormateurs) {
        contexte += QString("- %1 %2 | Spécialité: %3 | Statut: %4\n")
                        .arg(f.getNom())
                        .arg(f.getPrenom())
                        .arg(f.getSpecialite())
                        .arg(f.getStatut());
    }

    return contexte;
}

void MainWindow::afficherMessageChat(const QString &auteur, const QString &texte)
{
    historiqueChatCours.append(qMakePair(auteur, texte));
    rafraichirChatCours();
}

void MainWindow::rafraichirChatCours()
{
    QString html;

    for (const auto &message : historiqueChatCours) {
        const QString &auteur = message.first;
        const QString &texte = message.second;
        bool estUtilisateur = (auteur == "Vous");

        QString contenu = texte.toHtmlEscaped().replace("\n", "<br>");
        QString bulleCouleur = estUtilisateur ? "#2d3138" : "#eef0f2";
        QString texteCouleur = estUtilisateur ? "#ffffff" : "#2c2f36";
        QString alignement = estUtilisateur ? "right" : "left";
        QString spacerGauche = estUtilisateur ? "<td width=\"18%\"></td>" : "";
        QString spacerDroite = estUtilisateur ? "" : "<td width=\"18%\"></td>";

        html += QString(
                    "<table width='100%' cellspacing='0' cellpadding='0' style='margin-bottom:4px;'><tr>"
                    "%1"
                    "<td align='%2'>"
                    "<div style='color:#8a8f98; font-size:10px; margin-bottom:2px;'>%3</div>"
                    "<table cellspacing='0' cellpadding='9' bgcolor='%4'><tr><td>"
                    "<font color='%5'>%6</font>"
                    "</td></tr></table>"
                    "</td>"
                    "%7"
                    "</tr></table>"
                    ).arg(spacerGauche, alignement, auteur, bulleCouleur, texteCouleur, contenu, spacerDroite);
    }

    if (reflexionEnCoursActive) {
        QString points = QString(".").repeated(pointsReflexionCours);
        html += QString(
                    "<table width='100%' cellspacing='0' cellpadding='0'><tr>"
                    "<td align='left'>"
                    "<div style='color:#8a8f98; font-size:10px; margin-bottom:2px;'>Assistant</div>"
                    "<table cellspacing='0' cellpadding='9' bgcolor='#f3f4f6'><tr><td>"
                    "<font color='#8a8f98'><i>Réflexion en cours%1</i></font>"
                    "</td></tr></table>"
                    "</td>"
                    "<td width='18%'></td>"
                    "</tr></table>"
                    ).arg(points);
    }

    ui->textEditChatCours->setHtml(html);
    ui->textEditChatCours->verticalScrollBar()->setValue(ui->textEditChatCours->verticalScrollBar()->maximum());
}

void MainWindow::definirIndicateurReflexionCours(bool actif)
{
    reflexionEnCoursActive = actif;

    if (actif) {
        if (!timerReflexionCours) {
            timerReflexionCours = new QTimer(this);
            connect(timerReflexionCours, &QTimer::timeout, this, [this]() {
                pointsReflexionCours = (pointsReflexionCours % 3) + 1;
                rafraichirChatCours();
            });
        }
        pointsReflexionCours = 1;
        timerReflexionCours->start(450);
    } else if (timerReflexionCours) {
        timerReflexionCours->stop();
    }

    rafraichirChatCours();
}

void MainWindow::on_btnEnvoyerQuestionCours_clicked()
{
    QString question = ui->lineEditQuestionCours->text().trimmed();
    if (question.isEmpty())
        return;

    afficherMessageChat("Vous", question);
    ui->lineEditQuestionCours->clear();
    ui->btnEnvoyerQuestionCours->setEnabled(false);

    definirIndicateurReflexionCours(true);

    QString contexte = construireContexteCours();

    GroqService::poserQuestion(question, contexte, [this](const QString &reponse) {
        definirIndicateurReflexionCours(false);
        afficherMessageChat("Assistant", reponse);
        ui->btnEnvoyerQuestionCours->setEnabled(true);
    });
}


// ---------------------------------------------------------
// Statistiques Cours
// ---------------------------------------------------------

void MainWindow::rafraichirStatistiquesCours()
{
    // --- Bar chart : nombre de cours par niveau ---
    QMap<QString, int> statsNiveau = Cours::statsParNiveau();

    QBarSet *barSet = new QBarSet("Cours");
    QStringList categories;
    for (auto it = statsNiveau.constBegin(); it != statsNiveau.constEnd(); ++it) {
        *barSet << it.value();
        categories << it.key();
    }

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(barSet);

    QChart *chartNiveau = new QChart();
    chartNiveau->addSeries(barSeries);
    chartNiveau->setTitle("Cours par niveau");
    chartNiveau->legend()->hide();

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartNiveau->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->applyNiceNumbers();
    chartNiveau->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    if (chartViewNiveauCours) {
        ui->widgetChartNiveauCours->layout()->removeWidget(chartViewNiveauCours);
        delete chartViewNiveauCours;
    }
    chartViewNiveauCours = new QChartView(chartNiveau);
    chartViewNiveauCours->setRenderHint(QPainter::Antialiasing);
    ui->widgetChartNiveauCours->layout()->addWidget(chartViewNiveauCours);

    // --- Pie chart : répartition par statut ---
    QMap<QString, int> statsStatut = Cours::statsParStatut();

    QPieSeries *pieSeries = new QPieSeries();
    for (auto it = statsStatut.constBegin(); it != statsStatut.constEnd(); ++it) {
        QPieSlice *slice = pieSeries->append(QString("%1 (%2)").arg(it.key()).arg(it.value()), it.value());
        if (it.key() == "Terminé")
            slice->setColor(QColor(0, 150, 0));
        else if (it.key() == "En cours")
            slice->setColor(QColor(0, 120, 200));
        else if (it.key() == "Annulé")
            slice->setColor(QColor(200, 0, 0));
        else
            slice->setColor(QColor(230, 180, 0)); // Planifié
    }
    pieSeries->setLabelsVisible(true);

    QChart *chartStatut = new QChart();
    chartStatut->addSeries(pieSeries);
    chartStatut->setTitle("Répartition des cours par statut");

    if (chartViewStatutCours) {
        ui->widgetChartStatutCours->layout()->removeWidget(chartViewStatutCours);
        delete chartViewStatutCours;
    }
    chartViewStatutCours = new QChartView(chartStatut);
    chartViewStatutCours->setRenderHint(QPainter::Antialiasing);
    ui->widgetChartStatutCours->layout()->addWidget(chartViewStatutCours);
}

void MainWindow::on_btnActualiserStatsCours_clicked()
{
    rafraichirStatistiquesCours();
}

// ---------------------------------------------------------
// Tableau de bord
// ---------------------------------------------------------

void MainWindow::rafraichirDashboard()
{
    QList<Formateur> formateurs = Formateur::afficherTous();
    QList<Cours> coursList = Cours::afficherTous();

    // ---- Carte Formateurs ----
    int totalFormateurs = formateurs.size();
    QMap<QString, int> statsFormateurStatut = Formateur::statsParStatut();
    int actifs = statsFormateurStatut.value("Actif", 0);
    int inactifs = statsFormateurStatut.value("Inactif", 0);

    ui->labelTotalFormateursDashboard->setText(QString::number(totalFormateurs));
    ui->labelTotalFormateursSousTitre->setText(
        QString("%1 actif(s) · %2 inactif(s)").arg(actifs).arg(inactifs));

    // ---- Carte Cours ----
    int totalCours = coursList.size();
    QMap<QString, int> statsCoursStatut = Cours::statsParStatut();
    int enCours = statsCoursStatut.value("En cours", 0);
    int planifies = statsCoursStatut.value("Planifié", 0);

    ui->labelTotalCoursDashboard->setText(QString::number(totalCours));
    ui->labelTotalCoursSousTitre->setText(
        QString("%1 en cours · %2 planifié(s)").arg(enCours).arg(planifies));

    // ---- Carte Cours non assignés (indicateur d'alerte) ----
    int nonAssignes = 0;
    for (const Cours &c : coursList) {
        if (c.getIdFormateur() <= 0)
            nonAssignes++;
    }

    ui->labelNonAssignesDashboard->setText(QString::number(nonAssignes));
    if (nonAssignes > 0) {
        ui->labelNonAssignesDashboard->setStyleSheet(
            "font-size: 30px; font-weight: 700; color: #c0392b;");
        ui->labelNonAssignesSousTitre->setText("nécessitent un formateur");
    } else {
        ui->labelNonAssignesDashboard->setStyleSheet(
            "font-size: 30px; font-weight: 700; color: #1f2229;");
        ui->labelNonAssignesSousTitre->setText("tous les cours sont assignés");
    }

    // ---- Carte Formateur le plus sollicité ----
    QMap<QString, int> chargeParFormateur;
    for (const Cours &c : coursList) {
        if (c.getIdFormateur() > 0 && !c.getNomFormateur().isEmpty())
            chargeParFormateur[c.getNomFormateur()]++;
    }

    if (chargeParFormateur.isEmpty()) {
        ui->labelTopFormateurNom->setText("—");
        ui->labelTopFormateurDetail->setText("aucun cours assigné");
    } else {
        QString meilleurNom;
        int meilleurNombre = 0;
        for (auto it = chargeParFormateur.constBegin(); it != chargeParFormateur.constEnd(); ++it) {
            if (it.value() > meilleurNombre) {
                meilleurNombre = it.value();
                meilleurNom = it.key();
            }
        }
        ui->labelTopFormateurNom->setText(meilleurNom);
        ui->labelTopFormateurDetail->setText(
            QString("%1 cours assigné(s)").arg(meilleurNombre));
    }

    // ---- Tableau "Prochains cours" ----
    QList<Cours> prochains;
    QDate aujourdHui = QDate::currentDate();
    for (const Cours &c : coursList) {
        if (c.getDateDebut().isValid() && c.getDateDebut() >= aujourdHui)
            prochains.append(c);
    }
    std::sort(prochains.begin(), prochains.end(), [](const Cours &a, const Cours &b) {
        return a.getDateDebut() < b.getDateDebut();
    });
    if (prochains.size() > 5)
        prochains = prochains.mid(0, 5);

    ui->tableProchainsCours->verticalHeader()->setVisible(false);
    ui->tableProchainsCours->setRowCount(prochains.size());
    for (int i = 0; i < prochains.size(); ++i) {
        const Cours &c = prochains.at(i);
        ui->tableProchainsCours->setItem(i, 0, new QTableWidgetItem(c.getTitre()));
        ui->tableProchainsCours->setItem(i, 1, new QTableWidgetItem(c.getDateDebut().toString("dd/MM/yyyy")));
        QString nomF = c.getNomFormateur().isEmpty() ? "Non assigné" : c.getNomFormateur();
        ui->tableProchainsCours->setItem(i, 2, new QTableWidgetItem(nomF));
    }
    if (prochains.isEmpty()) {
        ui->tableProchainsCours->setRowCount(1);
        auto *item = new QTableWidgetItem("Aucun cours à venir");
        item->setForeground(QColor("#8a8f98"));
        ui->tableProchainsCours->setItem(0, 0, item);
        ui->tableProchainsCours->setItem(0, 1, new QTableWidgetItem(""));
        ui->tableProchainsCours->setItem(0, 2, new QTableWidgetItem(""));
    }
}

// ---------------------------------------------------------
// Génération PDF - Cours
// ---------------------------------------------------------

void MainWindow::on_btnGenererPDFCours_clicked()
{
    if (ui->tableCours->rowCount() == 0) {
        QMessageBox::information(this, "Aucune donnée",
                                 "La liste est vide, rien à exporter.");
        return;
    }

    auto lignesSelectionnees = ui->tableCours->selectionModel()->selectedRows();

    if (lignesSelectionnees.isEmpty()) {
        auto reponse = QMessageBox::question(this, "Aucune sélection",
                                             "Aucun cours sélectionné.\nVoulez-vous exporter TOUS les cours affichés ?",
                                             QMessageBox::Yes | QMessageBox::No);
        if (reponse != QMessageBox::Yes)
            return;
    } else {
        QString message = QString("Exporter les %1 cours sélectionné(s) ?").arg(lignesSelectionnees.size());
        auto reponse = QMessageBox::question(this, "Confirmer l'export", message,
                                             QMessageBox::Yes | QMessageBox::No);
        if (reponse != QMessageBox::Yes)
            return;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          + "/rapport_cours.pdf";

    QString chemin = QFileDialog::getSaveFileName(this, "Enregistrer le rapport PDF",
                                                  defaultPath, "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty())
        return;

    genererRapportPDFCours(chemin);

    QMessageBox::information(this, "Succès",
                             "Le rapport PDF a été généré avec succès.");
}

void MainWindow::genererRapportPDFCours(const QString &cheminFichier)
{
    QList<int> lignesAExporter;
    auto lignesSelectionnees = ui->tableCours->selectionModel()->selectedRows();
    if (!lignesSelectionnees.isEmpty()) {
        for (const QModelIndex &index : lignesSelectionnees)
            lignesAExporter.append(index.row());
        std::sort(lignesAExporter.begin(), lignesAExporter.end());
    } else {
        for (int r = 0; r < ui->tableCours->rowCount(); ++r)
            lignesAExporter.append(r);
    }
    int totalAExporter = lignesAExporter.size();

    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(150);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(cheminFichier);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;
    if (!painter.begin(&printer))
        return;

    double echelle = printer.resolution() / 72.0;
    auto S = [echelle](double valeur) { return static_cast<int>(valeur * echelle); };

    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int marge = S(20);
    int y = pageRect.top() + marge;
    int pageNumero = 1;

    int largeurPage = pageRect.width() - 2 * marge;
    QVector<double> proportions = {0.06, 0.20, 0.10, 0.14, 0.11, 0.11, 0.18, 0.10};
    QVector<int> largeurs;
    for (double p : proportions)
        largeurs.append(static_cast<int>(largeurPage * p));

    QStringList entetes = {"ID", "Titre", "Durée", "Niveau", "Début", "Fin", "Formateur", "Statut"};

    auto dessinerEntetePage = [&]() {
        QFont titreFont("Arial", 16, QFont::Bold);
        painter.setFont(titreFont);
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(30),
                         Qt::AlignLeft, "Rapport des Cours - Centre de Formation");
        y += S(35);

        QFont sousTitreFont("Arial", 9);
        painter.setFont(sousTitreFont);
        painter.setPen(Qt::darkGray);
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(20), Qt::AlignLeft,
                         "Généré le : " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
        painter.drawText(pageRect.left() + marge, y, largeurPage, S(20), Qt::AlignRight,
                         QString("Total : %1 cours").arg(totalAExporter));
        painter.setPen(Qt::black);
        y += S(30);

        int x = pageRect.left() + marge;
        QFont enteteFont("Arial", 10, QFont::Bold);
        painter.setFont(enteteFont);
        painter.setBrush(QColor(220, 230, 245));
        painter.drawRect(x, y, largeurPage, S(30));
        for (int col = 0; col < entetes.size(); ++col) {
            painter.drawRect(x, y, largeurs[col], S(30));
            painter.drawText(x + S(5), y, largeurs[col] - S(10), S(30),
                             Qt::AlignVCenter | Qt::AlignLeft, entetes[col]);
            x += largeurs[col];
        }
        y += S(30);
        painter.setBrush(Qt::NoBrush);
    };

    dessinerEntetePage();

    QFont ligneFont("Arial", 9);
    painter.setFont(ligneFont);
    int hauteurLigne = S(26);

    int compteur = 0;
    for (int row : lignesAExporter) {
        if (y + hauteurLigne > pageRect.bottom() - marge - S(30)) {
            painter.setFont(QFont("Arial", 8));
            painter.drawText(pageRect.left(), pageRect.bottom() - S(20), pageRect.width(), S(20),
                             Qt::AlignCenter, QString("Page %1").arg(pageNumero));

            printer.newPage();
            pageNumero++;
            y = pageRect.top() + marge;
            dessinerEntetePage();
            painter.setFont(ligneFont);
        }

        int x = pageRect.left() + marge;
        if (compteur % 2 == 0) {
            painter.setBrush(QColor(245, 245, 245));
            painter.drawRect(x, y, largeurPage, hauteurLigne);
            painter.setBrush(Qt::NoBrush);
        }

        for (int col = 0; col < entetes.size(); ++col) {
            QString texte = ui->tableCours->item(row, col) ?
                                ui->tableCours->item(row, col)->text() : "";
            painter.drawRect(x, y, largeurs[col], hauteurLigne);
            painter.drawText(x + S(5), y, largeurs[col] - S(10), hauteurLigne,
                             Qt::AlignVCenter | Qt::AlignLeft, texte);
            x += largeurs[col];
        }
        y += hauteurLigne;
        compteur++;
    }

    painter.setFont(QFont("Arial", 8));
    painter.drawText(pageRect.left(), pageRect.bottom() - S(20), pageRect.width(), S(20),
                     Qt::AlignCenter, QString("Page %1").arg(pageNumero));

    painter.end();
}

// ---------------------------------------------------------
// Slot charts
// ---------------------------------------------------------

void MainWindow::on_btnActualiserStats_clicked()
{
    rafraichirStatistiques();
}

// ---------------------------------------------------------
// Slots - formulaire
// ---------------------------------------------------------

void MainWindow::on_btnEnregistrer_clicked()
{
    if (!validerFormulaire())
        return;

    Formateur f;
    f.setNom(ui->lineEditNom->text().trimmed());
    f.setPrenom(ui->lineEditPrenom->text().trimmed());
    f.setEmail(ui->lineEditEmail->text().trimmed());
    f.setTelephone(ui->lineEditTelephone->text().trimmed());
    f.setSpecialite(ui->comboSpecialiteForm->currentText().trimmed());
    f.setDateEmbauche(ui->dateEditEmbauche->date());
    f.setStatut(ui->comboStatutForm->currentText());

    bool succes;
    if (idSelectionne == -1) {
        succes = f.ajouter(); // nouveau formateur
    } else {
        f.setIdFormateur(idSelectionne);
        succes = f.modifier(); // mise à jour
    }

    if (succes) {
        QMessageBox::information(this, "Succès", "Formateur enregistré avec succès.");
        chargerCombos();
        chargerTableComplete();
        rafraichirDashboard();
        rafraichirStatistiques();
        viderFormulaire();
    } else {
        QMessageBox::critical(this, "Erreur",
                              "Échec de l'enregistrement. Vérifiez que l'email n'est pas déjà utilisé.");
    }
}

void MainWindow::on_btnReinitialiser_clicked()
{
    viderFormulaire();
}

void MainWindow::on_btnAnnuler_clicked()
{
    viderFormulaire();
}

// ---------------------------------------------------------
// Slots - filtre / recherche / tri
// ---------------------------------------------------------

void MainWindow::on_btnFiltrer_clicked()
{
    QString motCle = ui->lineEditRecherche->text().trimmed();
    QString specialite = ui->comboSpecialite->currentData().toString();
    QString statut = ui->comboStatut->currentData().toString();
    QString colonneTri = ui->comboTri->currentData().toString();
    bool ordreAscendant = (ui->comboOrdre->currentIndex() == 0); // 0 = Croissant

    QList<Formateur> resultats = Formateur::rechercher(motCle, specialite, statut, colonneTri, ordreAscendant);
    chargerTable(resultats);
}

// ---------------------------------------------------------
// Slots - sélection dans le tableau
// ---------------------------------------------------------

void MainWindow::on_tableFormateurs_itemSelectionChanged()
{
    auto lignesSelectionnees = ui->tableFormateurs->selectionModel()->selectedRows();
    if (lignesSelectionnees.size() == 1)
        remplirFormulaireDepuisSelection();
}