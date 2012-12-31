Thomas Zaorski
RIN: 660747712
EMAIL: zaorst@rpi.edu

My application uses threads to handle multiple client connections, so you must include -lpthread when you compile my server.c file.

Also, when working with my client.c I had to use "ubuntu" as my hostname on the command line. Not sure how that will be different for you.

Compile my server.c and client.c and run the server.c first.
