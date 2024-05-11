# Check that we run as root
if [ $EUID -ne 0 ]; then
	exit 1
fi

if [ $# -ne 1 ]; then
	exit 1
fi

FILE=$1

# Verify that the file exists
if [ ! -f $FILE ]; then
	exit 1
fi

# Verify that the file is not empty , empty files are safe
if [ ! -s $FILE ]; then
	echo "SAFE"
	exit 0
fi

IS_SUSPECT=0

# Count the number of lines, words and characters in the file
LINES=$(wc -l $FILE | awk '{print $1}')
WORDS=$(wc -w $FILE | awk '{print $1}')
CHARS=$(wc -m $FILE | awk '{print $1}')

# If there's less than 3 lines, more than 1000 words and more than 2000 characters, file is suspect
if [ $LINES -lt 3 ] && [ $WORDS -gt 1000 ] && [ $CHARS -gt 2000 ]; then
	IS_SUSPECT=1
fi

if [ $IS_SUSPECT -eq 1 ]; then
	# Check if the file has non-ascii words, or any of the following words: corrupted, dangerous, risk, attack, malware, malicious
	IS_MALICIOUS=0

	# Check if the file has non-ascii words
	TR_RESULT=$(tr -d -c '[:print:]\n' < $FILE)

	if [ "$TR_RESULT" != "$(cat $FILE)" ]; then
		IS_MALICIOUS=1
	fi

	# Checks for words: corrupted, dangerous, risk, attack, malware, malicious
	if egrep -q "corrupted|dangerous|risk|attack|malware|malicious" $FILE; then
		IS_MALICIOUS=1
	fi

	if [ $IS_MALICIOUS -eq 1 ]; then
		echo "$FILE"
		exit 0
	fi
fi

echo "SAFE"
exit 0
