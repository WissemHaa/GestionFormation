#include "formateur.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Formateur::Formateur() {}

Formateur::Formateur(int id, const QString &nom, const QString &prenom, const QString &email,
                     const QString &telephone, const QString &specialite,
                     const QDate &dateEmbauche, const QString &statut)
    : idFormateur(id), nom(nom), prenom(prenom), email(email),
    telephone(telephone), specialite(specialite),
    dateEmbauche(dateEmbauche), statut(statut) {}

// ---- Getters ----
int Formateur::getIdFormateur() const { return idFormateur; }
QString Formateur::getNom() const { return nom; }
QString Formateur::getPrenom() const { return prenom; }
QString Formateur::getEmail() const { return email; }
QString Formateur::getTelephone() const { return telephone; }
QString Formateur::getSpecialite() const { return specialite; }
QDate Formateur::getDateEmbauche() const { return dateEmbauche; }
QString Formateur::getStatut() const { return statut; }

// ---- Setters ----
void Formateur::setIdFormateur(int id) { idFormateur = id; }
void Formateur::setNom(const QString &n) { nom = n; }
void Formateur::setPrenom(const QString &p) { prenom = p; }
void Formateur::setEmail(const QString &e) { email = e; }
void Formateur::setTelephone(const QString &t) { telephone = t; }
void Formateur::setSpecialite(const QString &s) { specialite = s; }
void Formateur::setDateEmbauche(const QDate &d) { dateEmbauche = d; }
void Formateur::setStatut(const QString &s) { statut = s; }

// ---- CRUD ----

bool Formateur::ajouter() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("INSERT INTO formateur (nom, prenom, email, telephone, specialite, statut) "
                  "VALUES (:nom, :prenom, :email, :telephone, :specialite, :statut)");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        qDebug() << "Erreur ajout formateur :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Formateur::modifier() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("UPDATE formateur SET nom = :nom, prenom = :prenom, email = :email, "
                  "telephone = :telephone, specialite = :specialite, statut = :statut "
                  "WHERE id_formateur = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":statut", statut);
    query.bindValue(":id", idFormateur);

    if (!query.exec()) {
        qDebug() << "Erreur modification formateur :" << query.lastError().text();
        return false;
    }
    return true;
}

bool Formateur::supprimer() {
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("DELETE FROM formateur WHERE id_formateur = :id");
    query.bindValue(":id", idFormateur);

    if (!query.exec()) {
        qDebug() << "Erreur suppression formateur :" << query.lastError().text();
        return false;
    }
    return true;
}

QList<Formateur> Formateur::afficherTous() {
    QList<Formateur> liste;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT id_formateur, nom, prenom, email, telephone, specialite, date_embauche, statut "
                  "FROM formateur ORDER BY id_formateur");

    if (!query.exec()) {
        qDebug() << "Erreur affichage formateurs :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        Formateur f(
            query.value("id_formateur").toInt(),
            query.value("nom").toString(),
            query.value("prenom").toString(),
            query.value("email").toString(),
            query.value("telephone").toString(),
            query.value("specialite").toString(),
            query.value("date_embauche").toDate(),
            query.value("statut").toString()
            );
        liste.append(f);
    }
    return liste;
}

QList<QPair<int, QString>> Formateur::listeIdNom() {
    QList<QPair<int, QString>> liste;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT id_formateur, nom, prenom FROM formateur ORDER BY nom");

    if (!query.exec()) {
        qDebug() << "Erreur liste id/nom formateurs :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString nomComplet = query.value(1).toString() + " " + query.value(2).toString();
        liste.append(qMakePair(id, nomComplet));
    }
    return liste;
}

QList<Formateur> Formateur::rechercher(const QString &motCle, const QString &specialite,
                                       const QString &statut, const QString &colonneTri,
                                       bool ordreAscendant) {
    QList<Formateur> liste;

    QString mc = motCle.trimmed();
    QString spec = specialite.trimmed();
    QString stat = statut.trimmed();

    // Whitelist pour éviter toute injection via ORDER BY (impossible à "bindValue")
    QString colonne = "nom";
    if (colonneTri == "nom" || colonneTri == "prenom" || colonneTri == "email" ||
        colonneTri == "specialite" || colonneTri == "date_embauche" || colonneTri == "statut") {
        colonne = colonneTri;
    }
    QString ordre = ordreAscendant ? "ASC" : "DESC";

    QString sql = "SELECT id_formateur, nom, prenom, email, telephone, specialite, date_embauche, statut "
                  "FROM formateur WHERE 1=1";

    if (!mc.isEmpty())
        sql += " AND (LOWER(nom) LIKE LOWER(:motcle) OR LOWER(prenom) LIKE LOWER(:motcle) "
               "OR LOWER(email) LIKE LOWER(:motcle) OR LOWER(specialite) LIKE LOWER(:motcle))";
    if (!spec.isEmpty())
        sql += " AND specialite = :specialite";
    if (!stat.isEmpty())
        sql += " AND statut = :statut";

    sql += QString(" ORDER BY %1 %2").arg(colonne, ordre);

    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare(sql);

    if (!mc.isEmpty())
        query.bindValue(":motcle", "%" + mc + "%");
    if (!spec.isEmpty())
        query.bindValue(":specialite", spec);
    if (!stat.isEmpty())
        query.bindValue(":statut", stat);

    if (!query.exec()) {
        qDebug() << "Erreur recherche formateurs :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        Formateur f(
            query.value("id_formateur").toInt(),
            query.value("nom").toString(),
            query.value("prenom").toString(),
            query.value("email").toString(),
            query.value("telephone").toString(),
            query.value("specialite").toString(),
            query.value("date_embauche").toDate(),
            query.value("statut").toString()
            );
        liste.append(f);
    }
    return liste;
}

QStringList Formateur::listeSpecialites() {
    QStringList liste;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT DISTINCT specialite FROM formateur "
                  "WHERE specialite IS NOT NULL ORDER BY specialite");

    if (!query.exec()) {
        qDebug() << "Erreur liste spécialités :" << query.lastError().text();
        return liste;
    }

    while (query.next()) {
        liste.append(query.value(0).toString());
    }
    return liste;
}

QMap<QString, int> Formateur::statsParSpecialite() {
    QMap<QString, int> stats;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT specialite, COUNT(*) AS nb FROM formateur "
                  "WHERE specialite IS NOT NULL GROUP BY specialite ORDER BY specialite");

    if (!query.exec()) {
        qDebug() << "Erreur stats spécialité :" << query.lastError().text();
        return stats;
    }

    while (query.next()) {
        stats.insert(query.value("specialite").toString(), query.value("nb").toInt());
    }
    return stats;
}

QMap<QString, int> Formateur::statsParStatut() {
    QMap<QString, int> stats;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT statut, COUNT(*) AS nb FROM formateur GROUP BY statut");

    if (!query.exec()) {
        qDebug() << "Erreur stats statut :" << query.lastError().text();
        return stats;
    }

    while (query.next()) {
        stats.insert(query.value("statut").toString(), query.value("nb").toInt());
    }
    return stats;
}

Formateur Formateur::getParId(int id) {
    Formateur f;
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("SELECT id_formateur, nom, prenom, email, telephone, specialite, date_embauche, statut "
                  "FROM formateur WHERE id_formateur = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        f = Formateur(
            query.value("id_formateur").toInt(),
            query.value("nom").toString(),
            query.value("prenom").toString(),
            query.value("email").toString(),
            query.value("telephone").toString(),
            query.value("specialite").toString(),
            query.value("date_embauche").toDate(),
            query.value("statut").toString()
            );
    } else {
        qDebug() << "Erreur getParId formateur :" << query.lastError().text();
    }
    return f;
}