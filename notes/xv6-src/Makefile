
TEX_NAME=xv6_src_notes.tex
PDF_NAME=xv6_src_notes..pdf

.PHONY: $(PDF_NAME) all clean

all : $(PDF_NAME)

$(PDF_NAME): $(TEX_NAME)
	latexmk -shell-escape -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make $(TEX_NAME)

clean:
	latexmk -CA
