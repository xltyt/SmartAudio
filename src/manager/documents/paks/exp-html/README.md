exp-html
===

Expansive plugin for the HTML minifier.

Provides the 'minify-html' service.

### To install:

    pak install exp-html

### To configure in expansive.json:

* minify-html.enable &mdash; Enable the minify-html service to post-process HTML files.
* minify-html.options &mdash; Command line options to html-minifier

```
{
    services: {
        'minify-html': {
            enable: true,
            options: '--remove-comments \
                      --conservative-collapse \
                      --remove-attribute-quotes \
                      --remove-empty-attributes \
                      --remove- optional-tags'
        }
    }
}
```

### Get Pak from

[https://embedthis.com/pak/](https://embedthis.com/pak/download.html)
