man1_MANS = frobby.1

doc: docPs docPdf

docPs: manual.dvi
docPdf: manual.pdf

%.dvi: %.tex
	for i in 1 2 3; do latex $<; done

%.pdf: %.tex
	for i in 1 2 3; do pdflatex $<; done

CLEANFILES = *.aux *.dvi *.log *.pdf *.toc
