#include "connection.h"

// Ctor : initialise l'attribut db avec le driver ODBC
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

// Accès à l'instance unique (Meyers Singleton)
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    db.setDatabaseName("Source_Formation"); // nom de la source ODBC
    db.setUserName("formation");
    db.setPassword("esprit18");

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

QSqlDatabase Connection::getDatabase() {
    return db;
}