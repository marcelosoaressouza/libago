FILES=$(find $SRCDIRS -type f -name \*.c -o -name \*.h)
astyle --indent=spaces=2 --brackets=linux --convert-tabs ${FILES}
