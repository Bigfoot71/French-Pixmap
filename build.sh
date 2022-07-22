#!/bin/bash

#                           USAGE                           #

# ./build                   -   for compile release.
# ./build execute           -   for release and execute it.
# ./build debug             -   for compile debug.
# ./build debug execute     -   for debug and execute it.

#                                                           #

if [[ $1 == "debug" ]]; then

    printf "Compilation en cours de la version DEBUG ..."

    g++ -g -DDEBUG -W -Wall -Werror -Wextra -O3 src/pixmap/pixmap.cpp src/main.cpp -o bin/main_debug -lSDL2 -lSDL2_ttf

    # g++ -g -DDEBUG -W -Wall -Werror -Wextra -O3 src/pixmap/pixmap.cpp src/main.cpp `sdl2-config --cflags --libs` -o bin/main_debug -lSDL2_ttf

    if [[ $2 == "execute" ]]; then

        printf "\nExecution de la version DEBUG.\n\n"

        ./bin/main_debug

    else

        printf "\nLa compilation est fini. DIR: bin/main_debug\n"

    fi

else

    printf "Compilation en cours de la version RELEASE ..."

    g++ -g -W -Wall -Werror -Wextra -O3 src/pixmap/pixmap.cpp src/main.cpp -o bin/main_release -lSDL2 -lSDL2_ttf

    if [[ $1 == "execute" ]]; then

        printf "\nExecution de la version RELEASE.\n\n"

        ./bin/main_release

    else

        printf "\nLa compilation est fini. DIR: bin/main_release\n"

    fi

fi
