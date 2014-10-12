P = proposal
TEX_INPUTS = $(TEXINPUTS):../resources/tex/

all: clobber $(P).pdf clean

$(P).pdf:
	TEXINPUTS=$(TEX_INPUTS) xelatex $(P).tex

clean:
	rm -f *.aux
	rm -f *.out
	rm -f *.fdb_latexmk
	rm -f *.synctex.gz
	rm -f *.log
	rm -f *-eps-converted-to.pdf

clobber: clean
	rm -f $(P).pdf