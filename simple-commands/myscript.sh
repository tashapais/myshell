echo "This is a simple shell script for mysh."

pwd 
echo "Printed the current working directory."

cd ..
echo "Changed directory to the parent directory."

pwd
echo "Printed the current working directory."

mkdir test_directory
echo "Created a new directory called 'test_directory'."

pwd
echo "Printed the current working directory."

cd test_directory
echo "Changed directory to 'test_directory'."

pwd
echo "Printed the current working directory."

cd ..
rmdir test_directory
echo "Removed the 'test_directory'."

pwd
echo "Printed the current working directory."

echo "End of the script."
