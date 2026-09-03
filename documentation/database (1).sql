--------------------------------------------------------
--  GestionFormation - database.sql
--  Schema: FORMATION (Oracle)
--  Encoding: UTF-8
--------------------------------------------------------

--------------------------------------------------------
--  Sequences
--------------------------------------------------------

CREATE SEQUENCE "FORMATION"."COURS_SEQ"
  MINVALUE 1 MAXVALUE 9999999999999999999999999999
  INCREMENT BY 1 START WITH 4
  NOCACHE NOORDER NOCYCLE NOKEEP NOSCALE GLOBAL;

CREATE SEQUENCE "FORMATION"."FORMATEUR_SEQ"
  MINVALUE 1 MAXVALUE 9999999999999999999999999999
  INCREMENT BY 1 START WITH 6
  NOCACHE NOORDER NOCYCLE NOKEEP NOSCALE GLOBAL;

--------------------------------------------------------
--  Table FORMATEUR
--------------------------------------------------------

CREATE TABLE "FORMATION"."FORMATEUR"
(
    "ID_FORMATEUR"  NUMBER,
    "NOM"           VARCHAR2(50 BYTE),
    "PRENOM"        VARCHAR2(50 BYTE),
    "EMAIL"         VARCHAR2(100 BYTE),
    "TELEPHONE"     VARCHAR2(20 BYTE),
    "SPECIALITE"    VARCHAR2(100 BYTE),
    "DATE_EMBAUCHE" DATE DEFAULT SYSDATE,
    "STATUT"        VARCHAR2(20 BYTE) DEFAULT 'Actif'
);

--------------------------------------------------------
--  Table COURS
--------------------------------------------------------

CREATE TABLE "FORMATION"."COURS"
(
    "ID_COURS"      NUMBER,
    "TITRE"         VARCHAR2(100 BYTE),
    "DESCRIPTION"   VARCHAR2(500 BYTE),
    "DUREE_HEURES"  NUMBER,
    "NIVEAU"        VARCHAR2(20 BYTE),
    "DATE_DEBUT"    DATE,
    "DATE_FIN"      DATE,
    "ID_FORMATEUR"  NUMBER,
    "STATUT"        VARCHAR2(20 BYTE) DEFAULT 'Planifié'
);

--------------------------------------------------------
--  Triggers (auto-increment via sequence)
--------------------------------------------------------

CREATE OR REPLACE TRIGGER "FORMATION"."FORMATEUR_TRG"
BEFORE INSERT ON formateur
FOR EACH ROW
WHEN (NEW.id_formateur IS NULL)
BEGIN
    SELECT formateur_seq.NEXTVAL INTO :NEW.id_formateur FROM dual;
END;
/

CREATE OR REPLACE TRIGGER "FORMATION"."COURS_TRG"
BEFORE INSERT ON cours
FOR EACH ROW
WHEN (NEW.id_cours IS NULL)
BEGIN
    SELECT cours_seq.NEXTVAL INTO :NEW.id_cours FROM dual;
END;
/

--------------------------------------------------------
--  Constraints - FORMATEUR
--------------------------------------------------------

ALTER TABLE "FORMATION"."FORMATEUR" ADD PRIMARY KEY ("ID_FORMATEUR");
ALTER TABLE "FORMATION"."FORMATEUR" ADD UNIQUE ("EMAIL");
ALTER TABLE "FORMATION"."FORMATEUR" MODIFY ("NOM" NOT NULL);
ALTER TABLE "FORMATION"."FORMATEUR" MODIFY ("PRENOM" NOT NULL);
ALTER TABLE "FORMATION"."FORMATEUR" MODIFY ("EMAIL" NOT NULL);
ALTER TABLE "FORMATION"."FORMATEUR" ADD CONSTRAINT CK_FORMATEUR_STATUT
    CHECK (statut IN ('Actif', 'Inactif'));

--------------------------------------------------------
--  Constraints - COURS
--------------------------------------------------------

ALTER TABLE "FORMATION"."COURS" ADD PRIMARY KEY ("ID_COURS");
ALTER TABLE "FORMATION"."COURS" MODIFY ("TITRE" NOT NULL);
ALTER TABLE "FORMATION"."COURS" MODIFY ("DUREE_HEURES" NOT NULL);
ALTER TABLE "FORMATION"."COURS" ADD CONSTRAINT CK_COURS_DUREE
    CHECK (duree_heures > 0);
ALTER TABLE "FORMATION"."COURS" ADD CONSTRAINT CK_COURS_NIVEAU
    CHECK (niveau IN ('Débutant', 'Intermédiaire', 'Avancé'));
ALTER TABLE "FORMATION"."COURS" ADD CONSTRAINT CK_COURS_STATUT
    CHECK (statut IN ('Planifié', 'En cours', 'Terminé', 'Annulé'));

--------------------------------------------------------
--  Foreign key - COURS -> FORMATEUR
--------------------------------------------------------

ALTER TABLE "FORMATION"."COURS" ADD CONSTRAINT "FK_COURS_FORMATEUR"
    FOREIGN KEY ("ID_FORMATEUR")
    REFERENCES "FORMATION"."FORMATEUR" ("ID_FORMATEUR")
    ON DELETE SET NULL;

--------------------------------------------------------
--  Sample data
--------------------------------------------------------

SET DEFINE OFF;

INSERT INTO FORMATION.FORMATEUR (ID_FORMATEUR, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, DATE_EMBAUCHE, STATUT)
VALUES (1, 'hannechi', 'wissem', 'wissemhanachi666@gmail.com', '20123456', 'Développement', TO_DATE('27-AUG-26','DD-MON-RR'), 'Actif');

INSERT INTO FORMATION.FORMATEUR (ID_FORMATEUR, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, DATE_EMBAUCHE, STATUT)
VALUES (3, 'amin', 'zomita', 'aminzomita56@gmail.com', '2200068', 'Développement', TO_DATE('27-AUG-26','DD-MON-RR'), 'Inactif');

INSERT INTO FORMATION.FORMATEUR (ID_FORMATEUR, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, DATE_EMBAUCHE, STATUT)
VALUES (4, 'isaa', 'hzemi', 'hzemi@esprit.tn', '90800198', 'physique', TO_DATE('28-AUG-26','DD-MON-RR'), 'Actif');

INSERT INTO FORMATION.FORMATEUR (ID_FORMATEUR, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, DATE_EMBAUCHE, STATUT)
VALUES (5, 'yahya', 'zomita', 'yahya@esprit.tn', '54888036', 'mecanique', TO_DATE('30-AUG-26','DD-MON-RR'), 'Actif');

INSERT INTO FORMATION.COURS (ID_COURS, TITRE, DESCRIPTION, DUREE_HEURES, NIVEAU, DATE_DEBUT, DATE_FIN, ID_FORMATEUR, STATUT)
VALUES (1, 'Introduction à Qt', 'Apprendre les bases du framework Qt en C++', 20, 'Débutant', TO_DATE('01-SEP-26','DD-MON-RR'), TO_DATE('15-SEP-26','DD-MON-RR'), 4, 'Planifié');

INSERT INTO FORMATION.COURS (ID_COURS, TITRE, DESCRIPTION, DUREE_HEURES, NIVEAU, DATE_DEBUT, DATE_FIN, ID_FORMATEUR, STATUT)
VALUES (2, 'introduction a java script', 'welcome to java cours', 30, 'Débutant', TO_DATE('30-AUG-26','DD-MON-RR'), TO_DATE('09-OCT-26','DD-MON-RR'), 3, 'Planifié');

INSERT INTO FORMATION.COURS (ID_COURS, TITRE, DESCRIPTION, DUREE_HEURES, NIVEAU, DATE_DEBUT, DATE_FIN, ID_FORMATEUR, STATUT)
VALUES (3, 'intro a mecanique d''avion', 'mecanique d''avion', 48, 'Intermédiaire', TO_DATE('30-AUG-26','DD-MON-RR'), TO_DATE('26-SEP-26','DD-MON-RR'), 1, 'En cours');

COMMIT;
