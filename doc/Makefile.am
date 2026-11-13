dist_man1_MANS = frobby.1
EXTRA_DIST = manual.tex frobbyLatex.sty

doc: docPs docPdf

docPs: manual.dvi
docPdf: manual.pdf

%.dvi: %.tex
	for i in 1 2 3; do latex $<; done

%.pdf: %.tex
	for i in 1 2 3; do pdflatex $<; done

CLEANFILES = *.aux *.dvi *.log *.pdf *.toc
