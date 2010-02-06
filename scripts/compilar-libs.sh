# /bin/bash

AGO="`pwd`"
LIBDIR="`pwd`/src/lib"

LIBFT=agoft
LIBMATRIX=agomatrix
LIBLOG=agolog

CFLAGS="-g -O0 -Wall -Wextra -DVERBOSE"

#Log
gcc $CFLAGS -fPIC -c $LIBDIR/log/$LIBLOG.c \
    -I$AGO/src/include -DVERBOSE
gcc $CFLAGS -shared -Wl,-soname,lib$LIBLOG.so -o $LIBDIR/lib$LIBLOG.so $LIBLOG.o -lc

# Fault Tolerance
gcc $CFLAGS -fPIC -c $LIBDIR/ft/$LIBFT.c \
    -I$AGO/src/include -DVERBOSE
gcc $CFLAGS -fPIC -c $LIBDIR/ft/agocomm_socket.c \
    -I$AGO/src/include -DVERBOSE
gcc $CFLAGS -shared -Wl,-soname,lib$LIBFT.so -o $LIBDIR/lib$LIBFT.so $LIBFT.o agocomm_socket.o -lc -lpthread

# Matrix
gcc $CFLAGS -fPIC -c $LIBDIR/matrix/$LIBMATRIX.c \
    -I$AGO/src/include -DVERBOSE
gcc $CFLAGS -shared -Wl,-soname,lib$LIBMATRIX.so -o $LIBDIR/lib$LIBMATRIX.so $LIBMATRIX.o -lc

rm $AGO/*.o
sudo mv $LIBDIR/*.so /usr/lib
sudo /sbin/ldconfig
echo `date` > $LIBDIR/ultima_compilacao_libs
