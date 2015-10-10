OUT_JS := bundle.js

$(OUT_JS): tinygl.js package.json
	npm run-script build

.PHONY: clean
clean:
	rm -rf node_modules $(OUT_JS)

.PHONY: view
view: $(OUT_JS)
	open index.html
