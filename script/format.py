#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

# Converts from the source markup format to HTML for the web version.

import glob
import os
import re
import subprocess
import sys
import time
from datetime import datetime

import markdown
import smartypants

# Assumes cwd is root project dir.

GREEN = '\033[32m'
RED = '\033[31m'
DEFAULT = '\033[0m'
PINK = '\033[91m'
YELLOW = '\033[33m'

CHAPTERS = [
  "Acknowledgements",
  "Introduction",
  "Architecture, Performance, and Games",
  "Design Patterns Revisited",
  "Command",
  "Flyweight",
  "Observer",
  "Prototype",
  "Singleton",
  "State",
  "Sequencing Patterns",
  "Double Buffer",
  "Game Loop",
  "Update Method",
  "Behavioral Patterns",
  "Bytecode",
  "Subclass Sandbox",
  "Type Object",
  "Decoupling Patterns",
  "Component",
  "Event Queue",
  "Service Locator",
  "Optimization Patterns",
  "Data Locality",
  "Dirty Flag",
  "Object Pool",
  "Spatial Partition"
]

# URLs for hyperlinks to chapters. Note that the order is significant here.
# The index in this list + 1 is the chapter's number in the table of contents.
CHAPTER_HREFS = [
  "acknowledgements.html",
  "introduction.html",
  "architecture-performance-and-games.html",
  "command.html",
  "flyweight.html",
  "observer.html",
  "prototype.html",
  "singleton.html",
  "state.html",
  "double-buffer.html",
  "game-loop.html",
  "update-method.html",
  "bytecode.html",
  "subclass-sandbox.html",
  "type-object.html",
  "component.html",
  "event-queue.html",
  "service-locator.html",
  "data-locality.html",
  "dirty-flag.html",
  "object-pool.html",
  "spatial-partition.html"
]

num_chapters = 0
empty_chapters = 0
total_words = 0

extension = "html"

def output_path(pattern):
  return extension + "/" + pattern + "." + extension


def cpp_path(pattern):
  return 'code/cpp/' + pattern + '.h'


def pretty(text):
  '''Use nicer HTML entities and special characters.'''
  text = text.replace(" -- ", "&#8202;&mdash;&#8202;")
  text = text.replace("à", "&agrave;")
  text = text.replace("ï", "&iuml;")
  text = text.replace("ø", "&oslash;")
  text = text.replace("æ", "&aelig;")
  return text


def format_file(path, nav, skip_up_to_date):
  basename = os.path.basename(path)
  basename = basename.split('.')[0]

  # See if the HTML is up to date.
  sourcemod = os.path.getmtime(path)
  sourcemod = max(sourcemod, os.path.getmtime('asset/template.html'))
  if os.path.exists(cpp_path(basename)):
    sourcemod = max(sourcemod, os.path.getmtime(cpp_path(basename)))

  destmod = 0
  if os.path.exists(output_path(basename)):
    destmod = max(destmod, os.path.getmtime(output_path(basename)))

  if skip_up_to_date and sourcemod < destmod:
    return

  title = ''
  title_html = ''
  section = ''
  isoutline = False

  navigation = []

  # Read the markdown file and preprocess it.
  contents = ''
  with open(path, 'r') as input:
    # Read each line, preprocessing the special codes.
    for line in input:
      stripped = line.lstrip()
      indentation = line[:len(line) - len(stripped)]

      if stripped.startswith('^'):
        command,_,args = stripped.rstrip('\n').lstrip('^').partition(' ')
        args = args.strip()

        if command == 'title':
          title = args
          title_html = title

          # Remove any discretionary hyphens from the title.
          title = title.replace('&shy;', '')
        elif command == 'section':
          section = args
        elif command == 'code':
          contents = contents + include_code(basename, args, indentation)
        elif command == 'outline':
          isoutline = True
        else:
          print "UNKNOWN COMMAND:", command, args

      elif extension != "xml" and stripped.startswith('#'):
        # Build the page navigation from the headers.
        index = stripped.find(" ")
        headertype = stripped[:index]
        header = pretty(stripped[index:].strip())
        anchor = header.lower().replace(' ', '-')
        anchor = anchor.translate(None, '.?!:/"')

        # Add an anchor to the header.
        contents += indentation + headertype
        contents += '<a href="#' + anchor + '" name="' + anchor + '">' + header + '</a>\n'

        # Build the navigation.
        if len(headertype) == 2:
          navigation.append((len(headertype), header, anchor))

      else:
        contents += pretty(line)

  modified = datetime.fromtimestamp(os.path.getmtime(path))
  mod_str = modified.strftime('%B %d, %Y')

  with open("asset/template." + extension) as f:
    template = f.read()

  # Write the output.
  with open(output_path(basename), 'w') as out:
    title_text = title
    section_header = ""

    if section != "":
      title_text = title + " &middot; " + section
      section_href = section.lower().replace(" ", "-")
      section_header = '<span class="section"><a href="{}.html">{}</a></span>'.format(
        section_href, section)

    prev_link, next_link = make_prev_next(title)

    contents = contents.replace('<aside', '<aside markdown="1"')

    body = markdown.markdown(contents,
            extensions=['extra', 'def_list', 'codehilite'])
    body = body.replace('<aside markdown="1"', '<aside')

    body = smartypants.smartypants(body)

    output = template
    output = output.replace("{{title}}", title_text)
    output = output.replace("{{section_header}}", section_header)
    output = output.replace("{{header}}", title_html)
    output = output.replace("{{body}}", body)
    output = output.replace("{{prev}}", prev_link)
    output = output.replace("{{next}}", next_link)
    output = output.replace("{{navigation}}",
        navigation_to_html(title, navigation))

    if extension == "xml":
      output = clean_up_xml(output)

    out.write(output)

  global total_words
  global num_chapters
  global empty_chapters

  word_count = len(contents.split(None))
  if section:
    num_chapters += 1
    if word_count < 50:
      empty_chapters += 1
      print "  {}".format(basename)
    elif word_count < 2000:
      empty_chapters += 1
      print "{}-{} {} ({} words)".format(
        YELLOW, DEFAULT, basename, word_count)
    else:
      total_words += word_count
      print "{}✓{} {} ({} words)".format(
        GREEN, DEFAULT, basename, word_count)
  else:
    # Section header chapters aren't counted like regular chapters.
    print "{}•{} {} ({} words)".format(
      GREEN, DEFAULT, basename, word_count)


def clean_up_xml(output):
  """Takes the XHTML output and massages it to play nicer with InDesign's XML
  import... idiosyncracies."""

  # Split into preformatted code and regular markup sections. We need to treat
  # code blocks specially so we can preserve their formatting.
  in_code = False
  chunks = re.split("(</?pre>)", output)

  def clean_up_code_xml(code):
    # Ditch most code formatting tags.
    code = re.sub(r'<span class="(k|kt|mi|n|nb|nc|nf|nl|o|p)">([^<]+)</span>',
                  r"\2", code)

    # Turn comments into something InDesign can map to a style.
    code = re.sub(r'<span class="(c1|cn)">([^<]+)</span>',
                  r"<comment>\2</comment>", code)

    # Turn newlines into soft returns so code blocks stay one paragraph, except
    # for the last one.
    code = code[:-1].replace("\n", "&#x2028;") + "\n"

    return code

  def fix_link(match):
    tag = match.group(1)
    contents = match.group(2)
    href = re.search(r'href\s*=\s*"([^"]+)"', tag).group(1)

    # If it's not a link to a chapter, just return the contents of the link and
    # strip out the link itself.
    if not href in CHAPTER_HREFS:
      return contents

    # Turn it into a chapter number reference.
    return "{}<chap-ref> ({})</chap-ref>".format(
        contents, CHAPTER_HREFS.index(href) + 1)

  def clean_up_xhtml(html):
    # Replace chapter links with chapter number references and remove other
    # links.
    html = re.sub(r"<a\s+([^>]+)>([^<]+)</a>", fix_link, html)

    # Ditch newlines in the middle of blocks of text. Out of sheer malice,
    # even though they are meaningless in actual XML, InDesign treats them
    # as significant.
    html = re.sub(r"\n(?<!<)", " ", html)

    # Also collapse redundant whitespace.
    html = re.sub(r" +", " ", html)
    html = html.replace("> <", "><")

    # Re-add newlines after closing paragraph-level tags.
    html = html.replace("</p>", "</p>\n")
    html = html.replace("</h2>", "</h2>\n")
    html = html.replace("</h3>", "</h3>\n")
    html = html.replace("</li>", "</li>\n")
    html = html.replace("</ol>", "</ol>\n")
    html = html.replace("</ul>", "</ul>\n")
    html = html.replace("</pre>", "</pre>\n")
    html = html.replace("</aside>", "</aside>\n")
    html = html.replace("</blockquote>", "</blockquote>\n")

    # TODO: Non-breaking spaces in <code>...</code> sections.
    return html

  result = ""
  for chunk in chunks:
    if chunk == "<pre>":
      in_code = True
      result += chunk
    elif chunk == "</pre>":
      in_code = False
      result += chunk
    else:
      if in_code:
        result += clean_up_code_xml(chunk)
      else:
        result += clean_up_xhtml(chunk)

  return result


def title_to_file(title):
  """Given a title like "Event Queue", converts it to the corresponding file
  name like "event-queue"."""

  return (title.lower()
    .replace(" ", "-")
    .replace(",", ""))


def make_prev_next(title):
  """Generate the links that thread through the chapters."""
  chapter_index = CHAPTERS.index(title)
  prev_link = ""
  next_link = ""
  if chapter_index > 0:
    prev_href = title_to_file(CHAPTERS[chapter_index - 1])
    prev_link = '<span class="prev">&larr; <a href="{}.html">Previous Chapter</a></span>'.format(
      prev_href, CHAPTERS[chapter_index - 1])

  if chapter_index < len(CHAPTERS) - 1:
    next_href = title_to_file(CHAPTERS[chapter_index + 1])
    next_link = '<span class="next"><a href="{}.html">Next Chapter</a> &rarr;</span>'.format(
      next_href, CHAPTERS[chapter_index + 1])

  return (prev_link, next_link)


def navigation_to_html(chapter, headers):
  nav = ''

  # Section headers start two levels deep.
  currentdepth = 1
  for depth, header, anchor in headers:
    if currentdepth == depth:
      nav += '</li><li>\n'

    while currentdepth < depth:
      nav += '<ul><li>\n'
      currentdepth += 1

    while currentdepth > depth:
      nav += '</li></ul>\n'
      currentdepth -= 1

    nav += '<a href="#' + anchor + '">' + header + '</a>'


  # Close the lists.
  while currentdepth > 1:
    nav += '</li></ul>\n'
    currentdepth -= 1

  return nav


def include_code(pattern, index, indentation):
  with open(cpp_path(pattern), 'r') as source:
    lines = source.readlines()

  code = indentation + '    :::cpp\n'
  inblock = False
  omitting = False
  omitting_name = False
  blockindent = 0

  for line in lines:
    stripped = line.strip()

    if inblock:
      if stripped == '//^' + index:
        # End of our block.
        break

      elif stripped == '//^omit':
        omitting = not omitting

      elif stripped == '//^omit ' + index:
        # Omitting a section just for this block. Other blocks that
        # Contain this code may not omit it.
        omitting_name = not omitting_name

      elif stripped.startswith('//^'):
        # A code block comment for another block,
        # so just ignore it. This can occur with
        # nested code blocks.
        pass

      elif not omitting and not omitting_name:
        # Hackish. Can't strip the leading indent off of blank
        # lines.
        if stripped == '':
          code += '\n'
        else:
          code_line = line[blockindent:]
          if len(code_line) > 64:
            print "Warning long source line ({} chars):\n{}".format(
                len(code_line), code_line)
          code += indentation + '    ' + code_line

    else:
      if stripped == '//^' + index:
        inblock = True
        blockindent = len(line) - len(line.lstrip())

  return code


def buildnav(searchpath):
  nav = '<div class="nav">\n'
  nav = nav + '<h1><a href="/">Navigation</a></h1>\n'

  # Read the chapter outline from the index page.
  with open('html/index.html', 'r') as source:
    innav = False

    for line in source:
      if innav:
        nav = nav + line
        if line.startswith('</ul'):
          break;
      else:
        if line.startswith('<ul>'):
          nav = nav + line
          innav = True

  nav = nav + '</div>'

  return nav


def format_files(file_filter, skip_up_to_date):
  '''Process each markdown file.'''
  for f in glob.iglob(searchpath):
    if file_filter == None or file_filter in f:
      format_file(f, nav, skip_up_to_date)


def check_sass():
  sourcemod = os.path.getmtime('asset/style.scss')
  destmod = os.path.getmtime('html/style.css')
  if sourcemod < destmod:
    return

  subprocess.call(['sass', 'asset/style.scss', 'html/style.css'])
  print "{}✓{} style.css".format(GREEN, DEFAULT)


searchpath = ('book/*.markdown')

nav = buildnav(searchpath)

if len(sys.argv) == 2 and sys.argv[1] == '--watch':
  while True:
    format_files(None, True)
    check_sass()
    time.sleep(0.3)
else:
  if len(sys.argv) > 1 and sys.argv[1] == '--xml':
    extension = "xml"
    del sys.argv[1]

  # Can specify a file name filter to just regenerate a subset of the files.
  file_filter = None
  if len(sys.argv) > 1:
    file_filter = sys.argv[1]

  format_files(file_filter, False)

  average_word_count = total_words / (num_chapters - empty_chapters)
  estimated_word_count = total_words + (empty_chapters * average_word_count)
  percent_finished = total_words * 100 / estimated_word_count

  print "{}/~{} words ({}%)".format(
    total_words, estimated_word_count, percent_finished)
