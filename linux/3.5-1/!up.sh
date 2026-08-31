#!/bin/bash
#source .bash_colors
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)

LRED="\e[1;31m"
LGREEN="\e[1;32m"
BOLD=$(tput bold)
RESET=$(tput sgr0)

clear
#**********************************************************************************************#
#                   uploading executing file to ftp-server                                     #
#**********************************************************************************************#
echo -e "$GREEN === upload inventory to ftp-server ===  $RESET"
sleep 2s

if curl -T inventory ftp://a0831755:vbnz1002@ftp.denkin.ru/domains/denkin.ru/public_html/tmp/
#if curl -T inventory ftp://a0831755:vbnz1002@ftp.denkin.ru/tmp/
 then echo -e "${LGREEN} The process has been completed successfully !!! ${RESET}" # зеленым
 else echo -e "${RED}   Package build failure (;  ${RESET}"                        # красным
fi

sleep 3