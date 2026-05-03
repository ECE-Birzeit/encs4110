# Makefile for building experiment PDFs and full manual

TEX  = pdflatex
TEX_OPTS = -interaction=batchmode -shell-escape -quiet
TEX_PATH = C:/Users/Mohammad/AppData/Local/Programs/MiKTeX/miktex/bin/x64/pdflatex.exe

EXPS = 1 2 3 4 5 6 7 8 9

.PHONY: all clean help manual $(EXPS)

all: $(EXPS)

manual:
	@$(TEX_PATH) $(TEX_OPTS) main.tex \
		&& mv -f main.pdf encs4110_manual.pdf \
		|| (test -f main.pdf && mv -f main.pdf encs4110_manual.pdf || true)

.PHONY: encs4110_manual
	@$(TEX_PATH) $(TEX_OPTS) main.tex \
		&& mv -f main.pdf $@ \
		|| (test -f main.pdf && mv -f main.pdf $@ || true)

$(EXPS):
	@$(TEX_PATH) $(TEX_OPTS) "\def\EXPNUM{$(@)}\input{exp_standalone.tex}" \
		&& mv -f exp_standalone.pdf encs4110_$(@).pdf \
		|| (test -f exp_standalone.pdf && mv -f exp_standalone.pdf encs4110_$(@).pdf || true)

clean:
	@rm -f encs4110_*.pdf main.pdf exp_standalone.pdf *.aux *.log *.toc

help:
	@echo "Usage: make <target>"
	@echo ""
	@echo "Targets:"
	@echo "  1-9     Build individual experiment PDFs"
	@echo "  all     Build all standalone experiments"
	@echo "  manual  Build full manual"
	@echo "  clean   Remove generated PDFs"
	@echo "  help    Show this message"