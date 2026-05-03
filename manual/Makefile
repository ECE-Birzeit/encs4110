# Makefile for building experiment PDFs

TEX = pdflatex
TEX_OPTS = -interaction=nonstopmode -shell-escape

EXPERIMENTS = 1 2 3 4 5 6 7 8 9

.PHONY: all clean help

all: $(foreach n,$(EXPERIMENTS),encs4110_$(n).pdf)

encs4110_%.pdf: exp_standalone.tex chapters/exp%.tex
	@echo "Building Experiment $*..."
	$(TEX) $(TEX_OPTS) "\def\EXPNUM{$*}\input{exp_standalone.tex}" 2>/dev/null
	@mv exp_standalone.pdf $@

clean:
	@rm -f encs4110_*.pdf exp_standalone.pdf exp_standalone.aux exp_standalone.log exp_standalone.toc exp_standalone.out

help:
	@echo "Available targets:"
	@echo "  all      - Build all experiment PDFs"
	@echo "  clean    - Remove generated PDFs and temp files"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Individual experiments: make encs4110_1.pdf encs4110_2.pdf ..."