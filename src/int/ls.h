#ifndef LS_HEADER
#define LS_HEADER

/* Déclaration des fonctions externes */
ssize_t readlink(const char *path, char *buf, size_t bufsiz);
int lstat(const char *path, struct stat *buf);
int snprintf (char *str, size_t size, const char *format, ...);
/*************************************/

int test_couleurs(void);

/* Affiche le type du fichier (option -F) */
void affiche_type (mode_t mode);

/* Affiche les droits du fichier */
void affiche_droits (mode_t mode);

/* Affiche le nom du propriétaire du fichier */
void affiche_user (uid_t uid);

/* Affiche le nom du groupe du fichier */
void affiche_groupe (gid_t gid);

/* Affiche la date de modification */
void affiche_date_modif (time_t date_modif);

/* permet de lire la cible d'un lien symbolique */
char * read_the_link (char * filename);

/* affiche la taille d'un fichier */
void affiche_taille (off_t st_size);

/* fonction d'affichage d'un fichier sur une ligne (option -l) */
void affiche_fichier (char * nom, char * path, struct stat infos);

/* fonction de parcours d'un répertoire et affiche son contenu */
void parcours_rep (char * directory);

#endif
