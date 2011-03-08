#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nagios.h"
#include "macros.h"
#include "common.h"

#undef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "./smallconfig/nagios.cfg"

extern char* config_file;
extern char** macro_x[MACRO_X_COUNT];

typedef struct s_test
{
  char*        input;
  char*        result;
  int          option;
}              t_test;

static const t_test gl_tab_test[] =
  {
    { "test without macro", "test without macro", 0 },
    { "test without macro", "test without macro", STRIP_ILLEGAL_MACRO_CHARS },
    { "test without macro", "test without macro", URL_ENCODE_MACRO_CHARS },
    { "test show $$", "test show $", 0 },
    { "test show $$", "test show $", STRIP_ILLEGAL_MACRO_CHARS },
    { "test show $$", "test show $", URL_ENCODE_MACRO_CHARS },
    { "test macro ADMINEMAIL: |$ADMINEMAIL$|", "test macro ADMINEMAIL: |root@test.com|", 0 },
    { "test macro ADMINEMAIL: |$ADMINEMAIL$|", "test macro ADMINEMAIL: |root@test.com|", STRIP_ILLEGAL_MACRO_CHARS },
    { "test macro ADMINEMAIL: |$ADMINEMAIL$|", "test macro ADMINEMAIL: |root%40test.com|", URL_ENCODE_MACRO_CHARS },
    { "test url encode |$COMMANDFILE$|", "test url encode |&\"'(-_)=~#{[|`\^@]}+^*%!:/;.,?<>|", 0 },
    { "test url encode |$COMMANDFILE$|", "test url encode |(-_)=#{[^@]}+^*%!:/;.,?|", STRIP_ILLEGAL_MACRO_CHARS },
    { "test url encode |$COMMANDFILE$|", "test url encode |%28-_%29=%23%7B%5B%5E%40%5D%7D%2B%5E%2A%25%21:/%3B.%2C?|", URL_ENCODE_MACRO_CHARS },
    { "test macro TEST dosen't exist: |$TEST$|", "test macro TEST dosen't exist: ||", 0 },
    { "test macro TEST dosen't exist: |$TEST$|", "test macro TEST dosen't exist: ||", STRIP_ILLEGAL_MACRO_CHARS },
    { "test macro TEST dosen't exist: |$TEST$|", "test macro TEST dosen't exist: ||", URL_ENCODE_MACRO_CHARS },
    { NULL, NULL, 0 }
  };

int main(void)
{
  char* output = NULL;
  int i;

  config_file = DEFAULT_CONFIG_FILE;

  if (reset_variables() == ERROR ||
      read_main_config_file(config_file) == ERROR ||
      read_all_object_data(config_file) == ERROR)
    return (1);

  if (macro_x[MACRO_ADMINEMAIL] != NULL)
    free(macro_x[MACRO_ADMINEMAIL]);
  macro_x[MACRO_ADMINEMAIL] = strdup("root@test.com");

  if (macro_x[MACRO_COMMANDFILE] != NULL)
    free(macro_x[MACRO_COMMANDFILE]);
  macro_x[MACRO_COMMANDFILE] = strdup("&\"'(-_)=~#{[|`\^@]}+^*%!:/;.,?<>");

  for (i = 0; gl_tab_test[i].input != NULL; ++i)
    if (process_macros(gl_tab_test[i].input, &output, gl_tab_test[i].option) == OK &&
	!strcmp(gl_tab_test[i].result, output))
      free(output);
    else
      {
	fprintf(stderr,
		"input : `%s`\n"
		"output: `%s`\n"
		"result: `%s`\n",
		gl_tab_test[i].input, output, gl_tab_test[i].result);
	cleanup();
	return (1);
      }

  cleanup();
  return (0);
}
