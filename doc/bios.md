# Interruptions

*r1* holds the funcion code

## int 0

* 0 - print the integer value of *r2* to the console
* 1 - print the char value of *r2* to the console

## int 0x10 - gfx

* 0 - draw a pixel *(r2, r3)* with color in *r4*
* 5 - clears the screen with the color in *r2*
* 10 - prints the character in *r2*
* 11 - sets the cursor to *(r2, r3)*


## int 0x13 - disk i/o

* 0 - returns in *r1* if the disk is connected
* 1 - reads the sector *r2* in the location pointed by *r3*
* 2 - writes to the sector *r2* the contents pointed by *r3*
