all: main.pdf

main.pdf: main.tex biblio.bib transmission.tex
	pdflatex main.tex
	pdflatex main.tex
	bibtex main
	bibtex main
	pdflatex main.tex
	pdflatex main.tex

clean:
	rm -f *.aux *.log *.out *.bbl *.blg include/*.aux
