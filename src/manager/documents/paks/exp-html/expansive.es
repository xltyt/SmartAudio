Expansive.load({
    transforms: {
        name:       'minify-html',
        mappings:   'html',
        options:    '--remove-comments --conservative-collapse --remove-attribute-quotes --remove-empty-attributes --remove-optional-tags'
        script: `
            function transform(contents, meta, service) {
                let htmlmin = Cmd.locate('html-minifier')
                if (!htmlmin) {
                    trace('Warn', 'Cannot find html-minifier')
                    return contents
                }
                return run(htmlmin + ' ' + service.options, contents)
            }
        `
    }
})
