#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

# Converts from the source markup format to HTML for the web version.

import glob
import os
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

num_chapters = 0
empty_chapters = 0
total_words = 0


def htmlpath(pattern):
  return 'html/' + pattern + '.html'


def cpppath(pattern):
  return 'code/cpp/' + pattern + '.h'


def pretty(text):
  '''Use nicer HTML entities and special characters.'''
  text = text.replace(" -- ", "&thinsp;&mdash;&thinsp;")
  text = text.replace("à", "&agrave;")
  text = text.replace("ï", "&iuml;")
  text = text.replace("ø", "&oslash;")
  text = text.replace("æ", "&aelig;")
  return text


def formatfile(path, nav, skip_up_to_date):
  basename = os.path.basename(path)
  basename = basename.split('.')[0]

  # See if the HTML is up to date.
  sourcemod = os.path.getmtime(path)
  sourcemod = max(sourcemod, os.path.getmtime('asset/template.html'))
  if os.path.exists(cpppath(basename)):
    sourcemod = max(sourcemod, os.path.getmtime(cpppath(basename)))

  destmod = 0
  if os.path.exists(htmlpath(basename)):
    destmod = max(destmod, os.path.getmtime(htmlpath(basename)))

  if skip_up_to_date and sourcemod < destmod:
    return

  title = ''
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
        elif command == 'section':
          section = args
        elif command == 'code':
          contents = contents + includecode(basename, args, indentation)
        elif command == 'outline':
          isoutline = True
        else:
          print "UNKNOWN COMMAND:", command, args

      elif stripped.startswith('#'):
        # Build the page navigation from the headers.
        index = stripped.find(" ")
        headertype = stripped[:index]
        header = pretty(stripped[index:].strip())
        anchor = header.lower().replace(' ', '-')
        anchor = anchor.translate(None, '.?!:/')

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

  with open("asset/template.html") as f:
    template = f.read()

  # Write the HTML output.
  with open(htmlpath(basename), 'w') as out:
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

    html = template
    html = html.replace("{{title}}", title_text)
    html = html.replace("{{section_header}}", section_header)
    html = html.replace("{{header}}", title)
    html = html.replace("{{body}}", body)
    html = html.replace("{{prev}}", prev_link)
    html = html.replace("{{next}}", next_link)
    html = html.replace("{{navigation}}",
      navigationtohtml(title, navigation))

    out.write(html)

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


def navigationtohtml(chapter, headers):
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


def includecode(pattern, index, indentation):
  code = indentation + '    :::cpp\n'
  with open(cpppath(pattern), 'r') as source:
    inblock = False
    omitting = False
    omitting_name = False
    blockindent = 0

    for line in source:
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
            code += indentation + '    ' + line[blockindent:]

      else:
        if stripped == '//^' + index:
          inblock = True
          blockindent = len(line) - len(line.lstrip())

  return code


def buildnav(searchpath):
  nav = '<div class="nav">\n'
  nav = nav + '<h1><a href="index.html">Navigation</a></h1>\n'

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


def formatfiles(file_filter, skip_up_to_date):
  '''Process each markdown file.'''
  for f in glob.iglob(searchpath):
    if file_filter == None or file_filter in f:
      formatfile(f, nav, skip_up_to_date)


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
    formatfiles(None, True)
    check_sass()
    time.sleep(0.3)
else:
  # Can specify a file name filter to just regenerate a subset of the files.
  file_filter = None
  if len(sys.argv) > 1:
    file_filter = sys.argv[1]

  formatfiles(file_filter, False)

  average_word_count = total_words / (num_chapters - empty_chapters)
  estimated_word_count = total_words + (empty_chapters * average_word_count)
  percent_finished = total_words * 100 / estimated_word_count

  print "{}/~{} words ({}%)".format(
    total_words, estimated_word_count, percent_finished)
