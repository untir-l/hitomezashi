/** @file */

#include "hitomezashi_cli.h"

#include "SDL2/SDL.h"
#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "hitomezashi.h"
#include "hitomezashi_utils.h"
#include "optparse.h"

/** Initialise SDL, pass the information from hitomezashi_cli_handle_args() to
 * libhitomezashi, and save the resulting image. */
int main(int argc, char **argv) {
  char *out_file_path;
  int x_pattern_len;
  int y_pattern_len;
  char *x_pattern;
  char *y_pattern;
  int gap;
  int thickness;
  Uint32 fg_colour;
  Uint32 bg_colour;
  hitomezashi_cli_handle_args(&out_file_path, &x_pattern_len, &y_pattern_len,
                              &x_pattern, &y_pattern, &gap, &thickness,
                              &fg_colour, &bg_colour, argc, argv);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to initialise SDL: %s",
                    SDL_GetError());
    return Hitomezashi_Cli_Exit_Code_Err_Sdl_Init;
  }

  struct Hitomezashi_State state;
  if (hitomezashi_state_init(&state, x_pattern_len, y_pattern_len, x_pattern,
                             y_pattern, gap, thickness, fg_colour,
                             bg_colour) != 0) {
    return Hitomezashi_Cli_Exit_Code_Err_State_Init;
  }

  hitomezashi_draw(&state);

  if (SDL_SaveBMP(state.surface, out_file_path) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to save image");
    return Hitomezashi_Cli_Exit_Code_Err_Save_Image;
  }

  SDL_FreeSurface(state.surface);
  free(state.x_pattern);
  free(state.y_pattern);

  SDL_Quit();
}

void hitomezashi_cli_help(void) {
  if (puts("hitomezashi_cli - generate hitomezashi patterns\n"
           "Options:\n"
           "-o - specify the output image (BMP) path\n"
           "-x - specify the x pattern as a string of zeroes and ones (eg. "
           "0101)\n"
           "-y - specify the y pattern, as with -x\n"
           "-g - specify the gap between lines as an integer\n"
           "-t - specify the line thickness as an integer\n"
           "-f - (optional) specify the foreground colour in RGB as an "
           "integer, eg. 0x000000\n"
           "-b - (optional) specify the background colour in RGB as an "
           "integer, eg. 0xFFFFFF\n"
           "-h - print this help and exit") < 0) {
    exit(Hitomezashi_Cli_Exit_Code_Err_Print_Help);
  }
  exit(Hitomezashi_Cli_Exit_Code_Success);
}

void hitomezashi_cli_handle_args(char **out_file_path, int *x_pattern_len,
                                 int *y_pattern_len, char **x_pattern,
                                 char **y_pattern, int *gap, int *thickness,
                                 Uint32 *fg_colour, Uint32 *bg_colour, int argc,
                                 char **argv) {
  bool out_file_path_specified = false;
  bool x_pattern_specified = false;
  bool y_pattern_specified = false;
  bool gap_specified = false;
  bool thickness_specified = false;
  *fg_colour = 0x000000;
  *bg_colour = 0xffffff;

  struct optparse options;
  optparse_init(&options, argv);
  int option;
  while ((option = optparse(&options, ":o:x:y:g:t:f:b:h")) != -1) {
    switch (option) {
    case 'o':;
      *out_file_path = options.optarg;
      out_file_path_specified = true;
      break;
    case 'x':;
      size_t x_pattern_len_l = strlen(options.optarg);
      if (x_pattern_len_l >= INT_MAX) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                        "X pattern length must be shorter than %d", INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *x_pattern_len = x_pattern_len_l;
      *x_pattern =
          hitomezashi_ascii_binary_str_to_ints(options.optarg, *x_pattern_len);
      if (!*x_pattern) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Invalid x pattern; see -h");
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      x_pattern_specified = true;
      break;
    case 'y':;
      size_t y_pattern_len_l = strlen(options.optarg);
      if (y_pattern_len_l >= INT_MAX) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                        "Y pattern length must be shorter than %d", INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *y_pattern_len = y_pattern_len_l;
      *y_pattern =
          hitomezashi_ascii_binary_str_to_ints(options.optarg, *y_pattern_len);
      if (!*y_pattern) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Invalid y pattern; see -h");
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      y_pattern_specified = true;
      break;
    case 'g':;
      long gap_l = strtol(options.optarg, NULL, 0);
      if (gap_l <= 0 || gap_l >= INT_MAX) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                        "Value for gap must be positive and less than %d",
                        INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *gap = gap_l;
      gap_specified = true;
      break;
    case 't':;
      long thickness_l = strtol(options.optarg, NULL, 0);
      if (thickness_l <= 0 || thickness_l >= INT_MAX) {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_ERROR,
            "Value for line thickness must be positive and less than %d",
            INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *thickness = thickness_l;
      thickness_specified = true;
      break;
    case 'f':;
      long fg_colour_l = strtol(options.optarg, NULL, 0);
      if (fg_colour_l < 0 || fg_colour_l > INT_MAX) {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_ERROR,
            "Value for foreground colour must be non-negative and less than %d",
            INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *fg_colour = fg_colour_l;
      break;
    case 'b':;
      long bg_colour_l = strtol(options.optarg, NULL, 0);
      if (bg_colour_l < 0 || bg_colour_l > INT_MAX) {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_ERROR,
            "Value for background colour must be non-negative and less than %d",
            INT_MAX);
        exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
      }
      *bg_colour = bg_colour_l;
      break;
    case 'h':;
      hitomezashi_cli_help();
      break;
    default:;
      SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                      "Invalid argument: run with -h for help");
      exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
    }
  }
  if (!(out_file_path_specified && x_pattern_specified && y_pattern_specified &&
        gap_specified && thickness_specified)) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_ERROR,
        "-o, -x, -y, -g, -t *must* be specified; run with -h for help");
    exit(Hitomezashi_Cli_Exit_Code_Err_Handle_Args);
  }
}
