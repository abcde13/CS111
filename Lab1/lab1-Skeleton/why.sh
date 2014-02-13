true

g++ -c foo.c

: : :

cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

a b<c > d

#cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!

#a&&b||
# c &&
#  d | e && f|

#g<h

# This is a weird example: nobody would ever want to run this.
#a<b>c|d<e>f|g<h>i

#Special test case for lab 2
#ls | od -c > a

#Special case two
#(b < a) && cat a || pwd
