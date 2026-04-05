// By default it is compiled with this header file.
#pragma once

#define HOST_OS "Artix"

#define SV "/usr/bin/sv"

/*  Note: The paths of the directories without
          the separator ('/') at the end. */

#define SV_DIR "/etc/runit/sv"

#define SV_RUN_DIR "/run/runit/service"

#define SYS_LOG_DIR "/var/log"

// Only for SV_RUN_DIR
// Ignore some services. Separator format ':'
#define IGNORE_RUN_SERVICES "current:supervise"
