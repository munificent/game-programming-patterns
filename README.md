**Note: Now that the book is done, I'm not actively working on it.**

There are only so many hours in the day, and I have other projects that need my
love, including a new book. This means I'm not responding to issues and pull
requests here. However, I do intend to circle back and do a second edition at
*some* point, so please do continue to file bugs.

Even though I'm not responding, I do deeply appreciate every issue filed.
Thank you!

&ndash; bob

---

This is the source repo for the book [Game Programming Patterns][].

## Building the Book

The book is written in Markdown (in `book/`). A little Python script (`script/format.py`) converts that along with a SASS file (`asset/style.scss`) and HTML template (`asset/template.html`) to the final HTML (in `html/`). To run the format script locally, you'll need to have Python 2.7-ish, and install Python Markdown, Pygments, and SmartyPants:

    $ pip install markdown
    $ pip install pygments
    $ pip install smartypants

You may need `sudo` for those. Once that's done, you can run:

    $ python script/format.py

Make sure to run this from the root directory of the repo. That will regenerate all of the chapter and section intro HTML files. If you're editing stuff, the script can also be run in watch mode:

    $ python script/format.py --watch

That will monitor the file system for changes to the markdown files, SASS file, or HTML template, and reprocess them as needed.

[game programming patterns]: http://gameprogrammingpatterns.com/
