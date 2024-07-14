# Makefile to convert report.md to report.pdf using pandoc

# Variables
MD_FILE = report.md
PDF_FILE = report.pdf
PANDOC = pandoc

# Rule to convert markdown to pdf
$(PDF_FILE): $(MD_FILE)
	$(PANDOC) $(MD_FILE) -o temp.html
	$(PANDOC) temp.html -o $(PDF_FILE)
	rm -f temp.html

# Clean up generated files
clean:
	rm -f $(PDF_FILE)

# Phony targets
.PHONY: clean
