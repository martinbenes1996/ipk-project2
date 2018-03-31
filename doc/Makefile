all: dokumentace.pdf

dokumentace.pdf:
		pdflatex dokumentace.tex
		biber dokumentace
		pdflatex dokumentace.tex
		pdflatex dokumentace.tex

.PHONY: clean
clean:
		rm dokumentace.aux dokumentace.bbl dokumentace.bcf dokumentace.blg dokumentace.log dokumentace.run.xml dokumentace.pdf