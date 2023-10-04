#!/bin/sh


case "$1" in
    start)
        echo "starting"
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
        ;;
    stop)
        echo "stopping"
        start-stop-daemon -K -n aesdsocket
        ;;
    *)
        echo "unexpected"
        echo "$1"
        exit 1
        ;;
esac

exit 0
