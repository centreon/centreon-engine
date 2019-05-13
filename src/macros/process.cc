/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/process.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/*
 * replace macros in notification commands with their values,
 * the thread-safe version
 */
int process_macros_r(
      nagios_macros* mac,
      char const* input_buffer,
      char** output_buffer,
      int options) {
  char* temp_buffer = NULL;
  char* save_buffer = NULL;
  char* buf_ptr = NULL;
  char* delim_ptr = NULL;
  int in_macro = false;
  char* selected_macro = NULL;
  char* original_macro = NULL;
  std::string cleaned_macro;
  int clean_macro = false;
  int result = OK;
  int clean_options = 0;
  int free_macro = false;
  int macro_options = 0;

  logger(dbg_functions, basic)
    << "process_macros_r()";

  if (output_buffer == NULL)
    return ERROR;

  *output_buffer = string::dup("");

  if (input_buffer == NULL)
    return ERROR;

  in_macro = false;

  logger(dbg_macros, more)
    << "**** BEGIN MACRO PROCESSING ***********\n"
    "Processing: '" << input_buffer << "'";

  /* use a duplicate of original buffer, so we don't modify the original */
  save_buffer = buf_ptr = (input_buffer ? string::dup(input_buffer) : NULL);

  while (buf_ptr) {
    /* save pointer to this working part of buffer */
    temp_buffer = buf_ptr;

    /* find the next delimiter - terminate preceding string and advance buffer pointer for next run */
    if ((delim_ptr = strchr(buf_ptr, '$'))) {
      delim_ptr[0] = '\x0';
      buf_ptr = (char* )delim_ptr + 1;
    }
    /* no delimiter found - we already have the last of the buffer */
    else
      buf_ptr = NULL;

    logger(dbg_macros, most)
      << "  Processing part: '" << temp_buffer << "'";

    selected_macro = NULL;
    clean_macro = false;

    /* we're in plain text... */
    if (!in_macro) {

      /* add the plain text to the end of the already processed buffer */
      *output_buffer = resize_string(
                         *output_buffer,
                         strlen(*output_buffer)
                         + strlen(temp_buffer)
                         + 1);
      strcat(*output_buffer, temp_buffer);

      logger(dbg_macros, most)
        << "  Not currently in macro.  Running output ("
        << strlen(*output_buffer) << "): '" << *output_buffer << "'";
      in_macro = true;
    }
    /* looks like we're in a macro, so process it... */
    else {

      /* reset clean options */
      clean_options = 0;

      /* grab the macro value */
      result = grab_macro_value_r(
                 mac,
                 temp_buffer,
                 &selected_macro,
                 &clean_options,
                 &free_macro);
      logger(dbg_macros, most)
        << "  Processed '" << temp_buffer
        << "', Clean Options: " << clean_options
        << ", Free: " << free_macro;

      /* an error occurred - we couldn't parse the macro, so continue on */
      if (result == ERROR) {
        logger(dbg_macros, basic)
          << " WARNING: An error occurred processing macro '"
          << temp_buffer << "'!";
        if (free_macro) {
          delete[] selected_macro;
          selected_macro = NULL;
        }
      }

      /* we already have a macro... */
      if (result == OK) {
      }
      /* an escaped $ is done by specifying two $$ next to each other */
      else if (!strcmp(temp_buffer, "")) {
        logger(dbg_macros, most)
          << "  Escaped $.  Running output (" << strlen(*output_buffer)
          << "): '" << *output_buffer << "'";
        *output_buffer = resize_string(
                           *output_buffer,
                           strlen(*output_buffer) + 2);
        strcat(*output_buffer, "$");
      }
      /* a non-macro, just some user-defined string between two $s */
      else {
        logger(dbg_macros, most)
          << "  Non-macro.  Running output (" << strlen(*output_buffer)
          << "): '" << *output_buffer << "'";

        /* add the plain text to the end of the already processed buffer */
        /*
         *output_buffer=(char*)realloc(*output_buffer,strlen(*output_buffer)+strlen(temp_buffer)+3);
         strcat(*output_buffer,"$");
         strcat(*output_buffer,temp_buffer);
         strcat(*output_buffer,"$");
	*/
      }

      /* insert macro */
      if (selected_macro != NULL) {
        logger(dbg_macros, most)
          << "  Processed '" << temp_buffer
          << "', Clean Options: " << clean_options
          << ", Free: " << free_macro;

        /* include any cleaning options passed back to us */
        macro_options = (options | clean_options);

        logger(dbg_macros, most)
          << "  Cleaning options: global=" << options
          << ", local=" << clean_options
          << ", effective=" << macro_options;

        /* URL encode the macro if requested - this allocates new memory */
        if (macro_options & URL_ENCODE_MACRO_CHARS) {
          original_macro = selected_macro;
          selected_macro = get_url_encoded_string(selected_macro);
          if (free_macro) {
            delete[] original_macro;
            original_macro = NULL;
          }
          free_macro = true;
        }

        /* some macros are cleaned... */
        if (clean_macro
            || (macro_options & STRIP_ILLEGAL_MACRO_CHARS)
            || (macro_options & ESCAPE_MACRO_CHARS)) {

          /* add the (cleaned) processed macro to the end of the already
           * processed buffer */
          if (selected_macro != NULL) {
            cleaned_macro = clean_macro_chars(selected_macro, macro_options);
            if (!cleaned_macro.empty()) {
              *output_buffer = resize_string(
                *output_buffer,
                strlen(*output_buffer) + cleaned_macro.size() + 1);
              strcat(*output_buffer, cleaned_macro.c_str());

              logger(dbg_macros, basic)
                << "  Cleaned macro.  Running output ("
                << strlen(*output_buffer) << "): '" << *output_buffer << "'";
            }
          }
        }
        /* others are not cleaned */
        else {
          /* add the processed macro to the end of the already processed buffer */
          if (selected_macro != NULL) {
            *output_buffer = resize_string(
                               *output_buffer,
                               strlen(*output_buffer)
                               + strlen(selected_macro)
                               + 1);
            strcat(*output_buffer, selected_macro);

            logger(dbg_macros, basic)
              << "  Uncleaned macro.  Running output ("
              << strlen(*output_buffer) << "): '"
              << *output_buffer << "'";
          }
        }

        /* free memory if necessary (if we URL encoded the macro or we were told to do so by grab_macro_value()) */
        if (free_macro) {
          delete[] selected_macro;
          selected_macro = NULL;
        }
        logger(dbg_macros, basic)
          << "  Just finished macro.  Running output ("
          << strlen(*output_buffer) << "): '"
          << *output_buffer << "'";
      }

      in_macro = false;
    }
  }

  /* free copy of input buffer */
  delete[] save_buffer;

  logger(dbg_macros, more)
    << "  Done.  Final output: '" << *output_buffer << "'\n"
    "**** END MACRO PROCESSING *************";
  return OK;
}

int process_macros(
      char const* input_buffer,
      char** output_buffer,
      int options) {
  return (process_macros_r(
            get_global_macros(),
            input_buffer,
            output_buffer,
            options));
}
