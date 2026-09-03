# GestionFormation

Application de bureau développée en **C++ / Qt** pour la gestion d'un centre de formation, réalisée dans le cadre du projet C++ 2A — ESPRIT (session de repassage, septembre 2026).

L'application couvre deux modules : **Formateurs** et **Cours**, avec une base de données **Oracle**.

## Fonctionnalités

### Module Formateurs
- CRUD complet (ajout, modification, suppression, affichage)
- Recherche et tri multicritères (mot-clé, spécialité, statut, colonne de tri, ordre)
- Statistiques avec graphiques (répartition par spécialité, par statut) mises à jour automatiquement
- Génération de rapport PDF personnalisé (en-têtes, pagination, mise en forme — généré via `QPainter`/`QPrinter`)
- Envoi d'un rappel par email au formateur concernant son prochain cours assigné (EmailJS)

### Module Cours
- CRUD complet (ajout, modification, suppression, affichage)
- Recherche et tri multicritères (mot-clé, niveau, statut, colonne de tri, ordre)
- Statistiques avec graphiques (répartition par niveau, par statut) mises à jour automatiquement
- Génération de rapport PDF personnalisé
- Assistant IA (chatbot) répondant à des questions sur les cours, basé sur l'API Groq

## Architecture

- Connexion à la base de données via un **singleton** (`Connection`), utilisé dans toute l'application
- Toutes les requêtes SQL sont encapsulées dans les classes métier (`Formateur`, `Cours`) — jamais directement dans les slots de l'interface
- Requêtes préparées (`QSqlQuery::prepare` + `bindValue`) pour toutes les opérations, contre les injections SQL
- Interface organisée avec `QTabWidget` / `QStackedWidget`, sans fenêtres de dialogue (`QDialog`)

## Base de données

SGBD : **Oracle**, connexion via pilote **ODBC**.

Deux tables :

| Table | Description |
|---|---|
| `FORMATEUR` | id, nom, prénom, email (unique), téléphone, spécialité, date d'embauche, statut |
| `COURS` | id, titre, description, durée (h), niveau, dates début/fin, formateur assigné (FK nullable), statut |

La relation `FORMATEUR (0,N) — ASSURE — (0,1) COURS` est portée par `COURS.ID_FORMATEUR`, avec `ON DELETE SET NULL` : un cours peut exister sans formateur assigné.

Contraintes principales :
- `CHECK` sur `formateur.statut` : `Actif` / `Inactif`
- `CHECK` sur `cours.niveau` : `Débutant` / `Intermédiaire` / `Avancé`
- `CHECK` sur `cours.statut` : `Planifié` / `En cours` / `Terminé` / `Annulé`
- `CHECK` sur `cours.duree_heures > 0`
- Identifiants auto-générés par séquence + trigger `BEFORE INSERT`

Modèles complets (MCD / MLD / MPD) disponibles dans [`documentation/`](documentation/).

## Installation

### Prérequis
- Qt 6.7+ (modules : `widgets`, `sql`, `charts`, `printsupport`, `network`)
- Oracle Database + pilote ODBC Oracle configuré
- Compilateur compatible C++17 (MinGW ou MSVC)

### Étapes

1. **Cloner le dépôt**
   ```bash
   git clone https://github.com/WissemHaa/GestionFormation.git
   cd GestionFormation
   ```

2. **Créer le schéma Oracle**
   Exécuter [`documentation/database.sql`](documentation/database.sql) dans SQL Developer ou via SQL*Plus, sur le schéma `FORMATION`.

3. **Configurer la source de données ODBC**
   Créer un DSN nommé `Source_Formation` pointant vers la base Oracle (ODBC Data Source Administrator, onglet System DSN).

   > ⚠️ **Important — encodage** : si des accents (`é`, `è`...) s'affichent mal ou provoquent des erreurs `ORA-02290`, vérifier le charset de la base :
   > ```sql
   > SELECT value FROM nls_database_parameters WHERE parameter = 'NLS_CHARACTERSET';
   > ```
   > puis définir la variable d'environnement système `NLS_LANG` en conséquence, par exemple :
   > ```
   > NLS_LANG=AMERICAN_AMERICA.AL32UTF8
   > ```
   > (redémarrer complètement l'IDE après modification).

4. **Configurer les clés API**
   Copier `apikeys.h.example` en `apikeys.h` et renseigner vos clés EmailJS et Groq :
   ```bash
   cp apikeys.h.example apikeys.h
   ```
   `apikeys.h` est ignoré par Git — ne jamais le committer.

5. **Ouvrir et compiler**
   Ouvrir `GestionFormation.pro` dans Qt Creator, configurer le kit, puis build & run.

## Structure du projet

```text
GestionFormation/
├── connection.cpp / .h       # Singleton de connexion à la base
├── formateur.cpp / .h        # Modèle + CRUD + requêtes métier Formateur
├── cours.cpp / .h            # Modèle + CRUD + requêtes métier Cours
├── emailservice.cpp / .h     # Envoi de notifications par email (EmailJS)
├── groqservice.cpp / .h      # Assistant IA (API Groq)
├── mainwindow.cpp / .h / .ui # Interface graphique
├── main.cpp
├── apikeys.h.example         # Modèle de configuration des clés API
├── GestionFormation.pro
└── documentation/
    ├── MCD.pdf
    ├── MLD.pdf
    ├── MPD.pdf
    └── database.sql
```

## Auteur

Wissem Hannechi — Projet C++ 2A, ESPRIT
