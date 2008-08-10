/* 
 * build-in command ls \o/
 *
 * Grand merci à VANBESELAERE Remi et PODGORSEK Paul
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "ls.h"

#define couleurnormale()  printf("\033[0m")


#define OPTION_ALL             1
#define OPTION_RECURSIVE       2
#define OPTION_TRI_INVERSE     4
#define OPTION_LONG            8
#define OPTION_TYPE            16


char option_all=0;
char option_recursive=0;
char option_tri_inverse=0;
char option_long=0;
char option_type=0;
char option_aucune_couleur=0;

char  optstring[] = "aRrlFhn";


char * usage_message =
   "Usage: ls [OPTION]... [FICHIER]...\n"
   "Afficher les informations au sujet des FICHIERS (du repertoire courant par defaut).\n"
   "\n"
   "  -a,      Affiche tous les fichiers des repertoires,  y compris les fichiers caches.\n"
   "  -R       Afficher recursivement le contenu des sous-repertoires.\n"
   "  -r,      Affiche le resultat dans l'ordre inverse.\n"
   "  -l       Utiliser le format long d'affichage.\n"
   "  -F       Ajouter un caractere indiquant le type de fichier.\n"
   "  -n       aucune couleur.\n";



/******************************************************************************
 * Ecriture des fonctions
 ******************************************************************************/

int test_couleurs(void)
{
  printf("\033[01;31m");printf("Une jolie couleur\n");couleurnormale(); /* rouge */
  printf("\033[01;32m");printf("Une jolie couleur\n");couleurnormale(); /* vert*/
  printf("\033[01;33m");printf("Une jolie couleur\n");couleurnormale(); /* jaune */
  printf("\033[01;34m");printf("Une jolie couleur\n");couleurnormale(); /* bleu */
  printf("\033[01;35m");printf("Une jolie couleur\n");couleurnormale(); /* rose */
  printf("\033[01;36m");printf("Une jolie couleur\n");couleurnormale(); /* cyan */
  return 0;
}	


/* Affiche le type du fichier (option -F) */
void affiche_type (mode_t mode){
  if (S_ISREG(mode) && (mode & S_IXUSR)) 
    printf("* ");
  else if (S_ISDIR(mode))
    printf("/ ");
  else if (S_ISFIFO(mode))
    printf("| ");
  else if (S_ISLNK(mode))
    printf("@ ");
}


/* Affiche les droits du fichier */
void affiche_droits (mode_t mode){
  /* Droits pour l'utilisateur :*/
  (mode & S_IRUSR) ? printf("r") : printf("-");
  (mode & S_IWUSR) ? printf("w") : printf("-");
  (mode & S_IXUSR) ? printf("x") : printf("-");
  
  (mode & S_IRGRP) ? printf("r") : printf("-");
  (mode & S_IWGRP) ? printf("w") : printf("-");
  (mode & S_IXGRP) ? printf("x") : printf("-");
  
  (mode & S_IROTH) ? printf("r") : printf("-");
  (mode & S_IWOTH) ? printf("w") : printf("-");
  (mode & S_IXOTH) ? printf("x") : printf("-");
  
  printf("\t");
}


/* Affiche le nom du propriétaire du fichier */
void affiche_user (uid_t uid){
  struct passwd * user;
  if ( (user = getpwuid (uid)) )
    printf ("%s\t",user->pw_name);
}


/* Affiche le nom du groupe du fichier */
void affiche_groupe (gid_t gid){
  struct group * groupe;
  if ( (groupe = getgrgid (gid)) )
    printf ("%s\t",groupe->gr_name);
}


/* Affiche la date de modification */
void affiche_date_modif (time_t date_modif){
  struct tm *infotime;
  infotime = localtime (&date_modif);
  printf ( "%2d-%2d-%2d %2d:%2d ", infotime->tm_mday, infotime->tm_mon+1, infotime->tm_year+1900, infotime->tm_hour, infotime->tm_min );
}


/* permet de lire la cible d'un lien symbolique */
char * read_the_link (char * filename){
  size_t buf_size = 128;
  char *buffer = NULL;
  ssize_t link_length;
  
  for (;;){
    buffer = malloc (buf_size*sizeof(char));
    link_length = readlink (filename, buffer, buf_size);
    
    if (link_length < 0){
      free (buffer);
      return NULL;
    }
    if ((size_t) link_length < buf_size){
      buffer[link_length] = 0;
      return buffer;
    }
    free (buffer);
    buf_size<<=1;
  }
}



/* affiche la taille d'un fichier */
void affiche_taille (off_t st_size){
  int size = (int) st_size;
  if (size >= 1073741824) 
    printf ("%.1fG", (double)(size / 1073741824.0) );
  else if (size >= 1048576) 
    printf("%.1fM", (double)(size / 1048576.0) );
  else if (size >= 1024) 
    printf ("%.1fK", (double)(size / 1024.0 ) );
  else 
    printf ("%d", size);
  printf ("\t");
}


/* fonction d'affichage d'un fichier sur une ligne (option -l) */
void affiche_fichier (char * nom, char * path, struct stat infos){
  char * link;
  
  if (option_long){
    affiche_droits (infos.st_mode);
    affiche_user (infos.st_uid);
    affiche_groupe (infos.st_gid);
    affiche_taille (infos.st_size);
    affiche_date_modif (infos.st_mtime);
  }
 
  if ((S_ISREG(infos.st_mode) && (infos.st_mode & S_IXUSR))){
    if (option_aucune_couleur)
      printf ("%s", nom);
    else {
      /* on affiche un executable en vert */
      printf("\033[01;32m");
      printf ("%s", nom);
      couleurnormale();
    }
  }
  else if (S_ISDIR(infos.st_mode)){
    if (option_aucune_couleur)
      printf ("%s", nom);
    else {
      /* on affiche un dossier en bleu fonce */
      printf("\033[01;34m");
      printf ("%s", nom);
      couleurnormale();
    }
  }
  else if (S_ISLNK(infos.st_mode)){
    if (option_aucune_couleur)
      printf ("%s", nom);
    else {
      /* on affiche un lien en cyan */
      printf("\033[01;36m");
      printf("%s", nom);
      couleurnormale();
    }
  }
  else printf("%s", nom);
    
  if (option_type) 
    affiche_type (infos.st_mode); 

  printf("\t");

  if (S_ISLNK (infos.st_mode) && (option_long)){
    if ( (link=read_the_link (path)) ){
      printf (" -> %s", link);
      free (link);
    }
    else printf ("lien incorrect");
  }
  printf ("\n");
}


/* fonction de parcours d'un répertoire et affiche son contenu */
void parcours_rep (char * directory){
  DIR * rep;
  struct dirent * objet = NULL;
  struct stat s;
  unsigned int i;
  unsigned int taille_file = 0;
  unsigned int lng = strlen (directory);
  unsigned int lng_total;
  
  char **file = NULL;
  char *str_stat = NULL;
  
  /* Tentative d'ouverture du répertoire */
  rep = opendir (directory);
  if (rep == NULL){
    if (lstat (directory, &s) == 0){
      affiche_fichier (directory, directory, s);
      return ;
    }
    else{
      fprintf (stderr,"ls:: %s: Aucun fichier ou repertoire de ce type\n", directory);
      return;
    }
  }
  
  /* Si récursif, affichage du rép actuellement parcouru */
  if (option_recursive)
    printf ("\n%s:\n", directory);
      
  /* Parcours du contenu du répertoire */
  while ( (objet = readdir (rep)) ){
    lng_total = lng + strlen(objet->d_name) + 2;
    str_stat = malloc (lng_total * sizeof(char));
    snprintf (str_stat, lng_total, "%s/%s", directory, objet->d_name);
    
    if ( lstat (str_stat, &s) == -1 ){
      printf("ls:: %s: Permission non accordee\n", str_stat);
      free (str_stat);
    }
    else{
      /* fichier caché */
      if ( (objet->d_name[0] == '.' && (option_all)) || objet->d_name[0] != '.'){
	affiche_fichier(objet->d_name, str_stat, s);
	
	/* Affichage récursif */
	/* Mise en file d'attente */
	if ((option_recursive) && S_ISDIR (s.st_mode) ){
	  if ((option_all)){
	    if ( strcmp (objet->d_name, ".") && strcmp (objet->d_name, "..") )
	      {
		file = realloc (file, (++taille_file) * sizeof (char *) );
		file[taille_file-1] = str_stat;
	      }
	  }
	  else{
	    file = realloc (file, (++taille_file) * sizeof (char *) );
	    file[taille_file-1] = str_stat;
	  }
	}
	else {
	  free (str_stat);
	}
      }
      else {
	free (str_stat);
      }
    }
    
    str_stat = NULL;
    objet = NULL;
  }
  
  closedir (rep);
  
  /* Parcours de la file */
  for (i=0 ; i < taille_file ; i++){
    parcours_rep (file[i]);
    free (file[i]);
  }
  free(file);
}





/* fonction principale de ls appelee par le shell */
int internal_ls (int argc, char ** argv){
  int c;
  int diff;
  struct stat s;

  for (;;){
    c=getopt(argc, argv, optstring);
    if (c==-1)
      break;
    switch (c){
    case 'h':
      printf(usage_message);
      return 1;
      break;
    case 'n' :
      option_aucune_couleur = 1;
      break;
    case 'a':
      option_all=1;
      break;
    case 'l':
      option_long=1;
      break;
    case 'R':
      option_recursive=1;
      break;
    case 'F':
      option_type=1;
      break;
    case 'r':
      option_tri_inverse=1;
      break;
    case '?':
      /* option inconnue on rend la main au shell */
      option_all=0;
      option_recursive=0;
      option_tri_inverse=0;
      option_long=0;
      option_type=0;
      option_aucune_couleur=0;
      return 1;
    }
  }
  
  diff = argc-optind;
  if (!diff){
    parcours_rep (".");
  }
  else{
    while (optind < argc){  
      if (diff > 1){
	if ( lstat (argv[optind], &s) == -1 ){
	  printf("ls: %s: Permission non accordee\n", argv[optind]);
	}
	else if (S_ISDIR(s.st_mode)){
	  printf ("%s:\n", argv[optind]);
	  parcours_rep (argv[optind++]);
	  printf ("\n");
	}
	else{
	  parcours_rep (argv[optind++]);
	}
      }
      else{
	parcours_rep (argv[optind++]);
      }
    }
  }
  
  option_all=0;
  option_recursive=0;
  option_tri_inverse=0;
  option_long=0;
  option_type=0;
  option_aucune_couleur=0;
  return 0;
}

