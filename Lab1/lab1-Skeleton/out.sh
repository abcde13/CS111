echo BEGIN

echo THIS SHOULD PRINT || echo THIS SHOULD NOT PRINT

cat < test1bok.sh | tr a-z A-Z | sort -u || echo sort failed!

#a b<c > d

cat < test1bok.sh | tr a-z A-Z | sort -u > test.sh || echo sort failed!

((cat test1bok.sh | sort > test.sh) && od -c < test.sh) || echo MY WORLD; echo HELLO WORLD || echo THIS SHOULD NOT PRINT;  echo THIS SHOULD PRINT && echo THIS SHOULD PRINT TOO;

#COMMENT IS HERE

echo 1;
#COMMENT IS HERE
echo 2 echo 3;
#COMMENT IS HERE
echo 4

#COMMENT IS HERE
echo 567890;

