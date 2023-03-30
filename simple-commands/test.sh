echo "______Test 1: set up test environment as well as test basic creation and navigation shell commands"
touch foo bar baz
mkdir test
echo "this is the text for foo" > foo
echo "this is the text for bar" > foo
echo "this is the text for baz" > foo
echo this is a test of echo command
ls
pwd
cd test
touch new.txt
echo "This is the text in new.txt" > new.txt
ls
pwd
cd ..

echo "______Test 2: test display and redirect commands"
touch file1.txt file2.txt output.txt output2.txt
cat foo
cat foo bar baz
cat < input.txt 
echo "This is a test for output redirection." > output.txt
cat < foo > output2.txt

echo "________Test 3: test basic piping"
ls | grep file
ls | cat


echo "________Test 4: more intense redirect tests"
echo foo bar > baz
echo foo > baz bar
echo > baz foo bar
sort < foo > bar
sort > bar < foo
echo foo > bar > baz
cat < foo < bar


echo "________Test 5: testing the interaction between redirect and pipes. Shouldn't do anything unreasonable"
echo foo > bar | cat
echo foo | cat < bar
pwd > output.txt
touch foo | cd test
cd test | pwd

echo "________Test 6: testing file wildcards and directory wildcards"
ls test/*.c
echo */foo
cat foo*

echo "________Test 7: test compilation and permission modification"
gcc testing.c -o testing
./testing
./non
chmod -x testing
./testing
chmod +x testing
./testing

echo "________Test 8: testing home directory"
ls ~
echo ~

echo "________Test 9: remove created files and directory. comment this out if using second or third exit test"
rm foo bar baz test file1.txt file2.txt output.txt output2.txt testing

echo "________Test 10: test exit with pipes"
touch foo | exit

# exit | echo bar

# #test exit with redirect
# cd test
# exit > foo
