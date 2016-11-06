.PHONY: start new

start:
	@echo "Targets"
	@echo "- new - Make a new ESP-IDF Template app"

new:
	@echo "Creating a new ESP-IDF Template app"
	@git clone https://github.com/espressif/esp-idf-template.git newapp
	@rm -rf newapp/.git
	@echo "New app can be found in newapp"
