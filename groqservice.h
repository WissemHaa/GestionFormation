#ifndef GROQSERVICE_H
#define GROQSERVICE_H

#include <QString>
#include <functional>

class GroqService {
public:
    // Envoie une question à l'IA avec un contexte (résumé des données Cours/Formateur),
    // et appelle callback(reponse) une fois la réponse reçue (ou un message d'erreur).
    static void poserQuestion(const QString &question,
                              const QString &contexte,
                              std::function<void(const QString&)> callback);
};

#endif // GROQSERVICE_H