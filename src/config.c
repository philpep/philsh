#include <stdio.h>
#include <confuse.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PROMPT  "[\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m%s\033[37m] $> "

int config_init(void)
{

  /* On obtiens le chemin complet de la config */
  char *chaine_home = getenv("HOME");
  char *chaine_conf = "/.philshrc";
  strcat(chaine_home, chaine_conf);

  cfg_opt_t opts[] =
  {
    CFG_STR("prompt", DEFAULT_PROMPT, CFGF_NONE),
    CFG_END()
  };
  cfg_t *cfg;
  cfg = cfg_init(opts, CFGF_NONE);

  int ret;
  ret = cfg_parse(cfg, chaine_home);
  if(ret == CFG_FILE_ERROR){
    printf("Impossible de charger la configuration : %s\n", chaine_home);
    exit(EXIT_FAILURE);
  } else if(ret == CFG_PARSE_ERROR)
    cfg_error(cfg, "Impossible de parser la configuration : %s\n", chaine_home);

  /* char *prompt = cfg_getstr(cfg, "prompt"); */
  /* prompt = cfg_getstr(cfg, "prompt"); */

  cfg_free(cfg);
  return 0;
}
