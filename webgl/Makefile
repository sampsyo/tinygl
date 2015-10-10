OUT_JS := bundle.js

$(OUT_JS): tinygl.js package.json
	npm pages

.PHONY: clean
clean:
	rm -rf node_modules $(OUT_JS)

.PHONY: view
view:
	open index.html
