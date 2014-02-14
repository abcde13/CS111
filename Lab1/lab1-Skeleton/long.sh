ls -a

((ls -t | sort > b.txt) && od -c < a.txt) || echo MY WORLD; echo HELLO WORLD || echo THIS SHOULD NOT PRINT;  echo THIS SHOULD PRINT && echo THIS SHOULD PRINT TOO;

#COMMENT IS HERE

echo 1;
#COMMENT IS HERE
echo 2 echo 3;
#COMMENT IS HERE
echo 4

#COMMENT IS HERE
echo 567890;
