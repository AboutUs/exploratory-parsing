mkdir -p data/Wikipedia

VERSION=`curl -s http://dumps.wikimedia.org/backup-index.html |
	perl -ne 'print "$1\n" if /"(enwiki\/\d+)".*'done'/'`

echo Downloading version $VERSION

curl -s http://dumps.wikimedia.org/$VERSION/ |
	perl -ne '
		if (/"([^"]*?pages-articles(\d+).xml.bz2)"/) {
			print $to = sprintf("%3.3d",$2), "\n";
			`curl -sL $1 | bzcat >data/Wikipedia/$to`;
		}
	'

echo Done