#include "cours.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

Cours::Cours() : idCours(0), dureeHeures(0), idFormateur(-1) {}

Cours::Cours(int id, const QString &titre, const QString &description, int duree,
             const QString &niveau, const QDate &debut, const QDate &fin,
             int idFormateur, const QString &nomFormateur, const QString &statut)
    : idCours(id), titre(titre), description(description), dureeHeures(duree),
    niveau(niveau), dateDebut(debut), dateFin(fin), idFormateur(idFormateur),
    nomFormateur(nomFormateur), statut(statut) {}

// ---- Getters ----
int Cours::getIdCours() const { return idCours; }
QString Cours::getTitre() const { return titre; }
QString Cours::getDescription() const { return description; }
int Cours::getDureeHeures() const { return dureeHeures; }
QString Cours::getNiveau() const { return niveau; }
QDate Cours::getDateDebut() const { return dateDebut; }
QDate Cours::getDateFin() const { return dateFin; }
int Cours::getIdFormateur() const { return idFormateur; }
QString Cours::getNomFormateur() const { return nomFormateur; }
QString Cours::getStatut() const { return statut; }

// ---- Setters ----
void Cours::setIdCours(int id) { idCours = id; }
void Cours::setTitre(const QString &t) { titre = t; }
void Cours::setDescription(const QString &d) { description = d; }
void Cours::setDureeHeures(int d) { dureeHeures = d; }
void Cours::setNiveau(const QString &n) { niveau = n; }
void Cours::setDateDebut(const QDate &d) { dateDebut = d; }
void Cours::setDateFin(const QDate &d) { dateFin = d; }
void Cours::setIdFormateur(int id) { idFormateur = id; }
void Cours::setStatut(const QString &s) { statut = s; }

// ---- CRUD ----

bool Cours::ajouter() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("INSERT INTO cours (titre, description, duree_heures, niveau, "
                  "date_debut, date_fin, id_formateur, statut) "
                  "VALUES (:titre, :description, :duree, :niveau, :debut, :fin, :idFormateur, :statut)");
    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":duree", dureeHeures);
    query.bindValue(":niveau", niveau);
    query.bindValue(":debut", dateDebut.isValid() ? QVariant(dateDebut) : QVariant());
    query.bindValue(":fin", dateFin.isValid() ? QVariant(dateFin) : QVariant());
    query.bindValue(":idFormateur", idFormateur > 0 ? QVariant(idFormateur) : QVariant());
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        qDebug() << "Erreur ajout cours :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Cours::modifier() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("UPDATE cours SET titre = :titre, description = :description, "
                  "duree_heures = :duree, niveau = :niveau, date_debut = :debut, "
                  "date_fin = :fin, id_formateur = :idFormateur, statut = :statut "
                  "WHERE id_cours = :id");
    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":duree", dureeHeures);
    query.bindValue(":niveau", niveau);
    query.bindValue(":debut", dateDebut.isValid() ? QVariant(dateDebut) : QVariant());
    query.bindValue(":fin", dateFin.isValid() ? QVariant(dateFin) : QVariant());
    query.bindValue(":idFormateur", idFormateur > 0 ? QVariant(idFormateur) : QVariant());
    query.bindValue(":statut", statut);
    query.bindValue(":id", idCours);

    if (!query.exec()) {
        qDebug() << "Erreur modification cours :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Cours::supprimer() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("DELETE FROM cours WHERE id_cours = :id");
    query.bindValue(":id", idCours);

    if (!query.exec()) {
        qDebug() << "Erreur suppression cours :" << query.lastError().text();
        return false;
    }
    return true;
}

QList<Cours> Cours::afficherTous() {
    QList<Cours> liste;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT c.id_cours, c.titre, c.description, c.duree_heures, c.niveau, "
                  "c.date_debut, c.date_fin, c.id_formateur, "
                  "f.nom || ' ' || f.prenom AS nom_formateur, c.statut "
                  "FROM cours c LEFT JOIN formateur f ON c.id_formateur = f.id_formateur "
                  "ORDER BY c.id_cours");

    if (!query.exec()) {
        qDebug() << "Erreur affichage cours :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        Cours c(
            query.value("id_cours").toInt(),
            query.value("titre").toString(),
            query.value("description").toString(),
            query.value("duree_heures").toInt(),
            query.value("niveau").toString(),
            query.value("date_debut").toDate(),
            query.value("date_fin").toDate(),
            query.value("id_formateur").isNull() ? -1 : query.value("id_formateur").toInt(),
            query.value("nom_formateur").toString(),
            query.value("statut").toString()
            );
        liste.append(c);
    }
    return liste;
}

QList<Cours> Cours::rechercher(const QString &motCle, const QString &niveau,
                               const QString &statut, const QString &colonneTri,
                               bool ordreAscendant) {
    QList<Cours> liste;

    QString mc = motCle.trimmed();
    QString niv = niveau.trimmed();
    QString stat = statut.trimmed();

    QString colonne = "titre";
    if (colonneTri == "titre" || colonneTri == "niveau" || colonneTri == "duree_heures" ||
        colonneTri == "date_debut" || colonneTri == "date_fin" || colonneTri == "statut") {
        colonne = colonneTri;
    }
    QString ordre = ordreAscendant ? "ASC" : "DESC";

    QString sql = "SELECT c.id_cours, c.titre, c.description, c.duree_heures, c.niveau, "
                  "c.date_debut, c.date_fin, c.id_formateur, "
                  "f.nom || ' ' || f.prenom AS nom_formateur, c.statut "
                  "FROM cours c LEFT JOIN formateur f ON c.id_formateur = f.id_formateur "
                  "WHERE 1=1";

    if (!mc.isEmpty())
        sql += " AND (LOWER(c.titre) LIKE LOWER(:motcle) OR LOWER(c.description) LIKE LOWER(:motcle))";
    if (!niv.isEmpty())
        sql += " AND c.niveau = :niveau";
    if (!stat.isEmpty())
        sql += " AND c.statut = :statut";

    sql += QString(" ORDER BY c.%1 %2").arg(colonne, ordre);

    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare(sql);

    if (!mc.isEmpty())
        query.bindValue(":motcle", "%" + mc + "%");
    if (!niv.isEmpty())
        query.bindValue(":niveau", niv);
    if (!stat.isEmpty())
        query.bindValue(":statut", stat);

    if (!query.exec()) {
        qDebug() << "Erreur recherche cours :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        Cours c(
            query.value("id_cours").toInt(),
            query.value("titre").toString(),
            query.value("description").toString(),
            query.value("duree_heures").toInt(),
            query.value("niveau").toString(),
            query.value("date_debut").toDate(),
            query.value("date_fin").toDate(),
            query.value("id_formateur").isNull() ? -1 : query.value("id_formateur").toInt(),
            query.value("nom_formateur").toString(),
            query.value("statut").toString()
            );
        liste.append(c);
    }
    return liste;
}

QMap<QString, int> Cours::statsParNiveau() {
    QMap<QString, int> stats;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT niveau, COUNT(*) AS nb FROM cours "
                  "WHERE niveau IS NOT NULL GROUP BY niveau ORDER BY niveau");

    if (!query.exec()) {
        qDebug() << "Erreur stats niveau :" << query.lastError().text();
        return stats;
    }
    while (query.next())
        stats.insert(query.value("niveau").toString(), query.value("nb").toInt());
    return stats;
}

QMap<QString, int> Cours::statsParStatut() {
    QMap<QString, int> stats;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT statut, COUNT(*) AS nb FROM cours GROUP BY statut");

    if (!query.exec()) {
        qDebug() << "Erreur stats statut cours :" << query.lastError().text();
        return stats;
    }
    while (query.next())
        stats.insert(query.value("statut").toString(), query.value("nb").toInt());
    return stats;
}

QList<Cours> Cours::listerParFormateur(int idFormateur) {
    QList<Cours> liste;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT c.id_cours, c.titre, c.description, c.duree_heures, c.niveau, "
                  "c.date_debut, c.date_fin, c.id_formateur, "
                  "f.nom || ' ' || f.prenom AS nom_formateur, c.statut "
                  "FROM cours c LEFT JOIN formateur f ON c.id_formateur = f.id_formateur "
                  "WHERE c.id_formateur = :idFormateur ORDER BY c.date_debut");
    query.bindValue(":idFormateur", idFormateur);

    if (!query.exec()) {
        qDebug() << "Erreur listerParFormateur :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        Cours c(
            query.value("id_cours").toInt(),
            query.value("titre").toString(),
            query.value("description").toString(),
            query.value("duree_heures").toInt(),
            query.value("niveau").toString(),
            query.value("date_debut").toDate(),
            query.value("date_fin").toDate(),
            query.value("id_formateur").isNull() ? -1 : query.value("id_formateur").toInt(),
            query.value("nom_formateur").toString(),
            query.value("statut").toString()
            );
        liste.append(c);
    }
    return liste;
}