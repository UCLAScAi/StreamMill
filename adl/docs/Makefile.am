EXTRA_DIST = SQL-AG.eps ntsql.tex ntsql-db2.tex sqlag.html sqlag.jpg
CLEANFILES = *.html *.old img*.gif *.aux *.log 

all: index.html db2index.html
	rehab.sh

index.html: ntsql.tex
	latex2html -split +3 -local_icons -link 3 -no_subdir ntsql.tex

db2index.html: ntsql-db2.tex
	latex2html -prefix db2 -split +3 -local_icons -link 3 -no_subdir ntsql-db2.tex

clean:
	rm -f *.html *.old

