all: main.pdf

main.pdf: _main.tex biblio.bib
	pdflatex _main.tex
	pdflatex _main.tex
	bibtex _main
	bibtex _main
	pdflatex _main.tex
	pdflatex _main.tex

clean:
	rm -f *.aux *.log *.out *.bbl *.blg include/*.aux
