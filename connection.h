#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>

/* Classe Connection
 * Implémentation du pattern Singleton pour gérer une connexion unique à la base de données.
 * Utilise QSqlDatabase pour établir et fermer la connexion.
 * L'instance est accessible via la méthode statique createInstance().
 */

class Connection {
public:
    static Connection& createInstance(); // Retourne la référence unique
    bool createConnection();             // Établit la connexion à la base
    QSqlDatabase getDatabase();          // Retourne l'objet db pour exécuter des requêtes

private:
    QSqlDatabase db;

    Connection();                                    // Constructeur privé
    ~Connection();                                    // Destructeur privé
    Connection(const Connection&) = delete;            // Interdire la copie
    Connection& operator=(const Connection&) = delete;  // Interdire l'affectation
};

#endif // CONNECTION_H