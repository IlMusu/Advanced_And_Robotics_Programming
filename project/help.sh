cat \
project_description.txt <(echo) \
sources/master/master.txt <(echo) \
sources/motor/motor.txt <(echo) \
sources/command_console/command_console.txt <(echo) \
sources/inspection_console/inspection_console.txt <(echo) \
sources/watchdog/watchdog.txt <(echo) \
sources/libraries/logger.txt <(echo) \
sources/log/log.txt <(echo) \
| more -d -c -p
clear
