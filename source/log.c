// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "3asm.h"
#include <libvacant/libvacant.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

// wrappers for libvacant

#define ESCAPE_RESET "\x1b[0m"
#define ESCAPE_UNDERLINED "\x1b[1;4m"

#define ESCAPE_FOREGROUND_BLACK "\x1b[1;38;5;232m"
#define ESCAPE_BACKGROUND_BLACK "\x1b[1;48;5;232m"

#define ESCAPE_FOREGROUND_WHITE "\x1b[1;38;5;231m"
#define ESCAPE_BACKGROUND_WHITE "\x1b[1;48;5;231m"

#define ESCAPE_FOREGROUND_RED "\x1b[1;31m"
#define ESCAPE_BACKGROUND_RED "\x1b[1;41m"

#define ESCAPE_BACKGROUND_BLUE "\x1b[1;48;5;27m"

#define ESCAPE_BACKGROUND_GREEN "\x1b[1;102m"

#define ESCAPE_BACKGROUND_YELLOW "\x1b[1;103m"

#define ESCAPE_BACKGROUND_CYAN "\x1b[1;46m"

#define ESCAPE_BACKGROUND_GRAY "\x1b[1;100m"

static char *level_labels[LOGGER_LEVEL_COUNT] = {
                                                         "     ???     " ESCAPE_RESET,
        ESCAPE_FOREGROUND_RED   ESCAPE_BACKGROUND_WHITE  "    FATAL    " ESCAPE_RESET,
        ESCAPE_FOREGROUND_WHITE ESCAPE_BACKGROUND_RED    "    ERROR    " ESCAPE_RESET,
        ESCAPE_FOREGROUND_BLACK ESCAPE_BACKGROUND_YELLOW "   WARNING   " ESCAPE_RESET,
        ESCAPE_FOREGROUND_WHITE ESCAPE_BACKGROUND_BLUE   " INFORMATION " ESCAPE_RESET,
        ESCAPE_FOREGROUND_BLACK ESCAPE_BACKGROUND_GREEN  "   VERBOSE   " ESCAPE_RESET,
        ESCAPE_FOREGROUND_WHITE ESCAPE_BACKGROUND_GRAY   "    DEBUG    " ESCAPE_RESET
};




void logger_wrapper(context_t *context, logger_level_t level, char *prefix, char *message) {
        logger_message(&context->logger, level, "");
        logger_message(&context->logger, level, "  %s " ESCAPE_FOREGROUND_WHITE ESCAPE_BACKGROUND_GRAY "  %s  " ESCAPE_RESET, level_labels[level], prefix);
        logger_message(&context->logger, level, ESCAPE_FOREGROUND_WHITE "  » " ESCAPE_UNDERLINED "%s" ESCAPE_RESET, message);
}

#define DEFINE_SYSTEM_LOGGER(name, level) \
void SYSTEM_LOGGER(name) { \
        va_list arguments; \
        char message[LOGGER_MESSAGE_BUFFER_SIZE]; \
        memset(message, 0, LOGGER_MESSAGE_BUFFER_SIZE); \
        va_start(arguments, format); \
        vsnprintf(message, LOGGER_MESSAGE_BUFFER_SIZE, format, arguments); \
        va_end(arguments); \
        logger_wrapper(context, level, prefix, message); \
} \


DEFINE_SYSTEM_LOGGER(fatal, LOGGER_FATAL)
DEFINE_SYSTEM_LOGGER(error, LOGGER_ERROR)
DEFINE_SYSTEM_LOGGER(warn, LOGGER_WARN)
DEFINE_SYSTEM_LOGGER(info, LOGGER_INFO)
DEFINE_SYSTEM_LOGGER(verbose, LOGGER_VERBOSE)
DEFINE_SYSTEM_LOGGER(debug, LOGGER_DEBUG)

