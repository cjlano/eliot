TARGETS=programming.ps programming.pdf

all: $(TARGETS) 

%.pdf: %.ps
	ps2pdf $< 

%.ps: %.dvi
	dvips $< -o

%.dvi: %.tex
	latex $<

%.eps: %.fig
	fig2dev -L eps $< > $@

clean:
	rm -f *.dvi *.ps *.pdf *.aux *.log *.bak *.eps

clean_intermediate_files:
	rm -f *.aux *.log *.bak *.eps

programming.dvi: programming.tex dawg.eps
